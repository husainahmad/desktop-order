#include "apiclient.h"
#include "tokenmanager.h"

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonParseError>

namespace {
QString stripUrlFromError(QString message)
{
    const QString separator = " - server replied: ";
    const int idx = message.indexOf(separator);
    if (idx >= 0) {
        message = message.mid(idx + separator.length());
    }
    return message;
}

QString friendlyError(const QByteArray &data, const QString &rawError, int httpStatus)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        const QString serverMessage = doc.object()["message"].toString();
        if (!serverMessage.isEmpty()) {
            return serverMessage;
        }
    }

    QString message = stripUrlFromError(rawError);
    if (message.isEmpty()) {
        message = httpStatus > 0 ? QString("HTTP %1").arg(httpStatus) : "Network error";
    }
    return message;
}
} // namespace

ApiClient &ApiClient::instance()
{
    static ApiClient client;
    return client;
}

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
{
}

void ApiClient::get(const QUrl &url, SuccessFn success, ErrorFn error)
{
    send(QNetworkAccessManager::GetOperation, url, QByteArray(), 1,
         std::move(success), std::move(error));
}

void ApiClient::post(const QUrl &url, const QByteArray &body, SuccessFn success, ErrorFn error)
{
    send(QNetworkAccessManager::PostOperation, url, body, 1,
         std::move(success), std::move(error));
}

void ApiClient::put(const QUrl &url, const QByteArray &body, SuccessFn success, ErrorFn error)
{
    send(QNetworkAccessManager::PutOperation, url, body, 1,
         std::move(success), std::move(error));
}

void ApiClient::send(QNetworkAccessManager::Operation op, const QUrl &url, const QByteArray &body,
                     int retriesRemaining, SuccessFn success, ErrorFn error)
{
    adjustPendingRequests(+1);

    QNetworkRequest request(url);
    const QString token = TokenManager::instance().getAccessToken();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", "Bearer " + token.toUtf8());
    }
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = nullptr;
    switch (op) {
    case QNetworkAccessManager::GetOperation:
        reply = m_manager.get(request);
        break;
    case QNetworkAccessManager::PostOperation:
        reply = m_manager.post(request, body);
        break;
    case QNetworkAccessManager::PutOperation:
        reply = m_manager.put(request, body);
        break;
    default:
        break;
    }

    if (!reply) {
        adjustPendingRequests(-1);
        if (error) {
            error(tr("Unsupported network operation"), -1);
        }
        return;
    }

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, op, url, body, retriesRemaining, success, error]() {
                handleFinished(reply, op, url, body, retriesRemaining, success, error);
            });
}

void ApiClient::handleFinished(QNetworkReply *reply, QNetworkAccessManager::Operation op,
                               const QUrl &url, const QByteArray &body, int retriesRemaining,
                               SuccessFn success, ErrorFn error)
{
    const QByteArray data = reply->readAll();
    const QString errorString = reply->errorString();
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        adjustPendingRequests(-1);
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            if (success) {
                success(doc.object());
            }
        } else if (success) {
            success(QJsonObject());
        }
        return;
    }

    if (httpStatus == 401 && retriesRemaining > 0) {
        TokenManager::instance().refreshToken(
            [this, op, url, body, retriesRemaining, success, error, errorString](bool refreshed) {
                if (refreshed) {
                    send(op, url, body, retriesRemaining - 1, success, error);
                } else {
                    adjustPendingRequests(-1);
                    if (error) {
                        error(stripUrlFromError(errorString), 401);
                    }
                }
            });
        return;
    }

    adjustPendingRequests(-1);
    if (error) {
        error(friendlyError(data, errorString, httpStatus), httpStatus);
    }
}

void ApiClient::adjustPendingRequests(int delta)
{
    const bool wasBusy = m_pendingRequests > 0;
    m_pendingRequests += delta;
    if (m_pendingRequests < 0) {
        m_pendingRequests = 0;
    }
    const bool isBusy = m_pendingRequests > 0;
    if (wasBusy != isBusy) {
        emit busyChanged(isBusy);
    }
}
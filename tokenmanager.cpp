#include "tokenmanager.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDateTime>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>

TokenManager& TokenManager::instance() {
    static TokenManager manager;
    return manager;
}

TokenManager::TokenManager() = default;

QString TokenManager::getAccessToken() {
    QString accessToken = settingConfig.getValue("authToken").toString();
    if (accessToken.isEmpty()) {
        return QString();
    }

    if (isAccessTokenExpired(accessToken)) {
        if (refreshAccessToken()) {
            return settingConfig.getValue("authToken").toString();
        }
        clearTokens();
        return QString();
    }

    return accessToken;
}

QString TokenManager::getRefreshToken() {
    return settingConfig.getValue("refreshToken").toString();
}

void TokenManager::setTokens(const QString &accessToken, const QString &refreshToken) {
    settingConfig.setValue("authToken", accessToken);
    settingConfig.setValue("refreshToken", refreshToken);
    settingConfig.sync();
}

void TokenManager::clearTokens() {
    settingConfig.setValue("authToken", QString());
    settingConfig.setValue("refreshToken", QString());
    settingConfig.sync();
}

bool TokenManager::isAccessTokenExpired(const QString &accessToken) const {
    QStringList parts = accessToken.split('.');
    if (parts.size() != 3) {
        return true;
    }

    QByteArray payload = QByteArray::fromBase64(parts[1].toUtf8(),
                                                QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isObject()) {
        return true;
    }

    double exp = doc.object()["exp"].toDouble();
    if (exp <= 0) {
        return true;
    }

    const qint64 bufferSeconds = 30;
    return QDateTime::currentSecsSinceEpoch() >= static_cast<qint64>(exp) - bufferSeconds;
}

bool TokenManager::refreshAccessToken() {
    QString refreshToken = getRefreshToken();
    if (refreshToken.isEmpty()) {
        qWarning() << "TokenManager: No refresh token available.";
        return false;
    }

    QString refreshUrl = settingConfig.getApiEndpoint("auth", "refresh-token");
    if (refreshUrl.isEmpty()) {
        qWarning() << "TokenManager: Refresh endpoint not configured.";
        return false;
    }

    QNetworkRequest request{QUrl(refreshUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager.post(request, refreshToken.toUtf8());

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(10000);

    loop.exec();

    if (!timer.isActive()) {
        reply->abort();
        reply->deleteLater();
        qWarning() << "TokenManager: Refresh request timed out.";
        return false;
    }
    timer.stop();

    bool refreshed = false;
    if (reply->error() == QNetworkReply::NoError) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            QString newAccessToken = obj["accessToken"].toString();
            QString newRefreshToken = obj["refreshToken"].toString();
            if (!newAccessToken.isEmpty()) {
                setTokens(newAccessToken, newRefreshToken);
                refreshed = true;
            }
        }
    } else {
        qWarning() << "TokenManager: Refresh request failed:" << reply->errorString();
    }

    reply->deleteLater();
    return refreshed;
}

#include "tokenmanager.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDateTime>
#include <QDebug>

TokenManager& TokenManager::instance() {
    static TokenManager manager;
    return manager;
}

TokenManager::TokenManager() = default;

QString TokenManager::getAccessToken() {
    return settingConfig.getValue("authToken").toString();
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

void TokenManager::refreshToken(const std::function<void(bool)> &callback) {
    const QString refreshToken = getRefreshToken();
    if (refreshToken.isEmpty()) {
        qWarning() << "TokenManager: No refresh token available.";
        clearTokens();
        if (callback) callback(false);
        return;
    }

    const QString refreshUrl = settingConfig.getApiEndpoint("auth", "refresh-token");
    if (refreshUrl.isEmpty()) {
        qWarning() << "TokenManager: Refresh endpoint not configured.";
        if (callback) callback(false);
        return;
    }

    QNetworkRequest request{QUrl(refreshUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager.post(request, refreshToken.toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        const bool success = reply->error() == QNetworkReply::NoError;
        const QString errorString = reply->errorString();
        reply->deleteLater();

        if (!success) {
            qWarning() << "TokenManager: Refresh request failed:" << errorString;
            clearTokens();
            if (callback) callback(false);
            return;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            const QString newAccessToken = obj["accessToken"].toString();
            const QString newRefreshToken = obj["refreshToken"].toString();
            if (!newAccessToken.isEmpty()) {
                setTokens(newAccessToken, newRefreshToken);
                if (callback) callback(true);
                return;
            }
        }

        clearTokens();
        if (callback) callback(false);
    });
}
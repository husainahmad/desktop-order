#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <QString>
#include <QNetworkAccessManager>
#include "setting.h"

class TokenManager
{
public:
    static TokenManager& instance();

    QString getAccessToken();
    QString getRefreshToken();
    void setTokens(const QString &accessToken, const QString &refreshToken);
    void clearTokens();

private:
    TokenManager();
    TokenManager(const TokenManager&) = delete;
    TokenManager& operator=(const TokenManager&) = delete;

    bool refreshAccessToken();
    bool isAccessTokenExpired(const QString &accessToken) const;
    QByteArray sendRefreshRequest(const QString &refreshToken) const;

    QNetworkAccessManager networkManager;
    Setting settingConfig;
};

#endif // TOKENMANAGER_H

#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <functional>
#include "setting.h"

class TokenManager : public QObject
{
    Q_OBJECT

public:
    static TokenManager& instance();

    QString getAccessToken();
    QString getRefreshToken();
    void setTokens(const QString &accessToken, const QString &refreshToken);
    void clearTokens();

    // Asynchronously refresh the access token. callback(true) on success.
    void refreshToken(const std::function<void(bool)> &callback);

private:
    TokenManager();
    TokenManager(const TokenManager&) = delete;
    TokenManager& operator=(const TokenManager&) = delete;

    bool isAccessTokenExpired(const QString &accessToken) const;

    QNetworkAccessManager networkManager;
    Setting settingConfig;
};

#endif // TOKENMANAGER_H
#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QUrl>
#include <functional>

class ApiClient : public QObject
{
    Q_OBJECT

public:
    using SuccessFn = std::function<void(const QJsonObject &response)>;
    using ErrorFn = std::function<void(const QString &message, int httpStatus)>;

    static ApiClient &instance();

    void get(const QUrl &url, SuccessFn success, ErrorFn error);
    void post(const QUrl &url, const QByteArray &body, SuccessFn success, ErrorFn error);
    void put(const QUrl &url, const QByteArray &body, SuccessFn success, ErrorFn error);

signals:
    void busyChanged(bool busy);

private:
    explicit ApiClient(QObject *parent = nullptr);
    ApiClient(const ApiClient &) = delete;
    ApiClient &operator=(const ApiClient &) = delete;

    void send(QNetworkAccessManager::Operation op, const QUrl &url, const QByteArray &body,
              int retriesRemaining, SuccessFn success, ErrorFn error);
    void handleFinished(QNetworkReply *reply, QNetworkAccessManager::Operation op,
                        const QUrl &url, const QByteArray &body, int retriesRemaining,
                        SuccessFn success, ErrorFn error);
    void adjustPendingRequests(int delta);

    QNetworkAccessManager m_manager;
    int m_pendingRequests = 0;
};

#endif // APICLIENT_H
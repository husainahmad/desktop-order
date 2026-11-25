#include "imageloader.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

ImageLoader &ImageLoader::instance()
{
    static ImageLoader loader;
    return loader;
}

ImageLoader::ImageLoader(QObject *parent) : QObject(parent)
{
}

QPixmap ImageLoader::cached(const QString &url) const
{
    return m_cache.value(url);
}

void ImageLoader::request(const QString &url)
{
    if (url.isEmpty() || m_cache.contains(url) || m_pending.contains(url)) {
        return;
    }
    m_pending.insert(url);

    QNetworkRequest request{QUrl(url)};
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = m_manager.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        reply->deleteLater();
        m_pending.remove(url);
        if (reply->error() != QNetworkReply::NoError) {
            return;
        }
        QPixmap pixmap;
        if (pixmap.loadFromData(reply->readAll())) {
            m_cache.insert(url, pixmap);
            emit imageLoaded(url, pixmap);
        }
    });
}

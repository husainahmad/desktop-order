#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QHash>
#include <QSet>
#include <QPixmap>

/**
 * Shared asynchronous loader for hosted product images.
 *
 * The menu service uploads images to ImgBB and returns only a URL, so product
 * images are downloads rather than base64 blobs. This keeps one network manager
 * and an in-memory cache keyed by URL, and notifies views via imageLoaded() so a
 * grid of cards sharing a link is fetched once.
 */
class ImageLoader : public QObject
{
    Q_OBJECT

public:
    static ImageLoader &instance();

    QPixmap cached(const QString &url) const;
    void request(const QString &url);

signals:
    void imageLoaded(const QString &url, const QPixmap &pixmap);

private:
    explicit ImageLoader(QObject *parent = nullptr);
    ImageLoader(const ImageLoader &) = delete;
    ImageLoader &operator=(const ImageLoader &) = delete;

    QNetworkAccessManager m_manager;
    QHash<QString, QPixmap> m_cache;
    QSet<QString> m_pending;
};

#endif // IMAGELOADER_H

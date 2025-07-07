#include "cacheutils.h"
#include <QString>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

CacheUtils::CacheUtils() {}

void CacheUtils::clearAppCache() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QFile::remove(appDataPath + "/category_cache.json");

    QDir dir(appDataPath);
    QStringList filters = QStringList() << "product_cache_*.json";
    for (const QString &file : dir.entryList(filters, QDir::Files)) {
        QFile::remove(appDataPath + "/" + file);
    }
}

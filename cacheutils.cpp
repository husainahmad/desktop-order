#include "cacheutils.h"
#include <QString>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

CacheUtils::CacheUtils() {}

void CacheUtils::clearAppCache() {
    const QStringList locations = {
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation),
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
    };

    for (const QString &dirPath : locations) {
        if (dirPath.isEmpty()) {
            continue;
        }

        QDir dir(dirPath);
        QStringList filters = QStringList() << "*.json";
        for (const QString &file : dir.entryList(filters, QDir::Files)) {
            QFile::remove(dir.filePath(file));
        }
    }
}

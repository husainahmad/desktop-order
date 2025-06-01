#include "setting.h"
#include <QCoreApplication>
#include <QDir>

Setting::Setting()
    : settings(QDir(QCoreApplication::applicationDirPath()).filePath("../../../config.app.ini"), QSettings::IniFormat)
{
    qDebug() << "Using config path:" << settings.fileName();
}

QString Setting::getApiEndpoint(const QString &section, const QString &key) {
    QString fullKey = section + "/" + key;

    if (!settings.contains(fullKey)) {
        qWarning() << "Config not found for key:" << fullKey;
        return QString();
    }

    return settings.value(fullKey).toString();
}

QString Setting::resolveIniPath() {
    QDir dir(QCoreApplication::applicationDirPath());
    while (!dir.exists("config.app.ini") && dir.cdUp()) {}
    return dir.filePath("config.app.ini");
}

void Setting::setValue(const QString &key, const QVariant &value) {
    settings.setValue(key, value);
}

QVariant Setting::getValue(const QString &key, const QVariant &defaultValue) {
    return settings.value(key, defaultValue);
}

void Setting::sync() {
    settings.sync();
}

#ifndef SETTING_H
#define SETTING_H

#include <QSettings>
#include <QDebug>

#include <QCoreApplication>
#include <QDir>

class Setting
{
public:
    Setting();
    QString getApiEndpoint(const QString &section, const QString &key);
    QString resolveIniPath();
    void setValue(const QString &key, const QVariant &value);
    QVariant getValue(const QString &key, const QVariant &defaultValue = QVariant());
    void sync();

private:
    QSettings settings;
};

#endif // SETTING_H

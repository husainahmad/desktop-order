#ifndef ORDERPRINT_H
#define ORDERPRINT_H

#include <QJsonObject>
#include "setting.h"
#include <QString>
#include <QByteArray>

class OrderPrint
{
public:
    OrderPrint(const QJsonObject &order);
    void sendToReceiptPrinter();
    void sendToKitchenPrinter();
    void sendSettlementToReceiptPrinter();
    bool sendRawDataToPrinter(const QString &printerName, const QByteArray &data);
private slots:

private:
    QJsonObject orderDetails;
    QLocale locale;
    Setting settingConfig;
};

#endif // ORDERPRINT_H

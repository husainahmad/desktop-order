#ifndef ORDERPRINT_H
#define ORDERPRINT_H

#include <QJsonObject>
#include "setting.h"

class OrderPrint
{
public:
    OrderPrint(const QJsonObject &order);
    void sendToReceiptPrinter();
    void sendToKitchenPrinter();
    void sendSettlementToReceiptPrinter();

private slots:

private:
    QJsonObject orderDetails;
    QLocale locale;
    Setting settingConfig;
};

#endif // ORDERPRINT_H

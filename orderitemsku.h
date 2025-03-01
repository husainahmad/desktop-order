#ifndef ORDERITEMSKU_H
#define ORDERITEMSKU_H

#include <QString>

class OrderItemSku
{
public:
    int skuId;
    QString skuName;
    int quantity;
    double price;
    double subTotal;
    OrderItemSku(int skuId, QString skuName, int quantity, double price, double subTotal);
};

#endif // ORDERITEMSKU_H

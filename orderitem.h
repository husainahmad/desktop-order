#ifndef ORDERITEM_H
#define ORDERITEM_H

#include <QString>
#include <QList>
#include "orderitemsku.h"

class OrderItem
{
public:
    int productId;
    QString productName;
    int categoryId;
    QList<OrderItemSku> orderItemSkus;
    OrderItem(int productId, QString productName, int categoryId, QList<OrderItemSku> orderItemSkus);
};

#endif // ORDERITEM_H

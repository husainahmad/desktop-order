#ifndef ORDER_H
#define ORDER_H

#include <QString>
#include <QList>
#include "orderitem.h"

class Order
{
public:
    int id;
    int customerId;
    QString customerName;
    QString remark;
    double subTotal;
    double subTotalTax;
    double subTotalDiscount;
    double grandTotal;
    QList<OrderItem> orderItems;
    Order();
    Order(int id, int customerId, QString customerName, QString remark, double subTotal, double subTotalTax,
          double subTotalDiscount, double grandTotal, QList<OrderItem> orderItems);
};

#endif // ORDER_H

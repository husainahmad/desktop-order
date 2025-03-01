#include "order.h"

Order::Order() {}
Order::Order(int id, int customerId, QString customerName, QString remark, double subTotal, double subTotalTax,
             double subTotalDiscount, double grandTotal, QList<OrderItem> orderItems)
    : id(id), customerId(customerId), customerName(customerName), remark(remark), subTotal(subTotal), subTotalTax(subTotalTax),
    subTotalDiscount(subTotalDiscount), grandTotal(grandTotal), orderItems(orderItems) {}

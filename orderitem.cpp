#include "orderitem.h"

OrderItem::OrderItem(int productId, QString productName, QList<OrderItemSku> orderItemSkus) :
    productId(productId), productName(productName), orderItemSkus(orderItemSkus) {}

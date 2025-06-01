#include "orderitem.h"

OrderItem::OrderItem(int productId, QString productName, int categoryId, QList<OrderItemSku> orderItemSkus) :
    productId(productId), productName(productName), categoryId(categoryId), orderItemSkus(orderItemSkus) {}

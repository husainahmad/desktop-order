#include "orderitemsku.h"

OrderItemSku::OrderItemSku(int skuId, QString skuName, int quantity, double price, double subTotal) :
    skuId(skuId), skuName(skuName), quantity(quantity), price(price), subTotal(subTotal) {}

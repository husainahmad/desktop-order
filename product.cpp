#include "product.h"
#include "sku.h"
#include "productimage.h"

Product::Product(int id, QString name, QString imagePath, QList<Sku> skus, ProductImage productImage)
    : id(id), name(name), imagePath(imagePath), skus(skus), productImage(productImage) {}

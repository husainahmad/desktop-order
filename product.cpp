#include "product.h"
#include "sku.h"
#include "productimage.h"

Product::Product(int id, QString name, int categoryId, QString imagePath, QList<Sku> skus, ProductImage productImage)
    : id(id), name(name), categoryId(categoryId), imagePath(imagePath), skus(skus), productImage(productImage) {}

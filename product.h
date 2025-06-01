#ifndef PRODUCT_H
#define PRODUCT_H

#include <QString>
#include <sku.h>
#include <QList>
#include "productimage.h"

class Product
{
public:
    int id;
    QString name;
    int categoryId;
    QString imagePath;
    QList<Sku> skus;
    ProductImage productImage;
    Product(int id, QString name, int categoryId, QString imagePath = "", QList<Sku> skus = QList<Sku>(), ProductImage productImage = ProductImage());
};

#endif // PRODUCT_H

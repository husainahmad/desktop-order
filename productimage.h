#ifndef PRODUCTIMAGE_H
#define PRODUCTIMAGE_H

#include <QString>

class ProductImage
{
public:
    int id;
    QString fileName;
    QString url;
    QString mimeType;

    ProductImage();
    ProductImage(int id, QString fileName, QString url, QString mimeType);
};

#endif // PRODUCTIMAGE_H

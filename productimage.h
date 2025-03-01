#ifndef PRODUCTIMAGE_H
#define PRODUCTIMAGE_H

#include <QString>

class ProductImage
{
public:
    int id;
    QString fileName;
    QString imageBlob;
    QString mimeType;

    ProductImage();
    ProductImage(int id, QString fileName, QString imageBlob, QString mimeType);
};

#endif // PRODUCTIMAGE_H

#include "productimage.h"
#include <QString>


ProductImage::ProductImage() : id(0), fileName(""), imageBlob(""), mimeType("") {}
ProductImage::ProductImage(int id, QString fileName, QString imageBlob, QString mimeType) :
    id(id), fileName{fileName}, imageBlob(imageBlob), mimeType(mimeType) {}

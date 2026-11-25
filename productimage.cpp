#include "productimage.h"
#include <QString>


ProductImage::ProductImage() : id(0), fileName(""), url(""), mimeType("") {}
ProductImage::ProductImage(int id, QString fileName, QString url, QString mimeType) :
    id(id), fileName{fileName}, url(url), mimeType(mimeType) {}

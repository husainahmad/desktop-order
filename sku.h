#ifndef SKU_H
#define SKU_H

#include <QString>
class Sku
{
public:
    int id;
    QString name;
    double price;
    Sku(int id, QString name, double price);
};

#endif // SKU_H

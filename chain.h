#ifndef CHAIN_H
#define CHAIN_H

#include <QString>
#include <brand.h>

class Chain
{
public:
    int id;
    QString name;
    Brand brand;
    Chain(int id, QString name, Brand brand);
};

#endif // CHAIN_H

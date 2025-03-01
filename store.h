#ifndef STORE_H
#define STORE_H
#include <QString>
#include <chain.h>

class Store
{
public:
    int id;
    QString name;
    Chain chain;
    Store(int id, QString name, Chain chain);
};

#endif // STORE_H

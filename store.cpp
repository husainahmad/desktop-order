#include "store.h"
#include <QString>
#include "chain.h"

Store::Store(int id, QString name, Chain chain) :
    id(id), name(name), chain(chain) {}

#include "user.h"
#include <QString>
#include "store.h"


User::User(int id, QString username, Store store) :
    id(id), username(username), store(store) {}

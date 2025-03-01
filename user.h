#ifndef USER_H
#define USER_H
#include <QString>

#include <store.h>

class User
{
public:
    int id;
    QString username;
    Store store;
    User(int id, QString username, Store store);
};


#endif // USER_H

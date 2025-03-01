#ifndef ORDERTABLE_H
#define ORDERTABLE_H

#include <QWidget>

namespace Ui {
class OrderTable;
}

class OrderTable : public QWidget
{
    Q_OBJECT

public:
    explicit OrderTable(const QJsonArray &dataArray, QWidget *parent = nullptr);
    ~OrderTable();

private:
    Ui::OrderTable *ui;
};

#endif // ORDERTABLE_H

#ifndef ORDERSUMMARY_H
#define ORDERSUMMARY_H

#include <QWidget>

namespace Ui {
class OrderSummary;
}

class OrderSummary : public QWidget
{
    Q_OBJECT

public:
    explicit OrderSummary(const QJsonArray &dataArray, QWidget *parent = nullptr);
    ~OrderSummary();

private:
    Ui::OrderSummary *ui;
};

#endif // ORDERSUMMARY_H

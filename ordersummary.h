#ifndef ORDERSUMMARY_H
#define ORDERSUMMARY_H

#include <QWidget>

class OrderSummary : public QWidget
{
    Q_OBJECT

public:
    explicit OrderSummary(const QJsonArray &dataArray, QWidget *parent = nullptr);
    ~OrderSummary();
};

#endif // ORDERSUMMARY_H

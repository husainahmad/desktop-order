#ifndef ORDERCARTWIDGET_H
#define ORDERCARTWIDGET_H

#include <QWidget>
#include "orderitem.h"
#include <QVBoxLayout>
#include "orderform.h"

namespace Ui {
class OrderCartWidget;
}

class OrderCartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderCartWidget(OrderItem orderItem, OrderForm *orderForm, QWidget *parent = nullptr);
    void orderCartSkuWidget(OrderItem &orderItem, QVBoxLayout *&skuLayout);
    double getTotal();
    ~OrderCartWidget();

private slots:
    void onIncreaseClicked(const OrderItem &orderItem, const OrderItemSku &orderItemSku);
    void onDecreaseClicked(const OrderItem &orderItem, const OrderItemSku &orderItemSku);
private:
    Ui::OrderCartWidget *ui;
    OrderForm *orderForm;
    double subTotal;
    QLocale locale;
};

#endif // ORDERCARTWIDGET_H

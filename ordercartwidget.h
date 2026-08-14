#ifndef ORDERCARTWIDGET_H
#define ORDERCARTWIDGET_H

#include <QWidget>
#include "orderitem.h"
#include <QVBoxLayout>
#include "orderform.h"
#include "setting.h"

namespace Ui {
class OrderCartWidget;
}

class OrderCartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderCartWidget(OrderItem orderItem, OrderForm *orderForm, QWidget *parent = nullptr);
    void orderCartSkuWidget(const OrderItem &orderItem, QVBoxLayout *&skuLayout);
    void updateItem(const OrderItem &orderItem);
    double getTotal();
    ~OrderCartWidget();

signals:
    void updateQuantity(const Product &product, const Sku &sku, bool add);

private slots:

private:
    Ui::OrderCartWidget *ui;
    OrderForm *orderForm;
    double subTotal;
    QVBoxLayout *m_skuLayout = nullptr;
    QLocale locale;
    Setting settingConfig;
};

#endif // ORDERCARTWIDGET_H

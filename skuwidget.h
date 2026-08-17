#ifndef SKUWIDGET_H
#define SKUWIDGET_H

#include <QWidget>

#include "product.h"
#include "orderform.h"

class SkuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SkuWidget(Product product, OrderForm *orderForm, QWidget *parent = nullptr);
    ~SkuWidget();
signals:
    void updateQuantity(const Product &product, const Sku &sku, bool add);

private:
    OrderForm *orderForm;
};

#endif // SKUWIDGET_H

#ifndef SKUWIDGET_H
#define SKUWIDGET_H

#include <QWidget>

#include "product.h"
#include "orderform.h"

namespace Ui {
class SkuWidget;
}

class SkuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SkuWidget(Product product, OrderForm *orderForm, QWidget *parent = nullptr);
    ~SkuWidget();

private:
    Ui::SkuWidget *ui;
    OrderForm *orderForm;
};

#endif // SKUWIDGET_H

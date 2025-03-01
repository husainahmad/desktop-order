#ifndef CARTITEMWIDGET_H
#define CARTITEMWIDGET_H

#include <QWidget>
#include "product.h"

namespace Ui {
class CartItemWidget;
}

class CartItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartItemWidget(Product product, QWidget *parent = nullptr);
    ~CartItemWidget();

private:
    Ui::CartItemWidget *ui;
};

#endif // CARTITEMWIDGET_H

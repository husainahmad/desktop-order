#ifndef PRODUCTDETAILPOPUP_H
#define PRODUCTDETAILPOPUP_H

#include <QDialog>
#include <QLocale>
#include <QHash>
#include "product.h"

class QLabel;
class QPushButton;

class OrderForm;

class ProductDetailPopup : public QDialog
{
    Q_OBJECT

public:
    explicit ProductDetailPopup(const Product &product, OrderForm *orderForm, QWidget *parent = nullptr);
    ~ProductDetailPopup();

private slots:
    void addSku(const Sku &sku);
    void removeSku(const Sku &sku);

private:
    void refreshSkuCount(int skuId);
    void setImageFromBase64(QLabel *label, const QString &base64String);

    Product m_product;
    OrderForm *m_orderForm;
    QLocale locale;
    QHash<int, QLabel *> m_countLabels;
    QHash<int, QPushButton *> m_minusButtons;
};

#endif // PRODUCTDETAILPOPUP_H
#include "ordercartwidget.h"
#include "orderitem.h"
#include "orderitemsku.h"
#include "orderform.h"
#include "screenutils.h"
#include <QList>
#include <QHBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

OrderCartWidget::OrderCartWidget(OrderItem orderItem, OrderForm *orderForm, QWidget *parent)
    : QWidget(parent), orderForm(orderForm) {
    locale = QLocale::English;

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setSpacing(6);
    layout->setContentsMargins(6, 6, 6, 6);

    QLabel *productNameLabel = new QLabel(orderItem.productName, this);
    productNameLabel->setStyleSheet(ScreenUtils::qss(
        "font-weight: bold; font-size: 14px; color: #0f172a;"
        "background-color: #eff6ff; border-radius: 8px; padding: 8px 12px;"));

    QVBoxLayout *skuLayout = new QVBoxLayout();
    m_skuLayout = skuLayout;
    orderCartSkuWidget(orderItem, m_skuLayout);

    // Add widgets to main layout
    layout->addWidget(productNameLabel, 3);
    layout->addLayout(m_skuLayout, 7);

    setObjectName("cartCard");

    setLayout(layout);
}

void OrderCartWidget::orderCartSkuWidget(const OrderItem &orderItem, QVBoxLayout *&skuLayout) {
    subTotal = 0;
    for (const OrderItemSku &orderItemSku : std::as_const(orderItem.orderItemSkus)) {
        // Create button layout for quantity control
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->setAlignment(Qt::AlignLeft);
        buttonLayout->setSpacing(2);
        buttonLayout->setContentsMargins(2, 2, 2, 2);

        QLabel *skuNameLabel = new QLabel(orderItemSku.skuName, this);
        skuNameLabel->setStyleSheet(ScreenUtils::qss("font-size: 15px; font-weight: bold;"));
        QLabel *skuPriceLabel = new QLabel("Rp. " + locale.toString(orderItemSku.subTotal, 'f', 0), this);
        skuPriceLabel->setStyleSheet(ScreenUtils::qss("color: #28a745; font-size: 14px; font-weight: bold;"));

        QPushButton *minusButton = new QPushButton("-", this);
        QPushButton *plusButton = new QPushButton("+", this);
        QLabel *totalLabel = new QLabel(locale.toString(orderItemSku.quantity), this);

        totalLabel->setFixedSize(ScreenUtils::px(34), ScreenUtils::px(30));
        totalLabel->setAlignment(Qt::AlignCenter);
        totalLabel->setStyleSheet(ScreenUtils::qss("font-size: 13px; font-weight: bold;"));

        // Style buttons for modern look
        minusButton->setObjectName("qtyButton");
        plusButton->setObjectName("qtyButton");

        int productId = orderItem.productId;
        QString productName = orderItem.productName;
        int categoryId = orderItem.categoryId;

        int skuId = orderItemSku.skuId;
        QString skuName = orderItemSku.skuName;
        double price = orderItemSku.price;

        connect(plusButton, &QPushButton::clicked, this, [=]() {
            Product product(productId, productName, categoryId);
            Sku sku(skuId, skuName, price);
            //orderForm->updateQuantity(product, sku, true);
            emit updateQuantity(product, sku, true);
        });

        connect(minusButton, &QPushButton::clicked, this, [=]() {
            Product product(productId, productName, categoryId);
            Sku sku(skuId, skuName, price);
            //orderForm->updateQuantity(product, sku, false);
            emit updateQuantity(product, sku, false);
        });

        subTotal += orderItemSku.subTotal;

        // Add widgets to layout
        buttonLayout->addWidget(skuNameLabel, 2, Qt::AlignLeft);
        buttonLayout->addWidget(skuPriceLabel, 1, Qt::AlignRight);
        buttonLayout->addWidget(minusButton, 0, Qt::AlignCenter);
        buttonLayout->addWidget(totalLabel, 0, Qt::AlignCenter);
        buttonLayout->addWidget(plusButton, 0, Qt::AlignCenter);

        skuLayout->addLayout(buttonLayout);
    }
}

void OrderCartWidget::updateItem(const OrderItem &orderItem) {
    subTotal = 0;
    if (!m_skuLayout) {
        return;
    }

    while (QLayoutItem *item = m_skuLayout->takeAt(0)) {
        if (QLayout *row = item->layout()) {
            while (QLayoutItem *child = row->takeAt(0)) {
                if (child->widget()) {
                    child->widget()->deleteLater();
                }
                delete child;
            }
        }
        delete item;
    }

    orderCartSkuWidget(orderItem, m_skuLayout);
}

double OrderCartWidget::getTotal() {
    return subTotal;
}

OrderCartWidget::~OrderCartWidget()
{
}

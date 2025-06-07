#include "ordercartwidget.h"
#include "ui_ordercartwidget.h"
#include "orderitem.h"
#include "orderitemsku.h"
#include "orderform.h"
#include <QList>
#include <QHBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

OrderCartWidget::OrderCartWidget(OrderItem orderItem, OrderForm *orderForm, QWidget *parent)
    : QWidget(parent), ui(new Ui::OrderCartWidget), orderForm(orderForm) {
    ui->setupUi(this);

    locale = QLocale::English;

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setSpacing(5);
    layout->setContentsMargins(10, 10, 10, 10);

    QLabel *productNameLabel = new QLabel(orderItem.productName, this);
    productNameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    QVBoxLayout *skuLayout = new QVBoxLayout();
    orderCartSkuWidget(orderItem, skuLayout);

    // Add widgets to main layout
    layout->addWidget(productNameLabel, 3);
    layout->addLayout(skuLayout, 7);

    setLayout(layout);
}

void OrderCartWidget::orderCartSkuWidget(OrderItem &orderItem, QVBoxLayout *&skuLayout) {
    subTotal = 0;
    for (const OrderItemSku &orderItemSku : std::as_const(orderItem.orderItemSkus)) {
        // Create button layout for quantity control
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->setAlignment(Qt::AlignLeft);
        buttonLayout->setSpacing(5);
        buttonLayout->setContentsMargins(5, 5, 5, 5);

        QLabel *skuNameLabel = new QLabel(orderItemSku.skuName, this);
        QLabel *skuPriceLabel = new QLabel("Rp. " + locale.toString(orderItemSku.subTotal, 'f', 0), this);
        skuPriceLabel->setStyleSheet("color: #28a745; font-size: 12px; font-weight: bold;");

        QPushButton *minusButton = new QPushButton("-", this);
        QPushButton *plusButton = new QPushButton("+", this);
        QLabel *totalLabel = new QLabel(locale.toString(orderItemSku.quantity), this);

        totalLabel->setFixedSize(30, 40);
        totalLabel->setAlignment(Qt::AlignCenter);
        totalLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

        // Style buttons for modern look
        QString buttonStyle =
            "QPushButton {"
            "   background-color: #007bff;"
            "   color: white;"
            "   font-size: 16px;"
            "   border-radius: 10px;"
            "   width: 30px;"
            "   height: 30px;"
            "}"
            "QPushButton:hover { background-color: #0056b3; }"
            "QPushButton:pressed { background-color: #004494; }";

        minusButton->setStyleSheet(buttonStyle);
        plusButton->setStyleSheet(buttonStyle);

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

double OrderCartWidget::getTotal() {
    return subTotal;
}

OrderCartWidget::~OrderCartWidget()
{
    delete ui;
}

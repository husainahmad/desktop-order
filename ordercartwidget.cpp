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

        minusButton->setFixedSize(40, 40);
        plusButton->setFixedSize(40, 40);
        totalLabel->setFixedSize(30, 40);
        totalLabel->setAlignment(Qt::AlignCenter);
        totalLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

        // Style buttons for modern look
        QString buttonStyle =
            "QPushButton {"
            "   background-color: #007bff;"
            "   color: white;"
            "   font-size: 18px;"
            "   border-radius: 10px;"
            "   width: 40px;"
            "   height: 40px;"
            "}"
            "QPushButton:hover { background-color: #0056b3; }"
            "QPushButton:pressed { background-color: #004494; }";

        minusButton->setStyleSheet(buttonStyle);
        plusButton->setStyleSheet(buttonStyle);

        // Fix lambda parameter copying issue (Pass by Reference)
        connect(plusButton, &QPushButton::clicked, this, [this, orderItem, orderItemSku]() {
            onIncreaseClicked(orderItem, orderItemSku);
        });

        connect(minusButton, &QPushButton::clicked, this, [this, orderItem, orderItemSku]() {
            onDecreaseClicked(orderItem, orderItemSku);
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

void OrderCartWidget::onIncreaseClicked(const OrderItem &orderItem, const OrderItemSku &orderItemSku) {
    Product product(orderItem.productId, orderItem.productName, orderItem.categoryId);
    Sku sku(orderItemSku.skuId, orderItemSku.skuName, orderItemSku.price);

    if (orderForm) {
        emit orderForm->updateQuantity(product, sku, true);
    } else {
        qDebug() << "Error: orderForm is nullptr!";
    }
}

void OrderCartWidget::onDecreaseClicked(const OrderItem &orderItem, const OrderItemSku &orderItemSku) {
    Product product(orderItem.productId, orderItem.productName, orderItem.categoryId);
    Sku sku(orderItemSku.skuId, orderItemSku.skuName, orderItemSku.price);
    if (orderForm) {
        emit orderForm->updateQuantity(product, sku, false);
    } else {
        qDebug() << "Error: orderForm is nullptr!";
    }
}

double OrderCartWidget::getTotal() {
    return subTotal;
}

OrderCartWidget::~OrderCartWidget()
{
    delete ui;
}

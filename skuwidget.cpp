#include "skuwidget.h"
#include "ui_skuwidget.h"
#include "product.h"
#include "sku.h"

#include <QVBoxLayout>
#include <QPushButton>
#include "orderform.h"


SkuWidget::SkuWidget(Product product, OrderForm *orderForm, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SkuWidget), orderForm(orderForm)
{
    ui->setupUi(this);

    QVBoxLayout *skuLayout = new QVBoxLayout(this);
    skuLayout->setContentsMargins(10, 10, 10, 10);
    skuLayout->setSpacing(10);
    skuLayout->setAlignment(Qt::AlignCenter);
    for (const Sku &sku: product.skus) {
        QString buttonString = "Add " + sku.name;
        QPushButton *addButton = new QPushButton(buttonString);
        addButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #28a745;"  // Green color
            "   color: white;"
            "   font-size: 14px;"
            "   font-weight: bold;"
            "   border-radius: 8px;"
            "   padding: 8px 16px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #218838;"  // Darker green on hover
            "}"
            "QPushButton:pressed {"
            "   background-color: #1e7e34;"  // Even darker green on press
            "}"
            );

        addButton->setFixedSize(150, 40);
        connect(addButton, &QPushButton::clicked, this, [orderForm, product, sku]() {
            emit orderForm->updateQuantity(product, sku, true);
        });
        skuLayout->addWidget(addButton);
        skuLayout->setAlignment(Qt::AlignCenter);
    }

    setLayout(skuLayout);
}

SkuWidget::~SkuWidget()
{
    delete ui;
}

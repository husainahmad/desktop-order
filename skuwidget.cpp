#include "skuwidget.h"
#include "ui_skuwidget.h"
#include "product.h"
#include "sku.h"
#include "screenutils.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QFontMetrics>
#include "orderform.h"


SkuWidget::SkuWidget(Product product, OrderForm *orderForm, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SkuWidget), orderForm(orderForm)
{
    ui->setupUi(this);

    QVBoxLayout *skuLayout = new QVBoxLayout(this);
    skuLayout->setContentsMargins(ScreenUtils::px(10), ScreenUtils::px(10), ScreenUtils::px(10), ScreenUtils::px(10));
    skuLayout->setSpacing(ScreenUtils::px(10));
    skuLayout->setAlignment(Qt::AlignCenter);
    for (const Sku &sku: product.skus) {
        QString buttonString = sku.name;
        QPushButton *addButton = new QPushButton(buttonString);
        addButton->setObjectName("successButton");
        addButton->setFixedSize(ScreenUtils::px(110), ScreenUtils::px(36));

        QFontMetrics fm(addButton->font());
        const int horizontalPadding = ScreenUtils::px(18) * 2;
        const int maxTextWidth = addButton->width() - horizontalPadding;
        if (fm.horizontalAdvance(buttonString) > maxTextWidth) {
            addButton->setText(fm.elidedText(buttonString, Qt::ElideRight, maxTextWidth));
        }
        addButton->setToolTip(buttonString);

        int productId = product.id;
        QString productName = product.name;
        int categoryId = product.categoryId;

        int skuId = sku.id;
        QString skuName = sku.name;
        double price = sku.price;

        connect(addButton, &QPushButton::clicked, this, [=]() {
            Product product(productId, productName, categoryId);
            Sku sku(skuId, skuName, price);
            emit updateQuantity(product, sku, true);
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

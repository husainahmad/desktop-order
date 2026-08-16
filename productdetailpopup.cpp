#include "productdetailpopup.h"
#include "orderform.h"
#include "screenutils.h"
#include "touchutils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QScrollArea>

ProductDetailPopup::ProductDetailPopup(const Product &product, OrderForm *orderForm, QWidget *parent)
    : QDialog(parent), m_product(product), m_orderForm(orderForm)
{
    setWindowTitle("Product Details");

    const int skuCount = m_product.skus.size();
    const int dialogWidth = ScreenUtils::px(560);
    const int headerHeight = ScreenUtils::px(142);
    const int sectionHeight = ScreenUtils::px(30);
    const int rowHeight = ScreenUtils::px(44);
    const int panelMargins = ScreenUtils::px(48);
    const int footerHeight = ScreenUtils::px(56);
    const int chromeHeight = ScreenUtils::px(96);
    const int contentHeight = headerHeight + sectionHeight + panelMargins
                              + skuCount * rowHeight + footerHeight + chromeHeight;
    const int maxHeight = qMax(240, ScreenUtils::availableHeight() - ScreenUtils::px(60));
    setFixedSize(dialogWidth, qMin(contentHeight, maxHeight));

    locale = QLocale::English;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(ScreenUtils::px(12));
    mainLayout->setContentsMargins(ScreenUtils::px(16), ScreenUtils::px(16),
                                   ScreenUtils::px(16), ScreenUtils::px(16));

    // ======================= Product detail header =======================
    QWidget *headerCard = new QWidget(this);
    headerCard->setObjectName("cardPanel");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(ScreenUtils::px(16), ScreenUtils::px(16),
                                     ScreenUtils::px(16), ScreenUtils::px(16));
    headerLayout->setSpacing(ScreenUtils::px(16));

    QLabel *imageLabel = new QLabel(headerCard);
    imageLabel->setFixedSize(ScreenUtils::px(110), ScreenUtils::px(110));
    imageLabel->setAlignment(Qt::AlignCenter);

    if (m_product.productImage.imageBlob.isEmpty()) {
        QPixmap pixmap(m_product.imagePath);
        imageLabel->setPixmap(pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        setImageFromBase64(imageLabel, m_product.productImage.imageBlob);
    }

    QLabel *nameLabel = new QLabel(m_product.name, headerCard);
    nameLabel->setObjectName("sectionHeader");
    nameLabel->setWordWrap(true);

    QVBoxLayout *nameLayout = new QVBoxLayout();
    nameLayout->addWidget(nameLabel);
    nameLayout->addStretch();

    headerLayout->addWidget(imageLabel, 0, Qt::AlignVCenter);
    headerLayout->addLayout(nameLayout, 1);
    headerCard->setLayout(headerLayout);

    mainLayout->addWidget(headerCard);

    // ======================= SKU section =======================
    QLabel *skuSectionLabel = new QLabel("Select Quantity", this);
    skuSectionLabel->setObjectName("sectionHeader");
    mainLayout->addWidget(skuSectionLabel);

    QWidget *skuPanel = new QWidget(this);
    skuPanel->setObjectName("cardPanel");
    QVBoxLayout *skuLayout = new QVBoxLayout(skuPanel);
    skuLayout->setContentsMargins(ScreenUtils::px(12), ScreenUtils::px(12),
                                  ScreenUtils::px(12), ScreenUtils::px(12));
    skuLayout->setSpacing(ScreenUtils::px(10));

    for (const Sku &sku : m_product.skus) {
        QHBoxLayout *row = new QHBoxLayout();
        row->setSpacing(ScreenUtils::px(10));

        QPushButton *minusButton = new QPushButton("-", skuPanel);
        minusButton->setObjectName("qtyButton");
        minusButton->setFixedSize(ScreenUtils::px(34), ScreenUtils::px(34));

        QLabel *skuInfoLabel = new QLabel(QString("%1\nRp %2")
                                              .arg(sku.name, locale.toString(sku.price, 'f', 0)),
                                          skuPanel);
        skuInfoLabel->setStyleSheet(ScreenUtils::qss(
            "font-size: 14px; font-weight: 600; color: #1f2937;"));

        QLabel *countLabel = new QLabel("0", skuPanel);
        countLabel->setAlignment(Qt::AlignCenter);
        countLabel->setFixedWidth(ScreenUtils::px(40));
        countLabel->setStyleSheet(ScreenUtils::qss(
            "font-size: 15px; font-weight: bold; color: #0f172a;"));

        QPushButton *plusButton = new QPushButton("+", skuPanel);
        plusButton->setObjectName("qtyButton");
        plusButton->setFixedSize(ScreenUtils::px(34), ScreenUtils::px(34));

        connect(minusButton, &QPushButton::clicked, this, [this, sku]() {
            removeSku(sku);
        });
        connect(plusButton, &QPushButton::clicked, this, [this, sku]() {
            addSku(sku);
        });

        row->addWidget(minusButton, 0, Qt::AlignLeft);
        row->addWidget(skuInfoLabel, 1, Qt::AlignLeft);
        row->addWidget(countLabel, 0, Qt::AlignRight);
        row->addWidget(plusButton, 0, Qt::AlignRight);

        skuLayout->addLayout(row);

        m_countLabels.insert(sku.id, countLabel);
        m_minusButtons.insert(sku.id, minusButton);
    }

    skuPanel->setLayout(skuLayout);

    QScrollArea *skuScrollArea = new QScrollArea(this);
    skuScrollArea->setWidgetResizable(true);
    skuScrollArea->setWidget(skuPanel);
    skuScrollArea->setFrameShape(QFrame::NoFrame);
    TouchUtils::enableTouchScrolling(skuScrollArea);

    mainLayout->addWidget(skuScrollArea, 1);

    // ======================= Close button =======================
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton *closeButton = new QPushButton("Close", this);
    closeButton->setObjectName("secondaryButton");
    closeButton->setFixedSize(ScreenUtils::px(110), ScreenUtils::px(40));
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);

    for (const Sku &sku : m_product.skus) {
        refreshSkuCount(sku.id);
    }
}

ProductDetailPopup::~ProductDetailPopup()
{
}

void ProductDetailPopup::addSku(const Sku &sku)
{
    if (m_orderForm) {
        m_orderForm->updateQuantity(m_product, sku, true);
    }
    refreshSkuCount(sku.id);
}

void ProductDetailPopup::removeSku(const Sku &sku)
{
    if (m_orderForm) {
        m_orderForm->updateQuantity(m_product, sku, false);
    }
    refreshSkuCount(sku.id);
}

void ProductDetailPopup::refreshSkuCount(int skuId)
{
    QLabel *countLabel = m_countLabels.value(skuId, nullptr);
    if (!countLabel) {
        return;
    }

    int count = m_orderForm ? m_orderForm->getSkuQuantity(m_product.id, skuId) : 0;
    countLabel->setText(QString::number(count));

    QPushButton *minusButton = m_minusButtons.value(skuId, nullptr);
    if (minusButton) {
        minusButton->setEnabled(count > 0);
    }
}

void ProductDetailPopup::setImageFromBase64(QLabel *label, const QString &base64String)
{
    QByteArray byteArray = QByteArray::fromBase64(base64String.toUtf8());

    QImage image;
    if (!image.loadFromData(byteArray)) {
        qDebug() << "Failed to load image from Base64 data.";
        return;
    }

    label->setPixmap(QPixmap::fromImage(image));
    label->setFixedSize(ScreenUtils::px(110), ScreenUtils::px(110));
    label->setScaledContents(true);
}
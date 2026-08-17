#include "productwidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QLocale>
#include <QGraphicsDropShadowEffect>
#include "product.h"
#include "sku.h"
#include "screenutils.h"

ProductWidget::ProductWidget(Product product, QWidget *parent)
    : QWidget(parent)
{
    locale = QLocale::English;

    setObjectName("productCard");
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
    setFixedSize(ScreenUtils::px(150), ScreenUtils::px(196));

    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(ScreenUtils::px(8));
    shadowEffect->setOffset(0, ScreenUtils::px(2));
    shadowEffect->setColor(QColor(15, 23, 42, 30));
    setGraphicsEffect(shadowEffect);

    // Create layout
    QVBoxLayout *productLayout = new QVBoxLayout(this);
    productLayout->setContentsMargins(ScreenUtils::px(8), ScreenUtils::px(8),
                                      ScreenUtils::px(8), ScreenUtils::px(8));
    productLayout->setSpacing(ScreenUtils::px(5));

    // Create image label
    QLabel *imageLabel = new QLabel(this);
    imageLabel->setFixedSize(ScreenUtils::px(110), ScreenUtils::px(110));
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet(ScreenUtils::qss(
        "background-color: #f8fafc; border-radius: 12px;"));
    m_imageLabel = imageLabel;

    // Load image
    if (product.productImage.imageBlob.isEmpty()) {
        QPixmap pixmap(product.imagePath);
        imageLabel->setPixmap(pixmap.scaled(ScreenUtils::px(96), ScreenUtils::px(96),
                                            Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        setImageFromBase64(imageLabel, product.productImage.imageBlob);
    }

    // Create name label
    QLabel *nameLabel = new QLabel(product.name, this);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    nameLabel->setMinimumHeight(ScreenUtils::px(20));
    nameLabel->setStyleSheet(ScreenUtils::qss(
        "font-size: 13px; font-weight: 600; color: #0f172a; background: transparent;"));

    // Create price label (starting price from the lowest SKU)
    double minPrice = 0;
    if (!product.skus.isEmpty()) {
        minPrice = product.skus.first().price;
        for (const Sku &sku : product.skus) {
            if (sku.price < minPrice) {
                minPrice = sku.price;
            }
        }
    }

    QLabel *priceLabel = new QLabel("Rp " + locale.toString(minPrice, 'f', 0), this);
    priceLabel->setAlignment(Qt::AlignCenter);
    priceLabel->setStyleSheet(ScreenUtils::qss(
        "color: #16a34a; font-size: 12px; font-weight: 700; background: transparent;"
        "border: 1px solid #bbf7d0; border-radius: 8px; padding: 1px 4px;"));
    if (product.skus.isEmpty()) {
        priceLabel->hide();
    }

    // Hover indicator badge shown at the top-right of the image
    hoverBadge = new QLabel("+", imageLabel);
    hoverBadge->setFixedSize(ScreenUtils::px(34), ScreenUtils::px(34));
    hoverBadge->setAlignment(Qt::AlignCenter);
    hoverBadge->setStyleSheet(ScreenUtils::qss(
        "background-color: #2563eb; color: #ffffff; border: 2px solid #ffffff;"
        "border-radius: 17px; font-size: 20px; font-weight: bold;"));
    hoverBadge->move(imageLabel->width() - hoverBadge->width() - ScreenUtils::px(6),
                     ScreenUtils::px(6));
    hoverBadge->hide();

    // Add widgets to layout
    productLayout->addWidget(imageLabel, 0, Qt::AlignHCenter);
    productLayout->addWidget(nameLabel, 1, Qt::AlignHCenter);
    productLayout->addWidget(priceLabel, 0, Qt::AlignHCenter);

    // Apply card style
    setStyleSheet(ScreenUtils::qss(
        "QWidget#productCard {"
        "   border: 1px solid #e2e8f0;"
        "   border-radius: 12px;"
        "   background-color: #ffffff;"
        "   padding: 2px;"
        "}"
        "QWidget#productCard:hover {"
        "   border: 2px solid #2563eb;"
        "   background-color: #dbeafe;"
        "}"
        "QWidget#productCard:pressed {"
        "   background-color: #bfdbfe;"
        "}"
        ));

    setLayout(productLayout);
}

void ProductWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}

void ProductWidget::enterEvent(QEnterEvent *event)
{
    if (hoverBadge) {
        hoverBadge->raise();
        hoverBadge->show();
    }
    if (m_imageLabel) {
        m_imageLabel->setStyleSheet(ScreenUtils::qss(
            "background-color: #dbeafe; border-radius: 12px;"));
    }
    QWidget::enterEvent(event);
}

void ProductWidget::leaveEvent(QEvent *event)
{
    if (hoverBadge) {
        hoverBadge->hide();
    }
    if (m_imageLabel) {
        m_imageLabel->setStyleSheet(ScreenUtils::qss(
            "background-color: #f8fafc; border-radius: 12px;"));
    }
    QWidget::leaveEvent(event);
}

void ProductWidget::setImageFromBase64(QLabel *label, const QString &base64String) {
    // Convert Base64 QString to QByteArray
    QByteArray byteArray = QByteArray::fromBase64(base64String.toUtf8());

    // Load image from QByteArray
    QImage image;
    if (!image.loadFromData(byteArray)) {
        qDebug() << "Failed to load image from Base64 data.";
        return;
    }

    // Convert QImage to QPixmap and set it to QLabel
    label->setPixmap(QPixmap::fromImage(image));
    label->setFixedSize(ScreenUtils::px(110), ScreenUtils::px(110));
    label->setScaledContents(true);  // Ensure image scales properly in QLabel
}

ProductWidget::~ProductWidget()
{
}

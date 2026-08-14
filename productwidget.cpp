#include "productwidget.h"
#include "ui_productwidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QLocale>
#include "product.h"
#include "screenutils.h"

ProductWidget::ProductWidget(Product product, QWidget *parent)
    : QWidget(parent), ui(new Ui::ProductWidget)
{
    ui->setupUi(this);
    locale = QLocale::English;

    // Create layout
    QVBoxLayout *productLayout = new QVBoxLayout(this);
    productLayout->setContentsMargins(ScreenUtils::px(5), ScreenUtils::px(5), ScreenUtils::px(5), ScreenUtils::px(5));
    productLayout->setSpacing(ScreenUtils::px(5));

    // Create image label
    QLabel *imageLabel = new QLabel(this);
    imageLabel->setFixedSize(ScreenUtils::px(100), ScreenUtils::px(100)); // Ensure consistent size
    imageLabel->setAlignment(Qt::AlignCenter);

    // Load image
    if (product.productImage.imageBlob.isEmpty()) {
        QPixmap pixmap(product.imagePath);
        imageLabel->setPixmap(pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        setImageFromBase64(imageLabel, product.productImage.imageBlob);
    }

    // Create name label
    QLabel *nameLabel = new QLabel(product.name, this);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true); // Allow multi-line product names
    nameLabel->setStyleSheet(ScreenUtils::qss("font-size: 14px; font-weight: bold; color: #1f2937;"));

    // Add widgets to layout
    productLayout->addWidget(imageLabel);
    productLayout->addWidget(nameLabel);
    productLayout->setAlignment(Qt::AlignCenter);

    // Apply card style (hover effect is handled by the global theme)
    setObjectName("productCard");
    setStyleSheet(ScreenUtils::qss(
        "QWidget#productCard {"
        "   border: 1px solid #e2e8f0;"
        "   border-radius: 12px;"
        "   background-color: #ffffff;"
        "   padding: 10px;"
        "}"
        "QWidget#productCard:hover {"
        "   border: 1px solid #2563eb;"
        "   background-color: #f0f6ff;"
        "}"
        "QLabel {"
        "   background: transparent;"
        "}"
        ));

    setLayout(productLayout);
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
    delete ui;
}

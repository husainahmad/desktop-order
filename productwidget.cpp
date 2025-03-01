#include "productwidget.h"
#include "ui_productwidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QLocale>
#include "product.h"

ProductWidget::ProductWidget(Product product, QWidget *parent)
    : QWidget(parent), ui(new Ui::ProductWidget)
{
    ui->setupUi(this);
    locale = QLocale::English;

    // Create layout
    QVBoxLayout *productLayout = new QVBoxLayout(this);
    productLayout->setContentsMargins(5, 5, 5, 5);
    productLayout->setSpacing(5);

    // Create image label
    QLabel *imageLabel = new QLabel(this);
    imageLabel->setFixedSize(100, 100); // Ensure consistent size
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
    nameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");

    // Add widgets to layout
    productLayout->addWidget(imageLabel);
    productLayout->addWidget(nameLabel);
    productLayout->setAlignment(Qt::AlignCenter);

    // Apply styles
    setStyleSheet(
        "QWidget {"
        "   border: 1px solid #ccc;"
        "   border-radius: 8px;"
        "   background-color: #f9f9f9;"
        "   padding: 10px;"
        "}"
        "QLabel {"
        "   font-family: Arial;"
        "}"
        );

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
    label->setFixedSize(110, 110);
    label->setScaledContents(true);  // Ensure image scales properly in QLabel
}

ProductWidget::~ProductWidget()
{
    delete ui;
}

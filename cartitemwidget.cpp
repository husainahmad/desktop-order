#include "cartitemwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

CartItemWidget::CartItemWidget(Product product, QWidget *parent)
    : QWidget(parent), ui(new Ui::CartItemWidget)
{
    ui->setupUi(this);

    // Create main layout
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setSpacing(0);

    // Create product info labels
    QLabel *productNameLabel = new QLabel(product.name, this);

    // Create button layout for quantity control
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setAlignment(Qt::AlignTop);
    buttonLayout->setSpacing(0);

    QPushButton *minusButton = new QPushButton("-", this);
    QPushButton *plusButton = new QPushButton("+", this);
    QLabel *totalLabel = new QLabel("1", this); // Default quantity

    // Add widgets to button layout
    buttonLayout->addWidget(minusButton);
    buttonLayout->addWidget(totalLabel);
    buttonLayout->addWidget(plusButton);

    // Add widgets to main layout
    layout->addWidget(productNameLabel);
    layout->addLayout(buttonLayout);

    connect(plusButton, &QPushButton::clicked, this, [totalLabel]() {
        int count = totalLabel->text().toInt();
        totalLabel->setText(QString::number(count + 1));
    });

    connect(minusButton, &QPushButton::clicked, this, [totalLabel]() {
        int count = totalLabel->text().toInt();
        if (count > 1) {
            totalLabel->setText(QString::number(count - 1));
        }
    });


    // Set layout to the widget
    setLayout(layout);

}

CartItemWidget::~CartItemWidget()
{
    delete ui;
}

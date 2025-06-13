#include "ordertablewidget.h"
#include "ui_ordertablewidget.h"
#include "orderpopupwindow.h"

#include <QBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>

#include <QDateTime>
#include <QTimeZone>
#include <Qt>
#include <QStatusBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDateEdit>
#include <QJsonObject>
#include <QJsonArray>
#include <QPushButton>

OrderTableWidget::OrderTableWidget(const QJsonArray &dataArray, QTabWidget *tabWidget, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderTableWidget), tabWidget(tabWidget)
{
    ui->setupUi(this);

    double qrTotal = 0;
    double cashTotal = 0;
    double cardTotal = 0;
    double grandTotal = 0;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);

    QLabel *titleLabel = new QLabel("", this);
    titleLabel->setStyleSheet("font-size: 15px; color: white; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    QTableWidget *tableWidget = new QTableWidget(1, 9, this);

    tableWidget->setStyleSheet(
        "QHeaderView::section {"
        "   background-color: black;"  // Black background
        "   color: white;"              // White text
        "   font-weight: bold;"         // Bold font
        "   font-size: 14px;"           // Font size
        "   padding: 6px;"              // Padding inside header cells
        "   border: 1px solid #444;"    // Slight border for separation
        "}"
        );

    // Set header labels
    tableWidget->setHorizontalHeaderLabels({"Order No", "Date", "Customer", "Payment", "Status", "Sub Total", "Disc", "Total", "Action"});

    // Resize mode for proper spacing
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);

    tableWidget->setRowCount(dataArray.size());  // Adjust row count dynamically

    double total = 0;
    for (int i = 0; i < dataArray.size(); ++i) {
        QJsonObject orderObj = dataArray[i].toObject();

        QString orderNo = orderObj["orderNo"].toString();
        QString customerName = orderObj["customerName"].toString();
        QString statusOrder = orderObj["status"].toString();
        double subTotal = orderObj["subTotal"].toDouble();
        double discountTotal = orderObj["discountTotal"].toDouble();
        double grandTotal = orderObj["grandTotal"].toDouble();

        int paymentId = 0; // UnPaid

        QJsonObject paymentObject = orderObj["orderPayment"].toObject();

        if (!paymentObject.isEmpty()) {
            paymentId = paymentObject["paymentId"].toInt();
            if (paymentId==1) {
                cashTotal += orderObj["grandTotal"].toDouble();
            } else if (paymentId==2) {
                qrTotal += orderObj["grandTotal"].toDouble();
            } else if (paymentId==3) {
                cardTotal += orderObj["grandTotal"].toDouble();
            }
        }

        total += grandTotal;
        // Parse "createdAt" safely
        QString createdAtStr = orderObj.contains("createdAt") && orderObj["createdAt"].isString() ? orderObj["createdAt"].toString() : "";
        QDateTime createdAt;
        if (!createdAtStr.isEmpty()) {
            createdAt = QDateTime::fromString(createdAtStr, Qt::ISODate);
            createdAt.setTimeZone(QTimeZone::utc());  // Assume the input is in UTC
            createdAt = createdAt.toTimeZone(QTimeZone("Asia/Jakarta"));  // Convert to WIB (UTC+7)
        } else {
            createdAt = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone("Asia/Jakarta"));  // Default to WIB
        }

        tableWidget->setItem(i, 0, new QTableWidgetItem(orderNo));
        tableWidget->setItem(i, 1, new QTableWidgetItem(createdAt.toString("yyyy-MM-dd HH:mm")));
        tableWidget->setItem(i, 2, new QTableWidgetItem(customerName));

        QString paymentMethod;
        switch (paymentId) {
            case 1: paymentMethod = "CASH"; break;
            case 2: paymentMethod = "QR"; break;
            case 3: paymentMethod = "CARD"; break;
            default: paymentMethod = ""; break;
        }

        tableWidget->setItem(i, 3, new QTableWidgetItem(paymentMethod));

        tableWidget->setItem(i, 4, new QTableWidgetItem(statusOrder));

        QTableWidgetItem *subTotalItem = new QTableWidgetItem("Rp." + QLocale(QLocale::English).toString(subTotal, 'f', 0));
        QTableWidgetItem *discountItem = new QTableWidgetItem("Rp." + QLocale(QLocale::English).toString(discountTotal, 'f', 0));
        QTableWidgetItem *totalItem = new QTableWidgetItem("Rp." + QLocale(QLocale::English).toString(grandTotal, 'f', 0));

        // Align numeric values to the right
        subTotalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        discountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        // Make items read-only
        subTotalItem->setFlags(subTotalItem->flags() & ~Qt::ItemIsEditable);
        discountItem->setFlags(discountItem->flags() & ~Qt::ItemIsEditable);
        totalItem->setFlags(totalItem->flags() & ~Qt::ItemIsEditable);

        // Insert into table
        tableWidget->setItem(i, 5, subTotalItem);
        tableWidget->setItem(i, 6, discountItem);
        tableWidget->setItem(i, 7, totalItem);

        QPushButton *button = new QPushButton("Detail");

        button->setStyleSheet(
            "QPushButton {"
            "   background-color: #007bff;"  // Blue background
            "   color: white;"                // White text
            "   border-radius: 5px;"          // Rounded corners
            "   padding: 5px 10px;"           // Padding inside button
            "   font-size: 14px;"             // Font size
            "}"
            "QPushButton:hover {"
            "   background-color: #0056b3;"   // Darker blue on hover
            "}"
            "QPushButton:pressed {"
            "   background-color: #004494;"   // Even darker blue when pressed
            "}"
            );

        QObject::connect(button, &QPushButton::clicked, [this, orderObj]() {
            OrderPopupWindow popup(orderObj, this->tabWidget);
            popup.exec();
        });

        // Create a widget to hold the button
        QWidget *container = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(container);
        layout->addWidget(button);
        layout->setAlignment(Qt::AlignCenter);
        layout->setContentsMargins(0, 0, 0, 0);
        container->setLayout(layout);
        tableWidget->setCellWidget(i, 8, container);
        tableWidget->setAlternatingRowColors(true);
    }

    QString titleString = "Total Order : " + QLocale(QLocale::English).toString(total, 'f', 0)  +
                          " | CASH : Rp. " + QLocale(QLocale::English).toString(cashTotal, 'f', 0) +
                          " | QRIS : Rp. " + QLocale(QLocale::English).toString(qrTotal, 'f', 0) +
                          " | CARD : Rp. " + QLocale(QLocale::English).toString(cardTotal, 'f', 0) +
                          " | Total Amount : Rp. " + QLocale(QLocale::English).toString(grandTotal, 'f', 0);
    titleLabel->setText(titleString);

    // Add table to layout
    mainLayout->addWidget(tableWidget);

    // Step 5: Assign Layout
    this->setLayout(mainLayout);
}

OrderTableWidget::~OrderTableWidget()
{
    delete ui;
}

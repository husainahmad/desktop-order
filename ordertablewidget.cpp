#include "ordertablewidget.h"
#include "ui_ordertablewidget.h"
#include "orderpopupwindow.h"
#include "touchutils.h"
#include "screenutils.h"

#include <QBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QAbstractItemView>

#include <QDateTime>
#include <QTimeZone>
#include <Qt>
#include <QStatusBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDateEdit>
#include <QJsonObject>
#include <QJsonArray>

OrderTableWidget::OrderTableWidget(const QJsonArray &dataArray, QTabWidget *tabWidget, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderTableWidget), tabWidget(tabWidget)
{
    ui->setupUi(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);

    QTableWidget *tableWidget = new QTableWidget(1, 8, this);

    // Set header labels
    tableWidget->setHorizontalHeaderLabels({"Order No", "Date", "Customer", "Payment", "Status", "Sub Total", "Disc", "Total"});

    // Resize mode for proper spacing
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    TouchUtils::enableItemViewScrolling(tableWidget);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setAlternatingRowColors(true);

    connect(tableWidget, &QTableWidget::cellClicked, this, [this, tableWidget](int row, int) {
        QTableWidgetItem *item = tableWidget->item(row, 0);
        if (!item) {
            return;
        }
        OrderPopupWindow popup(item->data(Qt::UserRole).toJsonObject(), this->tabWidget);
        popup.exec();
    });

    tableWidget->setRowCount(dataArray.size());  // Adjust row count dynamically

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
        }

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
        tableWidget->item(i, 0)->setData(Qt::UserRole, orderObj);
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
    }

    // Add table to layout
    mainLayout->addWidget(tableWidget);

    // Step 5: Assign Layout
    this->setLayout(mainLayout);
}

OrderTableWidget::~OrderTableWidget()
{
    delete ui;
}

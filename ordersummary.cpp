#include "ordersummary.h"
#include "ui_ordersummary.h"

#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QTableWidget>
#include <QHeaderView>
#include <QPair>
#include <QString>
#include <QVBoxLayout>
#include <QLabel>

OrderSummary::OrderSummary(const QJsonArray &dataArray, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderSummary)
{
    ui->setupUi(this);

    // Step 1: Extract & Aggregate Sales Data
    QMap<QString, double> itemSalesMap;

    double qrTotal = 0;
    double cashTotal = 0;
    double cardTotal = 0;
    double grandTotal = 0;

    for (int i = 0; i < dataArray.size(); ++i) {
        QJsonObject orderObj = dataArray[i].toObject();
        QJsonArray itemsArray = orderObj["orderDetails"].toArray();  // Assuming items are in an array
        grandTotal += orderObj["grandTotal"].toDouble();

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

        for (const QJsonValue &itemValue : itemsArray) {
            QJsonObject itemObj = itemValue.toObject();
            QString itemName = itemObj["productName"].toString();

            QJsonArray itemSkusArray = itemObj["orderDetailSkus"].toArray();
            double itemTotal = 0;

            for (const QJsonValue &itemSkuValue : itemSkusArray) {
                QJsonObject itemSkuObj = itemSkuValue.toObject();

                itemTotal += itemSkuObj["quantity"].toDouble();
            }

            // Aggregate total sales per item
            itemSalesMap[itemName] += itemTotal;
        }
    }

    // Step 2: Sort & Get Top 10 Items
    QList<QPair<QString, double>> sortedItemSales;
    for (auto it = itemSalesMap.begin(); it != itemSalesMap.end(); ++it) {
        sortedItemSales.append(qMakePair(it.key(), it.value()));
    }

    std::sort(sortedItemSales.begin(), sortedItemSales.end(), [](const QPair<QString, double> &a, const QPair<QString, double> &b) {
        return a.second > b.second;  // Descending order
    });

    // Keep only top 10 items
    if (sortedItemSales.size() > 10) {
        sortedItemSales = sortedItemSales.mid(0, 10);
    }

    QString formattedCount = QLocale(QLocale::English).toString(dataArray.size());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);

    QString titleString = "🔝 Top 10 Best-Selling Items | Total Order : " + formattedCount + " | CASH : Rp. " + QLocale(QLocale::English).toString(cashTotal, 'f', 0) +
                          " | QRIS : Rp. " + QLocale(QLocale::English).toString(qrTotal, 'f', 0) +
                          " | CARD : Rp. " + QLocale(QLocale::English).toString(cardTotal, 'f', 0) +
                          " | Total Amount : Rp. " + QLocale(QLocale::English).toString(grandTotal, 'f', 0);

    QLabel *titleLabel = new QLabel(titleString, this);
    titleLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #333;");
    mainLayout->addWidget(titleLabel);

    // Step 4: Display Summary Table
    QTableWidget *summaryTable = new QTableWidget(this);
    summaryTable->setRowCount(sortedItemSales.size());
    summaryTable->setColumnCount(2);
    summaryTable->setHorizontalHeaderLabels({"Product Name", "Total QTY"});
    summaryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Populate the summary table
    for (int i = 0; i < sortedItemSales.size(); ++i) {

        QTableWidgetItem *qtyOrder = new QTableWidgetItem(QLocale(QLocale::English).toString(sortedItemSales[i].second, 'f', 0));
        qtyOrder->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

        summaryTable->setItem(i, 0, new QTableWidgetItem(sortedItemSales[i].first));
        summaryTable->setItem(i, 1, qtyOrder);
    }

    // Style the table
    summaryTable->setStyleSheet(
        "QHeaderView::section {"
        "   background-color: black;"  // Black background
        "   color: white;"              // White text
        "   font-weight: bold;"         // Bold font
        "   font-size: 14px;"           // Font size
        "   padding: 6px;"              // Padding inside header cells
        "   border: 1px solid #444;"    // Slight border for separation
        "}"
        );

    // Add table to layout
    mainLayout->addWidget(summaryTable);

    // Step 5: Assign Layout
    this->setLayout(mainLayout);
}

OrderSummary::~OrderSummary()
{
    delete ui;
}

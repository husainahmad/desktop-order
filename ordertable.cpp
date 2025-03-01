#include "ordertable.h"
#include "ui_ordertable.h"
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

OrderTable::OrderTable(const QJsonArray &dataArray, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderTable)
{
    ui->setupUi(this);

    // Step 1: Extract & Aggregate Sales Data
    QMap<QString, double> itemSalesMap;

    double qrTotal = 0;
    double cashTotal = 0;
    double cardTotal = 0;
    double grandTotal = 0;

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
    tableWidget->setHorizontalHeaderLabels(
        {"Order No", "Date", "Customer", "Payment", "Status", "Sub Total", "Disc", "Total", "Action"}
        );

    // Resize mode for proper spacing
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    QString formattedCount = QLocale(QLocale::English).toString(dataArray.size());

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);

    QString titleString = "Total Order : " + formattedCount + " | CASH : Rp. " + QLocale(QLocale::English).toString(cashTotal, 'f', 0) +
                          " | QRIS : Rp. " + QLocale(QLocale::English).toString(qrTotal, 'f', 0) +
                          " | CARD : Rp. " + QLocale(QLocale::English).toString(cardTotal, 'f', 0) +
                          " | Total Amount : Rp. " + QLocale(QLocale::English).toString(grandTotal, 'f', 0);

    QLabel *titleLabel = new QLabel(titleString, this);
    titleLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #333;");
    mainLayout->addWidget(titleLabel);


    // Add table to layout
    mainLayout->addWidget(tableWidget);

    // Step 5: Assign Layout
    this->setLayout(mainLayout);
}

OrderTable::~OrderTable()
{
    delete ui;
}

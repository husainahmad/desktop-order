#include "salesreportscreen.h"
#include "tokenmanager.h"
#include "screenutils.h"
#include "touchutils.h"
#include "apiclient.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QColor>
#include <QFont>
#include <QDebug>

SalesReportScreen::SalesReportScreen(QWidget *parent)
    : QWidget(parent)
    , tableWidget(new QTableWidget(this))
    , startDateEdit(new QDateEdit(this))
    , endDateEdit(new QDateEdit(this))
{
    locale = QLocale::English;

    setWindowTitle("Sales Report");
    resize(ScreenUtils::fittedSize(1000, 600, 0.96, 0.94));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Sales Report", this);
    titleLabel->setObjectName("sectionHeader");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QWidget *filterWidget = new QWidget(this);
    QHBoxLayout *filterLayout = new QHBoxLayout(filterWidget);

    QLabel *startLabel = new QLabel("Start:", this);
    startLabel->setStyleSheet(ScreenUtils::qss("font-size: 14px; font-weight: 600; color: #334155;"));

    startDateEdit->setCalendarPopup(true);
    startDateEdit->setDate(QDate::currentDate());
    startDateEdit->setDisplayFormat("yyyy-MM-dd");

    QLabel *endLabel = new QLabel("End:", this);
    endLabel->setStyleSheet(ScreenUtils::qss("font-size: 14px; font-weight: 600; color: #334155;"));

    endDateEdit->setCalendarPopup(true);
    endDateEdit->setDate(QDate::currentDate());
    endDateEdit->setDisplayFormat("yyyy-MM-dd");

    QPushButton *loadButton = new QPushButton("Load Report", this);
    loadButton->setObjectName("primaryButton");

    QPushButton *backButton = new QPushButton("Back", this);
    backButton->setObjectName("secondaryButton");

    filterLayout->addStretch();
    filterLayout->addWidget(startLabel);
    filterLayout->addWidget(startDateEdit);
    filterLayout->addWidget(endLabel);
    filterLayout->addWidget(endDateEdit);
    filterLayout->addWidget(loadButton);
    filterLayout->addStretch();
    filterLayout->addWidget(backButton);

    filterWidget->setLayout(filterLayout);
    mainLayout->addWidget(filterWidget);

    tableWidget->setColumnCount(6);
    tableWidget->setHorizontalHeaderLabels({"Category", "Product", "QTY", "Gross Sales", "Discount", "Net Sales"});
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setAlternatingRowColors(true);
    TouchUtils::enableItemViewScrolling(tableWidget);

    mainLayout->addWidget(tableWidget);

    setLayout(mainLayout);

    connect(loadButton, &QPushButton::clicked, this, &SalesReportScreen::fetchSalesReport);
    connect(backButton, &QPushButton::clicked, this, &SalesReportScreen::close);

    fetchSalesReport();
}

void SalesReportScreen::fetchSalesReport() {
    QString startDateTime = startDateEdit->date().toString("yyyy-MM-dd") + "T00:00:00";
    QString endDateTime = endDateEdit->date().toString("yyyy-MM-dd") + "T23:59:59";

    QUrl url(settingConfig.getApiEndpoint("reports", "sales"));
    QUrlQuery query;
    query.addQueryItem("start", startDateTime);
    query.addQueryItem("end", endDateTime);
    url.setQuery(query);

    ApiClient::instance().get(url, [this](const QJsonObject &response) {
        parseSalesReportResponse(QJsonDocument(response).toJson(QJsonDocument::Compact));
    }, [this](const QString &message, int) {
        qDebug() << "Sales Report API error:" << message;
        QMessageBox::warning(this, "Sales Report Error", "Failed to fetch sales report.");
    });
}

void SalesReportScreen::parseSalesReportResponse(const QByteArray &responseData) {
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject()) {
        return;
    }

    QJsonArray dataArray = doc.object()["data"].toArray();
    tableWidget->setRowCount(dataArray.size() + 1);

    int row = 0;
    int totalQuantity = 0;
    double totalGrossSales = 0.0;
    double totalDiscount = 0.0;
    double totalNetSales = 0.0;
    for (int i = 0; i < dataArray.size(); ++i) {
        QJsonObject productObj = dataArray[i].toObject();

        int quantity = productObj["quantity"].toInt();
        double grossSales = productObj["grossSales"].toDouble();
        double discount = productObj["discount"].toDouble();
        double netSales = productObj["netSales"].toDouble();
        totalQuantity += quantity;
        totalGrossSales += grossSales;
        totalDiscount += discount;
        totalNetSales += netSales;

        QTableWidgetItem *qtyItem = new QTableWidgetItem(locale.toString(quantity));
        qtyItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

        tableWidget->setItem(row, 0, new QTableWidgetItem(productObj["categoryName"].toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(productObj["productName"].toString()));
        tableWidget->setItem(row, 2, qtyItem);
        tableWidget->setItem(row, 3, new QTableWidgetItem(locale.toString(grossSales, 'f', 2)));
        tableWidget->setItem(row, 4, new QTableWidgetItem(locale.toString(discount, 'f', 2)));
        tableWidget->setItem(row, 5, new QTableWidgetItem(locale.toString(netSales, 'f', 2)));
        row++;
    }

    QFont boldFont = tableWidget->font();
    boldFont.setBold(true);

    QList<QTableWidgetItem*> footerItems;
    footerItems << new QTableWidgetItem("");
    QTableWidgetItem *totalLabel = new QTableWidgetItem("TOTAL");
    totalLabel->setFont(boldFont);
    totalLabel->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    footerItems << totalLabel;

    QTableWidgetItem *totalQtyItem = new QTableWidgetItem(locale.toString(totalQuantity));
    totalQtyItem->setFont(boldFont);
    totalQtyItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    footerItems << totalQtyItem;

    QTableWidgetItem *totalGrossItem = new QTableWidgetItem(locale.toString(totalGrossSales, 'f', 2));
    totalGrossItem->setFont(boldFont);
    totalGrossItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    footerItems << totalGrossItem;

    QTableWidgetItem *totalDiscountItem = new QTableWidgetItem(locale.toString(totalDiscount, 'f', 2));
    totalDiscountItem->setFont(boldFont);
    totalDiscountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    footerItems << totalDiscountItem;

    QTableWidgetItem *totalNetItem = new QTableWidgetItem(locale.toString(totalNetSales, 'f', 2));
    totalNetItem->setFont(boldFont);
    totalNetItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    footerItems << totalNetItem;

    int footerRow = dataArray.size();
    for (int col = 0; col < footerItems.size(); ++col) {
        QTableWidgetItem *item = footerItems[col];
        item->setBackground(QColor(240, 240, 240));
        tableWidget->setItem(footerRow, col, item);
    }
}

SalesReportScreen::~SalesReportScreen()
{
}
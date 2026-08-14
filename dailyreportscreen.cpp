#include "dailyreportscreen.h"
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
#include <QDebug>

DailyReportScreen::DailyReportScreen(QWidget *parent)
    : QWidget(parent)
    , tableWidget(new QTableWidget(this))
    , orderVolumeTableWidget(new QTableWidget(this))
    , startDateEdit(new QDateEdit(this))
    , endDateEdit(new QDateEdit(this))
{
    locale = QLocale::English;

    setWindowTitle("Daily Report");
    resize(ScreenUtils::fittedSize(900, 600, 0.96, 0.94));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Daily Report", this);
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

    filterLayout->addWidget(backButton);
    filterLayout->addStretch();
    filterLayout->addWidget(startLabel);
    filterLayout->addWidget(startDateEdit);
    filterLayout->addWidget(endLabel);
    filterLayout->addWidget(endDateEdit);
    filterLayout->addWidget(loadButton);
    filterLayout->addStretch();

    filterWidget->setLayout(filterLayout);
    mainLayout->addWidget(filterWidget);

    tableWidget->setColumnCount(5);
    tableWidget->setHorizontalHeaderLabels({"Date", "Product", "QTY", "Net Sales", "Discount"});
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setAlternatingRowColors(true);
    TouchUtils::enableItemViewScrolling(tableWidget);

    mainLayout->addWidget(tableWidget);

    QLabel *orderVolumeTitleLabel = new QLabel("Order Volume", this);
    orderVolumeTitleLabel->setObjectName("sectionHeader");
    orderVolumeTitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(orderVolumeTitleLabel);

    orderVolumeTableWidget->setColumnCount(3);
    orderVolumeTableWidget->setHorizontalHeaderLabels({"Total Orders", "Peak Time Orders", "Non Peak Time Orders"});
    orderVolumeTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    orderVolumeTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    orderVolumeTableWidget->setAlternatingRowColors(true);
    orderVolumeTableWidget->setMaximumHeight(ScreenUtils::px(120));
    TouchUtils::enableItemViewScrolling(orderVolumeTableWidget);

    mainLayout->addWidget(orderVolumeTableWidget);

    setLayout(mainLayout);

    connect(loadButton, &QPushButton::clicked, this, &DailyReportScreen::fetchDailyReport);
    connect(loadButton, &QPushButton::clicked, this, &DailyReportScreen::fetchOrderVolumeReport);
    connect(backButton, &QPushButton::clicked, this, &DailyReportScreen::close);

    fetchDailyReport();
    fetchOrderVolumeReport();
}

void DailyReportScreen::fetchDailyReport() {
    QString startDateTime = startDateEdit->date().toString("yyyy-MM-dd") + "T00:00:00";
    QString endDateTime = endDateEdit->date().toString("yyyy-MM-dd") + "T23:59:59";

    QUrl url(settingConfig.getApiEndpoint("reports", "daily"));
    QUrlQuery query;
    query.addQueryItem("start", startDateTime);
    query.addQueryItem("end", endDateTime);
    url.setQuery(query);

    ApiClient::instance().get(url, [this](const QJsonObject &response) {
        parseDailyReportResponse(QJsonDocument(response).toJson(QJsonDocument::Compact));
    }, [this](const QString &message, int) {
        qDebug() << "Daily Report API error:" << message;
        QMessageBox::warning(this, "Daily Report Error", "Failed to fetch daily report.");
    });
}

void DailyReportScreen::parseDailyReportResponse(const QByteArray &responseData) {
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject()) {
        return;
    }

    QJsonArray dataArray = doc.object()["data"].toArray();

    int rowCount = 0;
    for (int i = 0; i < dataArray.size(); ++i) {
        QJsonObject dayObj = dataArray[i].toObject();
        rowCount += dayObj["products"].toArray().size();
    }

    tableWidget->setRowCount(rowCount + 1);

    int row = 0;
    int totalQuantity = 0;
    double totalNetSales = 0.0;
    double totalDiscount = 0.0;
    for (int i = 0; i < dataArray.size(); ++i) {
        QJsonObject dayObj = dataArray[i].toObject();
        QString date = dayObj["date"].toString();
        QJsonArray productsArray = dayObj["products"].toArray();

        for (int j = 0; j < productsArray.size(); ++j) {
            QJsonObject productObj = productsArray[j].toObject();

            QTableWidgetItem *dateItem = new QTableWidgetItem(date);
            dateItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

            int quantity = productObj["quantity"].toInt();
            double netSales = productObj["netSales"].toDouble();
            double discount = productObj["discount"].toDouble();
            totalQuantity += quantity;
            totalNetSales += netSales;
            totalDiscount += discount;

            QTableWidgetItem *qtyItem = new QTableWidgetItem(locale.toString(quantity));
            qtyItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

            tableWidget->setItem(row, 0, dateItem);
            tableWidget->setItem(row, 1, new QTableWidgetItem(productObj["productName"].toString()));
            tableWidget->setItem(row, 2, qtyItem);
            tableWidget->setItem(row, 3, new QTableWidgetItem(locale.toString(netSales, 'f', 2)));
            tableWidget->setItem(row, 4, new QTableWidgetItem(locale.toString(discount, 'f', 2)));
            row++;
        }
    }

    QFont boldFont = tableWidget->font();
    boldFont.setBold(true);

    QTableWidgetItem *totalLabel = new QTableWidgetItem("TOTAL");
    totalLabel->setFont(boldFont);
    totalLabel->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QTableWidgetItem *totalQtyItem = new QTableWidgetItem(locale.toString(totalQuantity));
    totalQtyItem->setFont(boldFont);
    totalQtyItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QTableWidgetItem *totalNetSalesItem = new QTableWidgetItem(locale.toString(totalNetSales, 'f', 2));
    totalNetSalesItem->setFont(boldFont);
    totalNetSalesItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QTableWidgetItem *totalDiscountItem = new QTableWidgetItem(locale.toString(totalDiscount, 'f', 2));
    totalDiscountItem->setFont(boldFont);
    totalDiscountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QTableWidgetItem *totalSpacer = new QTableWidgetItem("");
    QList<QTableWidgetItem*> footerItems = {totalSpacer, totalLabel, totalQtyItem, totalNetSalesItem, totalDiscountItem};
    int footerRow = rowCount;
    for (int col = 0; col < footerItems.size(); ++col) {
        QTableWidgetItem *item = footerItems[col];
        item->setBackground(QColor(240, 240, 240));
        tableWidget->setItem(footerRow, col, item);
    }
}

void DailyReportScreen::fetchOrderVolumeReport() {
    QString startDateTime = startDateEdit->date().toString("yyyy-MM-dd") + "T00:00:00";
    QString endDateTime = endDateEdit->date().toString("yyyy-MM-dd") + "T23:59:59";

    QUrl url(settingConfig.getApiEndpoint("reports", "order-volume"));
    QUrlQuery query;
    query.addQueryItem("start", startDateTime);
    query.addQueryItem("end", endDateTime);
    url.setQuery(query);

    ApiClient::instance().get(url, [this](const QJsonObject &response) {
        parseOrderVolumeReportResponse(QJsonDocument(response).toJson(QJsonDocument::Compact));
    }, [this](const QString &message, int) {
        qDebug() << "Order Volume API error:" << message;
        QMessageBox::warning(this, "Order Volume Error", "Failed to fetch order volume report.");
    });
}

void DailyReportScreen::parseOrderVolumeReportResponse(const QByteArray &responseData) {
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (!doc.isObject()) {
        return;
    }

    QJsonObject dataObj = doc.object()["data"].toObject();

    orderVolumeTableWidget->setRowCount(1);
    orderVolumeTableWidget->setItem(0, 0, new QTableWidgetItem(locale.toString(dataObj["totalOrders"].toInt())));
    orderVolumeTableWidget->setItem(0, 1, new QTableWidgetItem(locale.toString(dataObj["peakTimeOrders"].toInt())));
    orderVolumeTableWidget->setItem(0, 2, new QTableWidgetItem(locale.toString(dataObj["nonPeakTimeOrders"].toInt())));

    for (int col = 0; col < orderVolumeTableWidget->columnCount(); ++col) {
        QTableWidgetItem *item = orderVolumeTableWidget->item(0, col);
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        item->setBackground(QColor(240, 240, 240));
    }
}

DailyReportScreen::~DailyReportScreen()
{
}
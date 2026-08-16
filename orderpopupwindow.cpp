#include "orderpopupwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonObject>
#include <QString>
#include <QJsonArray>
#include <QLocale>
#include <QTimeZone>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <orderprint.h>
#include <orderpaymentpopup.h>
#include "tokenmanager.h"
#include "screenutils.h"
#include "touchutils.h"
#include "apiclient.h"

OrderPopupWindow::OrderPopupWindow(const QJsonObject &order, QTabWidget *tabWidget, QWidget *parent)
    : QDialog(parent), orderDetails(order), tabWidget(tabWidget) {
    setWindowTitle("Order Details");
    setFixedSize(ScreenUtils::fittedSize(850, 700, 0.95, 0.92));

    locale = QLocale::English;

    QVBoxLayout *layout = new QVBoxLayout(this);

    // ======================= Header info =======================
    QString orderNo = order.value("orderNo").toString();
    QString customerName = order.value("customerName").toString();
    QString remark = order.value("remark").toString();

    QString createdAtStr = order.value("createdAt").isString() ? order.value("createdAt").toString() : "";
    QDateTime createdAt;
    if (!createdAtStr.isEmpty()) {
        createdAt = QDateTime::fromString(createdAtStr, Qt::ISODate);
        createdAt.setTimeZone(QTimeZone::utc());
        createdAt = createdAt.toTimeZone(QTimeZone("Asia/Jakarta"));
    } else {
        createdAt = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone("Asia/Jakarta"));
    }
    QString orderDateTime = createdAt.toString("yyyy-MM-dd HH:mm");

    QString headerText = QString("Order No: %1   |   Customer: %2   |   Date: %3")
                             .arg(orderNo, customerName, orderDateTime);

    QString orderType;
    switch (order.value("storeServiceTypesId").toInt()) {
        case 1: orderType = "DINE IN"; break;
        case 3: orderType = "TAKEWAY"; break;
        default: orderType = ""; break;
    }
    if (!orderType.isEmpty()) {
        headerText += QString("   |   Type: %1").arg(orderType);
    }

    if (!remark.isEmpty()) {
        headerText += QString("\nRemark: %1").arg(remark);
    }

    QLabel *headerLabel = new QLabel(headerText, this);
    headerLabel->setObjectName("sectionHeader");
    headerLabel->setWordWrap(true);
    layout->addWidget(headerLabel);

    // ======================= Items table =======================
    double subTotal = order.value("subTotal").toDouble();
    double discountTotal = order.value("discountTotal").toDouble();
    double grandTotal = order.value("grandTotal").toDouble();
    QJsonArray orderDetailsArray = order.value("orderDetails").toArray();

    QTableWidget *itemsTable = new QTableWidget(this);
    itemsTable->setColumnCount(5);
    itemsTable->setHorizontalHeaderLabels({"Product", "SKU", "Qty", "Price", "Amount"});
    itemsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    itemsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    itemsTable->setSelectionMode(QAbstractItemView::NoSelection);
    itemsTable->setAlternatingRowColors(true);
    TouchUtils::enableItemViewScrolling(itemsTable);

    int itemRows = 0;
    for (const QJsonValue &orderValue : orderDetailsArray) {
        QJsonObject orderObj = orderValue.toObject();
        QJsonArray orderSkus = orderObj.value("orderDetailSkus").toArray();
        itemRows += orderSkus.size();
    }

    itemsTable->setRowCount(itemRows + 3);

    int row = 0;
    for (const QJsonValue &orderValue : orderDetailsArray) {
        QJsonObject orderObj = orderValue.toObject();
        QJsonArray orderSkus = orderObj.value("orderDetailSkus").toArray();
        const QString productName = orderObj["productName"].toString();

        for (int j = 0; j < orderSkus.size(); ++j) {
            QJsonObject skuObj = orderSkus[j].toObject();

            if (j == 0) {
                QTableWidgetItem *nameItem = new QTableWidgetItem(productName);
                QFont nameFont = nameItem->font();
                nameFont.setBold(true);
                nameItem->setFont(nameFont);
                itemsTable->setItem(row, 0, nameItem);
                if (orderSkus.size() > 1) {
                    itemsTable->setSpan(row, 0, orderSkus.size(), 1);
                }
            }

            QTableWidgetItem *qtyItem = new QTableWidgetItem(locale.toString(skuObj["quantity"].toDouble(), 'f', 0));
            qtyItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            QTableWidgetItem *priceItem = new QTableWidgetItem("Rp " + locale.toString(skuObj["price"].toDouble(), 'f', 0));
            priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            QTableWidgetItem *amountItem = new QTableWidgetItem("Rp " + locale.toString(skuObj["amount"].toDouble(), 'f', 0));
            amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            itemsTable->setItem(row, 1, new QTableWidgetItem(skuObj["skuName"].toString()));
            itemsTable->setItem(row, 2, qtyItem);
            itemsTable->setItem(row, 3, priceItem);
            itemsTable->setItem(row, 4, amountItem);
            row++;
        }
    }

    QFont boldFont = itemsTable->font();
    boldFont.setBold(true);

    int summaryRow = itemRows;
    auto addSummaryRow = [&](const QString &label, const QString &value) {
        itemsTable->setSpan(summaryRow, 0, 1, 4);
        QTableWidgetItem *labelItem = new QTableWidgetItem(label);
        labelItem->setFont(boldFont);
        labelItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QTableWidgetItem *valueItem = new QTableWidgetItem(value);
        valueItem->setFont(boldFont);
        valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemsTable->setItem(summaryRow, 0, labelItem);
        itemsTable->setItem(summaryRow, 4, valueItem);
        summaryRow++;
    };

    addSummaryRow("Sub Total", "Rp " + locale.toString(subTotal, 'f', 0));
    addSummaryRow("Discount", "Rp " + locale.toString(discountTotal, 'f', 0));
    addSummaryRow("Total", "Rp " + locale.toString(grandTotal, 'f', 0));

    layout->addWidget(itemsTable, 1);

    // ======================= Buttons =======================
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    printButton = new QPushButton("Print", this);
    printButton->setObjectName("primaryButton");
    connect(printButton, &QPushButton::clicked, this, &OrderPopupWindow::printOrder);
    buttonLayout->addWidget(printButton);

    printKitchenButton = new QPushButton("Kitchen Print", this);
    printKitchenButton->setObjectName("primaryButton");
    connect(printKitchenButton, &QPushButton::clicked, this, &OrderPopupWindow::kitchenPrintOrder);
    buttonLayout->addWidget(printKitchenButton);

    QString status = order.value("status").toString().toUpper();

    if (status == "CONFIRMED" || status == "PAID") {
        if (status == "CONFIRMED") {
            payButton = new QPushButton("Pay", this);
            payButton->setObjectName("successButton");
            connect(payButton, &QPushButton::clicked, this, &OrderPopupWindow::payOrder);
            buttonLayout->addWidget(payButton);
        }

        voidButton = new QPushButton("Void", this);
        voidButton->setObjectName("dangerButton");
        connect(voidButton, &QPushButton::clicked, this, &OrderPopupWindow::voidOrder);
        buttonLayout->addWidget(voidButton);
    }

    closeButton = new QPushButton("Close", this);
    closeButton->setObjectName("secondaryButton");
    connect(closeButton, &QPushButton::clicked, this, &OrderPopupWindow::closeWindow);
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);
}

OrderPopupWindow::~OrderPopupWindow() {
}

void OrderPopupWindow::payOrder() {
    OrderPaymentPopup *popup = new OrderPaymentPopup(this->orderDetails, this->tabWidget, this);

    connect(popup, &QDialog::accepted, this, [=]() {
      this->accept();
    });

    popup->exec();
}

void OrderPopupWindow::voidOrder() {
    int orderId = orderDetails["id"].toInt();
    QUrl apiUrl(QString(configSetting.getApiEndpoint("order", "void")).arg(orderId));

    ApiClient::instance().put(apiUrl, QByteArray(), [this](const QJsonObject &) {
        this->accept();
    }, [this](const QString &message, int) {
        qDebug() << "Void order failed:" << message;
        QMessageBox::warning(this, "Error", "Failed to void the order. Please try again.");
    });
}

void OrderPopupWindow::printOrder() {
    OrderPrint printer(orderDetails);
    printer.sendToReceiptPrinter();
}

void OrderPopupWindow::kitchenPrintOrder() {
    OrderPrint printer(orderDetails);
    printer.sendToKitchenPrinter();
}

void OrderPopupWindow::closeWindow() {
    this->accept();
}
#include "orderpaymentpopup.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDoubleValidator>
#include <QRegularExpression>
#include <QUrl>
#include <QButtonGroup>
#include <QGridLayout>
#include <QApplication>
#include <QScrollArea>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QTextEdit>
#include <orderprint.h>
#include "tokenmanager.h"
#include "screenutils.h"
#include "touchutils.h"
#include "apiclient.h"

OrderPaymentPopup::OrderPaymentPopup(const QJsonObject &order, QTabWidget *tabWidget, QWidget *parent)
    : QDialog(parent), orderDetails(order), tabWidget(tabWidget),
      cashGiven(0), cashChange(0) {
    setWindowTitle("Select Payment Method");
    setFixedSize(ScreenUtils::fittedSize(760, 700, 0.95, 0.98));
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    locale = QLocale::English;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    double subTotal = order["subTotal"].toDouble();
    this->subTotal = subTotal;
    double discountTotal = order["discountTotal"].toDouble();
    double grandTotal = order["grandTotal"].toDouble();
    if (grandTotal <= 0 && discountTotal <= 0) {
        discountTotal = 0;
        grandTotal = subTotal;
    }
    totalOrder = grandTotal;

    customerName = order["customerName"].toString();
    remark = order["remark"].toString();
    serviceTypeId = order["storeServiceTypesId"].toInt();
    if (serviceTypeId <= 0) serviceTypeId = 1;

    // ======================= Order info panel =======================
    QWidget *infoPanel = new QWidget(this);
    infoPanel->setObjectName("cardPanel");
    QGridLayout *infoLayout = new QGridLayout(infoPanel);
    infoLayout->setContentsMargins(12, 10, 12, 10);
    infoLayout->setSpacing(6);

    auto infoField = [&infoPanel](const QString &labelText, const QString &valueText) -> QWidget* {
        QWidget *field = new QWidget(infoPanel);
        QVBoxLayout *fieldLayout = new QVBoxLayout(field);
        fieldLayout->setContentsMargins(0, 0, 0, 0);
        fieldLayout->setSpacing(2);

        QLabel *label = new QLabel(labelText, field);
        label->setStyleSheet(ScreenUtils::qss("color: #64748b; font-size: 11px;"));

        QLabel *value = new QLabel(valueText, field);
        value->setStyleSheet(ScreenUtils::qss("color: #0f172a; font-size: 14px; font-weight: 600;"));
        value->setTextInteractionFlags(Qt::TextSelectableByMouse);

        fieldLayout->addWidget(label);
        fieldLayout->addWidget(value);
        field->setLayout(fieldLayout);
        return field;
    };

    const QString serviceTypeName = (serviceTypeId == 3) ? "Take Away" : "Dine In";
    infoLayout->addWidget(infoField("Customer", customerName), 0, 0);
    infoLayout->addWidget(infoField("Order Type", serviceTypeName), 0, 1);
    infoLayout->addWidget(infoField("Discount", "Rp " + locale.toString(discountTotal, 'f', 0)), 0, 2);
    infoLayout->addWidget(infoField("Note", remark), 1, 0, 1, 3);

    infoPanel->setLayout(infoLayout);
    mainLayout->addWidget(infoPanel);

    QJsonArray orderDetailsArray = order.value("orderDetails").toArray();

    // ======================= Order items table =======================
    QTableWidget *itemsTable = new QTableWidget(this);
    itemsTable->setColumnCount(5);
    itemsTable->setHorizontalHeaderLabels({"Product", "SKU", "Qty", "Price", "Amount"});
    itemsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    itemsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    itemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    itemsTable->setSelectionMode(QAbstractItemView::NoSelection);
    itemsTable->setAlternatingRowColors(true);
    itemsTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    itemsTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    TouchUtils::enableItemViewScrolling(itemsTable);

    int itemRows = 0;
    for (const QJsonValue &orderValue : orderDetailsArray) {
        QJsonObject orderObj = orderValue.toObject();
        QJsonArray orderSkus = orderObj.value("orderDetailSkus").toArray();
        itemRows += orderSkus.size();
    }

    itemsTable->setRowCount(itemRows + 3); // +3 for subtotal/discount/total rows

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

            QTableWidgetItem *skuItem = new QTableWidgetItem(skuObj["skuName"].toString());
            QTableWidgetItem *qtyItem = new QTableWidgetItem(locale.toString(skuObj["quantity"].toDouble(), 'f', 0));
            qtyItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            QTableWidgetItem *priceItem = new QTableWidgetItem("Rp " + locale.toString(skuObj["price"].toDouble(), 'f', 0));
            priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            QTableWidgetItem *amountItem = new QTableWidgetItem("Rp " + locale.toString(skuObj["amount"].toDouble(), 'f', 0));
            amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            itemsTable->setItem(row, 1, skuItem);
            itemsTable->setItem(row, 2, qtyItem);
            itemsTable->setItem(row, 3, priceItem);
            itemsTable->setItem(row, 4, amountItem);
            row++;
        }
    }

    QFont boldFont = itemsTable->font();
    boldFont.setBold(true);

    int summaryRow = itemRows;
    auto addSummaryRow = [&](const QString &label, const QString &value, const QFont &font) -> QTableWidgetItem* {
        itemsTable->setSpan(summaryRow, 0, 1, 4);
        QTableWidgetItem *labelItem = new QTableWidgetItem(label);
        labelItem->setFont(font);
        labelItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QTableWidgetItem *valueItem = new QTableWidgetItem(value);
        valueItem->setFont(font);
        valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemsTable->setItem(summaryRow, 0, labelItem);
        itemsTable->setItem(summaryRow, 4, valueItem);
        summaryRow++;
        return valueItem;
    };

    addSummaryRow("Sub Total", "Rp " + locale.toString(subTotal, 'f', 0), boldFont);
    addSummaryRow("Discount", "Rp " + locale.toString(discountTotal, 'f', 0), boldFont);
    totalSummaryItem = addSummaryRow("Total", "Rp " + locale.toString(grandTotal, 'f', 0), boldFont);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(itemsTable);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFixedHeight(qMin(ScreenUtils::px(300), ScreenUtils::availableHeight() / 3));
    mainLayout->addWidget(scrollArea);

    // ======================= Payment method selection =======================
    paymentGroup = new QButtonGroup(this);

    QHBoxLayout *paymentLayout = new QHBoxLayout();
    paymentLayout->setSpacing(10);

    qrPaymentBtn = new QPushButton("Scan QR Code", this);
    cardPaymentBtn = new QPushButton("Pay by Card", this);
    cashPaymentBtn = new QPushButton("Pay by Cash", this);

    for (QPushButton *btn : {qrPaymentBtn, cardPaymentBtn, cashPaymentBtn}) {
        btn->setObjectName("paymentOptionButton");
        btn->setCheckable(true);
        paymentGroup->addButton(btn);
    }

    paymentGroup->setId(qrPaymentBtn, 2);
    paymentGroup->setId(cardPaymentBtn, 3);
    paymentGroup->setId(cashPaymentBtn, 1);
    paymentLayout->addWidget(qrPaymentBtn);
    paymentLayout->addWidget(cardPaymentBtn);
    paymentLayout->addWidget(cashPaymentBtn);
    mainLayout->addLayout(paymentLayout);

    // ======================= Cash payment panel =======================
    cashPanel = new QWidget(this);
    cashPanel->setVisible(false);
    QVBoxLayout *cashLayout = new QVBoxLayout();
    cashLayout->setContentsMargins(0, 10, 0, 0);
    cashLayout->setSpacing(10);

    QHBoxLayout *cashTopLayout = new QHBoxLayout();

    QVBoxLayout *totalColumn = new QVBoxLayout();
    totalColumn->setSpacing(5);
    QLabel *totalLabel = new QLabel("TOTAL", cashPanel);
    totalLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #666666;"));
    totalColumn->addWidget(totalLabel);

    cashTotalText = new QLabel("Rp " + locale.toString(totalOrder, 'f', 0), cashPanel);
    cashTotalText->setStyleSheet(ScreenUtils::qss("font-size: 20px; font-weight: bold; color: #333333;"));
    totalColumn->addWidget(cashTotalText);

    QGridLayout *cashGrid = new QGridLayout();
    cashGrid->setSpacing(5);
    QStringList denominations = {"1.000", "2.000", "5.000", "10.000", "20.000", "50.000", "100.000", "200.000"};
    QList<int> denomValues = {1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000};
    for (int i = 0; i < denominations.size(); ++i) {
        QPushButton *btn = new QPushButton(denominations[i], cashPanel);
        btn->setProperty("denomValue", denomValues[i]);
        btn->setObjectName("denomButton");
        connect(btn, &QPushButton::clicked, this, &OrderPaymentPopup::onCashButtonClicked);
        int row = i / 4;
        int col = i % 4;
        cashGrid->addWidget(btn, row, col);
    }
    totalColumn->addLayout(cashGrid);

    cashTopLayout->addLayout(totalColumn);

    QVBoxLayout *receivedColumn = new QVBoxLayout();
    receivedColumn->setSpacing(5);
    QLabel *receivedLabel = new QLabel("UANG DITERIMA", cashPanel);
    receivedLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #666666;"));
    receivedColumn->addWidget(receivedLabel);

    cashReceivedText = new QLabel("Rp 0", cashPanel);
    cashReceivedText->setStyleSheet(ScreenUtils::qss("font-size: 20px; font-weight: bold; color: #333333;"));
    receivedColumn->addWidget(cashReceivedText);

    QLabel *changeLabel = new QLabel("KEMBALIAN", cashPanel);
    changeLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #666666; margin-top: 12px;"));
    receivedColumn->addWidget(changeLabel);

    cashChangeText = new QLabel("Rp 0", cashPanel);
    cashChangeText->setStyleSheet(ScreenUtils::qss("font-size: 20px; font-weight: bold; color: #4CAF50;"));
    receivedColumn->addWidget(cashChangeText);

    QHBoxLayout *cashControlLayout = new QHBoxLayout();
    cashControlLayout->setSpacing(8);
    cashControlLayout->setAlignment(Qt::AlignRight);

    clearButton = new QPushButton("CLEAR", cashPanel);
    clearButton->setObjectName("ghostButton");
    connect(clearButton, &QPushButton::clicked, this, &OrderPaymentPopup::clearCashAmount);
    cashControlLayout->addWidget(clearButton);

    exactButton = new QPushButton("EXACT", cashPanel);
    exactButton->setObjectName("successButton");
    connect(exactButton, &QPushButton::clicked, this, &OrderPaymentPopup::setExactCash);
    cashControlLayout->addWidget(exactButton);

    receivedColumn->addLayout(cashControlLayout);

    cashTopLayout->addLayout(receivedColumn);
    cashLayout->addLayout(cashTopLayout);

    cashPanel->setLayout(cashLayout);
    mainLayout->addWidget(cashPanel);

    connect(paymentGroup, QOverload<int, bool>::of(&QButtonGroup::idToggled), this, &OrderPaymentPopup::selectPaymentMethod);
    cashPaymentBtn->setChecked(true);

    // ======================= Pay / Close =======================
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    payButton = new QPushButton("Pay", this);
    payButton->setObjectName("primaryButton");
    buttonLayout->addWidget(payButton);

    QPushButton *closeButton = new QPushButton("Close", this);
    closeButton->setObjectName("secondaryButton");
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    connect(closeButton, &QPushButton::clicked, this, &OrderPaymentPopup::reject);

    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    connect(payButton, &QPushButton::clicked, this, &OrderPaymentPopup::processPayment);
}

OrderPaymentPopup::~OrderPaymentPopup() {}

void OrderPaymentPopup::selectPaymentMethod(int id) {
    cashPanel->setVisible(id == 1);
}

void OrderPaymentPopup::onCashButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int denomValue = btn->property("denomValue").toInt();
    cashGiven += denomValue;
    updateCashDisplay();
}

void OrderPaymentPopup::clearCashAmount() {
    cashGiven = 0;
    updateCashDisplay();
}

void OrderPaymentPopup::setExactCash() {
    cashGiven = totalOrder;
    updateCashDisplay();
}

void OrderPaymentPopup::updateCashDisplay() {
    cashReceivedText->setText("Rp " + locale.toString(cashGiven, 'f', 0));
    cashChange = cashGiven - totalOrder;
    if (cashChange < 0) cashChange = 0;
    cashChangeText->setText("Rp " + locale.toString(cashChange, 'f', 0));
}

void OrderPaymentPopup::processPayment() {

    int paymentId = 1;

    if (paymentGroup->id(paymentGroup->checkedButton()) == 2) {
        paymentId = 2;  // QR Payment
    } else if (paymentGroup->id(paymentGroup->checkedButton()) == 3) {
        paymentId = 3;  // Card Payment
    } else if (paymentGroup->id(paymentGroup->checkedButton()) == 1) {
        if (cashGiven < totalOrder) {
            QMessageBox::warning(this, "Error", "Insufficient cash entered!");
            return;
        }
    } else {
        QMessageBox::warning(this, "Error", "Please select a payment method!");
        return;
    }

    QJsonObject confirmData;
    confirmData["storeServiceTypesId"] = serviceTypeId;
    confirmData["customerName"] = customerName;
    confirmData["customerId"] = -1;
    confirmData["remark"] = remark;
    confirmData["orderDetails"] = orderDetails["orderDetails"].toArray();
    confirmData["discount"] = locale.toString(orderDetails["discountTotal"].toDouble(), 'f', 0);

    const QUrl confirmUrl(configSetting.getApiEndpoint("order", "confirm"));
    if (!confirmUrl.isValid()) {
        QMessageBox::warning(this, "Error", "Invalid API URL. Please check config.");
        return;
    }

    ApiClient::instance().post(confirmUrl, QJsonDocument(confirmData).toJson(),
        [this, paymentId](const QJsonObject &response) {
            QJsonObject data = response["data"].toObject();
            const int orderId = data["id"].toInt();
            if (orderId <= 0) {
                QMessageBox::warning(this, "Error", "Order placement failed. No order id returned.");
                return;
            }

            QJsonObject payData;
            payData["orderId"] = orderId;
            payData["paymentId"] = paymentId;

            QUrl payUrl(configSetting.getApiEndpoint("order", "payment"));
            if (!payUrl.isValid()) {
                QMessageBox::warning(this, "Error", "Invalid API URL. Please check config.");
                return;
            }

            ApiClient::instance().put(payUrl, QJsonDocument(payData).toJson(),
                [this](const QJsonObject &payResponse) {
                    QJsonObject payObj = payResponse["data"].toObject();

                    OrderPrint orderPrinter(payObj);
                    orderPrinter.sendToReceiptPrinter();

                    if (this->tabWidget) {
                        int index = this->tabWidget->currentIndex();
                        if (index > 0) {
                            QWidget *page = this->tabWidget->widget(index);
                            this->tabWidget->removeTab(index);
                            if (page) {
                                page->deleteLater();
                            }
                        }
                    }

                    this->accept();
                }, [this](const QString &message, int) {
                    qDebug() << "Order payment failed:" << message;
                    QMessageBox::warning(this, "Error", "Failed to process payment. Please try again.");
                });
        }, [this](const QString &message, int) {
            qDebug() << "Order placement failed:" << message;
            QMessageBox::warning(this, "Error", "Failed to place order. Please try again.");
        });
}

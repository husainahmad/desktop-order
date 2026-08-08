#include "orderpaymentpopup.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QTextBrowser>
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
#include <orderprint.h>
#include "tokenmanager.h"
#include "screenutils.h"
#include "touchutils.h"

OrderPaymentPopup::OrderPaymentPopup(const QJsonObject &order, QTabWidget *tabWidget, QWidget *parent)
    : QDialog(parent), networkManager(new QNetworkAccessManager(this)), orderDetails(order), tabWidget(tabWidget),
      cashGiven(0), cashChange(0) {
    setWindowTitle("Select Payment Method");
    setFixedSize(ScreenUtils::fittedSize(750, 700, 0.95, 0.92));
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    locale = QLocale::English;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    double subTotal = order["subTotal"].toDouble();
    double discountTotal = order["discountTotal"].toDouble();
    double grandTotal = order["grandTotal"].toDouble();
    totalOrder = grandTotal;

    QJsonArray orderDetailsArray = order.value("orderDetails").toArray();
    QString table = R"(
            <style>
                .scroll-container {
                    width: 100%;
                    overflow-x: auto;
                }
                table {
                    width: 100%;
                    min-width: 580px;
                    border-collapse: collapse;
                    border: 1px solid #ddd;
                }
                th, td {
                    border: 1px solid #ddd;
                    padding: 8px;
                    text-align: left;
                    font-size: 13px;
                    white-space: nowrap;
                }
                th {
                    background-color: #4CAF50;
                    color: white;
                }
                tr:nth-child(even) {
                    background-color: #f9f9f9;
                }

                /* 👇 Specific column widths and alignments */
                th:nth-child(3), td:nth-child(3) {
                    width: 50px;
                    text-align: center;
                }

                th:nth-child(4), td:nth-child(4),
                th:nth-child(5), td:nth-child(5) {
                    width: 90px;
                    text-align: right;
                }
            </style>

        <div class='scroll-container'>
    )";

    // Header section

    table += "<table width=100%>"
             "<tr>"
             "<th>Product Name</th>"
             "<th>SKU</th>"
             "<th>Quantity</th>"
             "<th>Price</th>"
             "<th>Amount</th>"
             "</tr>";

    for (int i = 0; i < orderDetailsArray.size(); ++i) {
        QJsonObject orderObj = orderDetailsArray[i].toObject();
        QJsonArray orderSKus = orderObj.value("orderDetailSkus").toArray();

        for (int j = 0; j < orderSKus.size(); j++) {
            QJsonObject skuObj = orderSKus[j].toObject();
            table += "<tr>";

            // Product Name (only for the first SKU)
            if (j == 0) {
                table += "<td rowspan='" + QString::number(orderSKus.size()) + "'>"
                                                                               "<strong>" + orderObj["productName"].toString() + "</strong>"
                                                                "</td>";
            }

            // SKU, Quantity, Price, Amount
            table += "<td>" + skuObj["skuName"].toString() + "</td>";
            table += "<td style='text-align: center;'>" + locale.toString(skuObj["quantity"].toDouble(), 'f', 0) + "</td>";
            table += "<td style='text-align: right;'>Rp " + locale.toString(skuObj["price"].toDouble(), 'f', 0) + "</td>";
            table += "<td style='text-align: right;'>Rp " + locale.toString(skuObj["amount"].toDouble(), 'f', 0) + "</td>";
            table += "</tr>";
        }
    }

    // Summary Row
    table += "<tr><td colspan=4 style='text-align: right; font-weight: bold;'>Sub Total</td>"
             "<td style='text-align: right;'>Rp " + locale.toString(subTotal, 'f', 0) + "</td></tr>";

    table += "<tr><td colspan=4 style='text-align: right; font-weight: bold;'>Discount</td>"
             "<td style='text-align: right; color: red;'>Rp " + locale.toString(discountTotal, 'f', 0) + "</td></tr>";

    table += "<tr><td colspan=4 style='text-align: right; font-size: 16px; font-weight: bold;'>Total</td>"
             "<td style='text-align: right; font-size: 16px; font-weight: bold; color: #4CAF50;'>Rp "
             + locale.toString(grandTotal, 'f', 0) + "</td></tr>";

    table += "</table>";

    totalHtmlWidget = new QTextBrowser(this);
    totalHtmlWidget->setHtml(table);
    totalHtmlWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    totalHtmlWidget->setMinimumWidth(520);  // Set a minimum width
    totalHtmlWidget->setMaximumWidth(16777215);  // Ensure no limit
    totalHtmlWidget->setFixedHeight(qMin(300, ScreenUtils::availableHeight() / 3));
    totalHtmlWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    totalHtmlWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    TouchUtils::enableTouchScrolling(totalHtmlWidget);

    // Wrap in scroll area for better scrolling
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(totalHtmlWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFixedHeight(qMin(300, ScreenUtils::availableHeight() / 3));
    mainLayout->addWidget(scrollArea);

    // Payment method selection using buttons
    paymentGroup = new QButtonGroup(this);

    QHBoxLayout *paymentLayout = new QHBoxLayout();
    paymentLayout->setSpacing(10);

    qrPaymentBtn = new QPushButton("Scan QR Code", this);
    cardPaymentBtn = new QPushButton("Pay by Card", this);
    cashPaymentBtn = new QPushButton("Pay by Cash", this);

    QString paymentBtnStyle = R"(
        QPushButton {
            font-size: 14px;
            padding: 12px 20px;
            border-radius: 6px;
            border: 1px solid #cccccc;
            background-color: #f0f0f0;
        }
        QPushButton:checked {
            background-color: #007bff;
            color: white;
            border: 1px solid #007bff;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
        }
    )";

    qrPaymentBtn->setCheckable(true);
    cardPaymentBtn->setCheckable(true);
    cashPaymentBtn->setCheckable(true);
    qrPaymentBtn->setStyleSheet(paymentBtnStyle);
    cardPaymentBtn->setStyleSheet(paymentBtnStyle);
    cashPaymentBtn->setStyleSheet(paymentBtnStyle);

    paymentGroup->addButton(qrPaymentBtn, 2);
    paymentGroup->addButton(cardPaymentBtn, 3);
    paymentGroup->addButton(cashPaymentBtn, 1);
    paymentLayout->addWidget(qrPaymentBtn);
    paymentLayout->addWidget(cardPaymentBtn);
    paymentLayout->addWidget(cashPaymentBtn);
    mainLayout->addLayout(paymentLayout);

    // Cash payment panel
    cashPanel = new QWidget(this);
    cashPanel->setVisible(false);
    QVBoxLayout *cashLayout = new QVBoxLayout();
    cashLayout->setContentsMargins(0, 10, 0, 0);
    cashLayout->setSpacing(10);

    // Total and denomination buttons
    QHBoxLayout *cashTopLayout = new QHBoxLayout();

    QVBoxLayout *totalColumn = new QVBoxLayout();
    totalColumn->setSpacing(5);
    QLabel *totalLabel = new QLabel("TOTAL", cashPanel);
    totalLabel->setStyleSheet("font-size: 12px; color: #666666;");
    totalColumn->addWidget(totalLabel);

    cashTotalText = new QLabel("Rp " + locale.toString(totalOrder, 'f', 0), cashPanel);
    cashTotalText->setStyleSheet("font-size: 20px; font-weight: bold; color: #333333;");
    totalColumn->addWidget(cashTotalText);

    // Cash denomination buttons grid
    cashGrid = new QGridLayout();
    cashGrid->setSpacing(5);
    QStringList denominations = {"1.000", "2.000", "5.000", "10.000", "20.000", "50.000", "100.000", "200.000", "500.000"};
    QList<int> denomValues = {1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000};
    for (int i = 0; i < denominations.size(); ++i) {
        QPushButton *btn = new QPushButton(denominations[i], cashPanel);
        btn->setProperty("denomValue", denomValues[i]);
        btn->setStyleSheet(
            "QPushButton {"
            "font-size: 13px;"
            "padding: 10px;"
            "background-color: #f0f0f0;"
            "border: 1px solid #cccccc;"
            "border-radius: 6px;"
            "}"
            "QPushButton:hover {"
            "background-color: #e0e0e0;"
            "}"
            "QPushButton:pressed {"
            "background-color: #d0d0d0;"
            "}"
        );
        connect(btn, &QPushButton::clicked, this, &OrderPaymentPopup::onCashButtonClicked);
        int row = i / 3;
        int col = i % 3;
        cashGrid->addWidget(btn, row, col);
    }
    totalColumn->addLayout(cashGrid);

    // CLEAR and EXACT buttons
    QHBoxLayout *cashControlLayout = new QHBoxLayout();
    cashControlLayout->setSpacing(8);

    clearButton = new QPushButton("CLEAR", cashPanel);
    clearButton->setStyleSheet(
        "QPushButton {"
        "font-size: 13px;"
        "padding: 10px;"
        "background-color: transparent;"
        "border: 1px solid #ff9800;"
        "border-radius: 6px;"
        "color: #ff9800;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(255, 152, 0, 0.1);"
        "}"
    );
    connect(clearButton, &QPushButton::clicked, this, &OrderPaymentPopup::clearCashAmount);
    cashControlLayout->addWidget(clearButton);

    exactButton = new QPushButton("EXACT", cashPanel);
    exactButton->setStyleSheet(
        "QPushButton {"
        "font-size: 13px;"
        "padding: 10px;"
        "background-color: #4CAF50;"
        "border: none;"
        "border-radius: 6px;"
        "color: white;"
        "}"
    );
    connect(exactButton, &QPushButton::clicked, this, &OrderPaymentPopup::setExactCash);
    cashControlLayout->addWidget(exactButton);

    totalColumn->addLayout(cashControlLayout);
    cashTopLayout->addLayout(totalColumn);

    // Received and Change columns
    QVBoxLayout *receivedColumn = new QVBoxLayout();
    receivedColumn->setSpacing(5);
    QLabel *receivedLabel = new QLabel("UANG DITERIMA", cashPanel);
    receivedLabel->setStyleSheet("font-size: 12px; color: #666666;");
    receivedColumn->addWidget(receivedLabel);

    cashReceivedText = new QLabel("Rp 0", cashPanel);
    cashReceivedText->setStyleSheet("font-size: 20px; font-weight: bold; color: #333333;");
    receivedColumn->addWidget(cashReceivedText);

    QLabel *changeLabel = new QLabel("KEMBALIAN", cashPanel);
    changeLabel->setStyleSheet("font-size: 12px; color: #666666; margin-top: 12px;");
    receivedColumn->addWidget(changeLabel);

    cashChangeText = new QLabel("Rp 0", cashPanel);
    cashChangeText->setStyleSheet("font-size: 20px; font-weight: bold; color: #4CAF50;");
    receivedColumn->addWidget(cashChangeText);

    bayarButton = new QPushButton("BAYAR", cashPanel);
    bayarButton->setStyleSheet(
        "QPushButton {"
        "font-size: 15px;"
        "padding: 12px;"
        "background-color: #4CAF50;"
        "border: none;"
        "border-radius: 6px;"
        "color: white;"
        "margin-top: 16px;"
        "}"
    );
    connect(bayarButton, &QPushButton::clicked, this, &OrderPaymentPopup::payCash);
    receivedColumn->addWidget(bayarButton);

    cashTopLayout->addLayout(receivedColumn);
    cashLayout->addLayout(cashTopLayout);

    cashPanel->setLayout(cashLayout);
    mainLayout->addWidget(cashPanel);

    // Connect payment group selection
    connect(paymentGroup, QOverload<int, bool>::of(&QButtonGroup::idToggled), this, &OrderPaymentPopup::selectPaymentMethod);
    cashPaymentBtn->setChecked(true);

    // Common style for bigger buttons
    QString buttonStyle = R"(
        QPushButton {
            font-size: 16px;
            padding: 12px 20px;
            min-height: 20px;
            min-width: 120px;
            background-color: #007bff;
            color: white;
            border-radius: 6px;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QPushButton:pressed {
            background-color: #004494;
        }
    )";

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    payButton = new QPushButton("Pay", this);
    payButton->setStyleSheet(buttonStyle);
    buttonLayout->addWidget(payButton);

    QPushButton *closeButton = new QPushButton("Close", this);
    closeButton->setStyleSheet(buttonStyle);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    connect(closeButton, &QPushButton::clicked, this, &OrderPaymentPopup::reject);

    // Add vertical stretch to push buttons to bottom
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    // Connect button signals
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

void OrderPaymentPopup::payCash() {
    if (cashGiven >= totalOrder) {
        processPayment();
    } else {
        QMessageBox::warning(this, "Error", "Insufficient cash entered!");
    }
}

void OrderPaymentPopup::updateCashDisplay() {
    cashReceivedText->setText("Rp " + locale.toString(cashGiven, 'f', 0));
    cashChange = cashGiven - totalOrder;
    if (cashChange < 0) cashChange = 0;
    cashChangeText->setText("Rp " + locale.toString(cashChange, 'f', 0));
}

void OrderPaymentPopup::processPayment() {

    // Default Payment ID (Cash)
    int paymentId = 1;

    if (paymentGroup->id(paymentGroup->checkedButton()) == 2) {
        paymentId = 2;  // QR Payment
    } else if (paymentGroup->id(paymentGroup->checkedButton()) == 3) {
        paymentId = 3;  // Card Payment
    } else if (paymentGroup->id(paymentGroup->checkedButton()) == 1) {
        double grandTotal = orderDetails["grandTotal"].toDouble();

        // Validate the cash entered
        if (cashGiven < grandTotal) {
            QMessageBox::warning(this, "Error", "Insufficient cash entered!");
            return;
        }
    } else {
        QMessageBox::warning(this, "Error", "Please select a payment method!");
        return;
    }

    // Prepare JSON order data for payment
    QJsonObject orderData;
    orderData["orderId"] = orderDetails["id"].toInt();
    orderData["paymentId"] = paymentId;

    QJsonDocument jsonDoc(orderData);
    QByteArray jsonData = jsonDoc.toJson();

    // API URL for order processing
    QString orderUrl = configSetting.getApiEndpoint("order", "payment");
    QUrl url(orderUrl);

    if (!url.isValid()) {
        qDebug() << "Invalid URL from config.ini:" << orderUrl;
        QMessageBox::warning(this, "Error", "Invalid API URL. Please check config.ini.");
        return;
    }

    // Load authentication token
    QString authToken = TokenManager::instance().getAccessToken();
    if (authToken.isEmpty()) {
        qDebug() << "Auth token is empty!";
        QMessageBox::warning(this, "Error", "Authentication token is missing. Please log in.");
        return;
    }

    // Prepare the API request
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + authToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager->put(request, jsonData);

    // Handle API response
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();

            if (responseData.isEmpty()) {
                qDebug() << "Empty response received!";
                QMessageBox::warning(this, "Error", "No response from the server.");
                reply->deleteLater();
                return;
            }

            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonDoc.object();

            OrderPrint orderPrinter(jsonObj["data"].toObject());
            orderPrinter.sendToReceiptPrinter();

            qDebug() << "Order processed successfully!";

            // Close the tab after successful payment
            if (this->tabWidget || this->tabWidget != NULL) {

                int index = tabWidget->currentIndex();
                if (index>0) {
                    this->tabWidget->removeTab(this->tabWidget->currentIndex());
                }
            }

            // Close the popup after successful payment
            this->accept();
        } else {
            qDebug() << "Order placement failed: " << reply->errorString();
            QMessageBox::warning(this, "Error", "Failed to process payment. Please try again.");
        }

        reply->deleteLater();
    });
}




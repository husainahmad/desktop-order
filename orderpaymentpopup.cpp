#include "orderpaymentpopup.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QLineEdit>
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
#include <orderprint.h>

OrderPaymentPopup::OrderPaymentPopup(const QJsonObject &order, QTabWidget *tabWidget, QWidget *parent)
    : QDialog(parent), networkManager(new QNetworkAccessManager(this)), orderDetails(order), tabWidget(tabWidget) {
    setWindowTitle("Select Payment Method");
    setFixedSize(750, 650);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    locale = QLocale::English;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    double subTotal = order["subTotal"].toDouble();
    double discountTotal = order["discountTotal"].toDouble();
    double grandTotal = order["grandTotal"].toDouble();
    totalOrder = grandTotal;

    QJsonArray orderDetails = order.value("orderDetails").toArray();
    QString table = R"(
        <style>
            .scroll-container {
                width: 100%;
                overflow-x: auto;
            }
            table {
                width: 100%;
                min-width: 700px; /* ensures wide tables */
                border-collapse: collapse;
                border: 1px solid #ddd;
            }
            th, td {
                border: 1px solid #ddd;
                padding: 12px;
                text-align: left;
                font-size: 14px;
                white-space: nowrap; /* prevent text from wrapping */
            }
            th {
                background-color: #4CAF50;
                color: white;
            }
            tr:nth-child(even) {
                background-color: #f9f9f9;
            }
            td:last-child, td:nth-child(3), td:nth-child(4) {
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

    for (int i = 0; i < orderDetails.size(); ++i) {
        QJsonObject orderObj = orderDetails[i].toObject();
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
    totalHtmlWidget->setMinimumWidth(600);  // Set a minimum width
    totalHtmlWidget->setMaximumWidth(16777215);  // Ensure no limit
    totalHtmlWidget->setFixedHeight(400);
    mainLayout->addWidget(totalHtmlWidget);

    // Payment method selection
    qrPayment = new QRadioButton("Scan QR Code", this);
    cardPayment = new QRadioButton("Pay by Card", this);
    cashPayment = new QRadioButton("Pay by Cash", this);
    mainLayout->addWidget(qrPayment);
    mainLayout->addWidget(cardPayment);
    mainLayout->addWidget(cashPayment);

    // 📌 Cash input layout
    QHBoxLayout *cashLayout = new QHBoxLayout();

    cashAmountInput = new QLineEdit(this);
    cashAmountInput->setPlaceholderText("Enter cash amount...");

    QFont font;
    font.setPointSize(32);
    cashAmountInput->setFont(font);
    cashAmountInput->setStyleSheet("padding: 10px; font-size: 16px;");

    QDoubleValidator *validator = new QDoubleValidator(0, 999999999, 2, this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    cashAmountInput->setValidator(validator);

    // 📌 Non-editable text for formatting
    cashAmountLabel = new QLabel("Rp 0", this);
    cashAmountLabel->setStyleSheet(
        "border: 1px solid #ccc; "
        "background: #f9f9f9; "
        "padding: 10px; "
        "text-align: right; "
        "color: #333333; "  // Darker color
        "font-weight: bold;"  // Make the font bold
        );

    cashLayout->addWidget(cashAmountInput);
    cashLayout->addWidget(cashAmountLabel);

    cashLayout->setSpacing(10);
    cashLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addLayout(cashLayout);
    cashLayout->setAlignment(Qt::AlignLeft);

    // Connect radio button selection
    connect(qrPayment, &QRadioButton::toggled, this, &OrderPaymentPopup::updateUI);
    connect(cardPayment, &QRadioButton::toggled, this, &OrderPaymentPopup::updateUI);
    connect(cashPayment, &QRadioButton::toggled, this, &OrderPaymentPopup::updateUI);

    // 📌 Update label when user enters cash amount
    connect(cashAmountInput, &QLineEdit::textChanged, this, &OrderPaymentPopup::updateCashLabel);

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
    payButton = new QPushButton("Pay", this);
    payButton->setStyleSheet(buttonStyle);
    buttonLayout->addWidget(payButton);

    mainLayout->addLayout(buttonLayout);
    // Connect button signals
    connect(payButton, &QPushButton::clicked, this, &OrderPaymentPopup::processPayment);
}

OrderPaymentPopup::~OrderPaymentPopup() {}

void OrderPaymentPopup::updateUI() {
    // Show cash input only if cash payment is selected
    cashAmountInput->setEnabled(cashPayment->isChecked());
    cashAmountLabel->setEnabled(cashPayment->isChecked());
}

void OrderPaymentPopup::updateCashLabel(const QString &text) {
    // Remove commas for processing, but leave them in the input
    QString numericText = text;
    numericText.remove(QRegularExpression("[^0-9.]"));  // Remove non-numeric characters except the dot

    bool ok;
    double amount = numericText.toDouble(&ok);
    if (ok) {

        // Update the label to show the remaining amount (Total - Amount Entered)
        double remainingAmount = totalOrder - amount;
        cashAmountLabel->setText(QString("Rp %1").arg(locale.toString(remainingAmount, 'f', 0)));  // Format the label with the correct currency
    } else {
        // If invalid amount, reset the label to show zero
        cashAmountLabel->setText("Rp 0");
    }
}

void OrderPaymentPopup::processPayment() {

    static QRegularExpression nonNumeric("[^0-9.]");

    // Default Payment ID (Cash)
    int paymentId = 1;

    if (qrPayment->isChecked()) {
        paymentId = 2;  // QR Payment
    } else if (cardPayment->isChecked()) {
        paymentId = 3;  // Card Payment
    } else if (cashPayment->isChecked()) {
        bool valid;
        cashAmountInput->setFocus();
        QString numericText = cashAmountInput->text().remove(nonNumeric);

        // Convert the cleaned-up string into a valid numeric value
        double cashGiven = numericText.toDouble(&valid);
        double grandTotal = orderDetails["grandTotal"].toDouble();

        // Validate the cash entered
        if (!valid || cashGiven < grandTotal) {
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
    QString authToken = configSetting.getValue("authToken").toString().trimmed();
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
            if (this->tabWidget) {
                this->tabWidget->removeTab(this->tabWidget->currentIndex());
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




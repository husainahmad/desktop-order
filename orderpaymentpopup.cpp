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

OrderPaymentPopup::OrderPaymentPopup(const QJsonObject &order, QTabWidget *tabWidget, QWidget *parent)
    : QDialog(parent), networkManager(new QNetworkAccessManager(this)), orderDetails(order), tabWidget(tabWidget) {
    setWindowTitle("Select Payment Method");
    setFixedSize(650, 550);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    locale = QLocale::English;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    double subTotal = order["subTotal"].toDouble();
    double discountTotal = order["discountTotal"].toDouble();
    double grandTotal = order["grandTotal"].toDouble();
    totalOrder = grandTotal;

    QJsonArray orderDetails = order.value("orderDetails").toArray();

    QString table = "<style>"
                    "body { margin: 0; padding: 0; font-family: Arial, sans-serif; }"
                    ".container { width: 100%; padding: 10px; box-sizing: border-box; }"
                    "table { width: 100%; border-collapse: collapse; }"
                    "th, td { border: 1px solid #ddd; padding: 10px; text-align: left; }"
                    "th { background-color: #4CAF50; color: white; font-size: 14px; }"
                    "tr:nth-child(even) { background-color: #f2f2f2; }"
                    "td:last-child { font-weight: bold; text-align: right; }"
                    "</style>";

    // Header section

    table += "<table>"
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
    font.setPointSize(16);
    cashAmountInput->setFont(font);
    cashAmountInput->setStyleSheet("padding: 10px; font-size: 16px;");

    QDoubleValidator *validator = new QDoubleValidator(0, 999999999, 2, this);
    validator->setNotation(QDoubleValidator::StandardNotation);
    cashAmountInput->setValidator(validator);

    // 📌 Non-editable text for formatting
    cashAmountLabel = new QLabel("Rp 0", this);
    cashAmountLabel->setFont(font);
    cashAmountLabel->setStyleSheet("border: 1px solid #ccc; background: #f9f9f9; padding: 10px; text-align: right;");

    cashLayout->addWidget(cashAmountInput);
    cashLayout->addWidget(cashAmountLabel);

    cashLayout->setSpacing(10);
    cashLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addLayout(cashLayout);
    cashLayout->setAlignment(Qt::AlignLeft);
    cashAmountInput->setVisible(false);
    cashAmountLabel->setVisible(false);

    // Connect radio button selection
    connect(qrPayment, &QRadioButton::toggled, this, &OrderPaymentPopup::updateUI);
    connect(cardPayment, &QRadioButton::toggled, this, &OrderPaymentPopup::updateUI);
    connect(cashPayment, &QRadioButton::toggled, this, &OrderPaymentPopup::updateUI);

    // 📌 Update label when user enters cash amount
    connect(cashAmountInput, &QLineEdit::textChanged, this, &OrderPaymentPopup::updateCashLabel);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    payButton = new QPushButton("Pay", this);
    buttonLayout->addWidget(payButton);
    mainLayout->addLayout(buttonLayout);

    // Connect button signals
    connect(payButton, &QPushButton::clicked, this, &OrderPaymentPopup::processPayment);
}

OrderPaymentPopup::~OrderPaymentPopup() {}

void OrderPaymentPopup::updateUI() {
    // Show cash input only if cash payment is selected
    cashAmountInput->setVisible(cashPayment->isChecked());
    cashAmountLabel->setVisible(cashPayment->isChecked());
}

void OrderPaymentPopup::updateCashLabel(const QString &text) {

    QString numericText = text;
    numericText.remove(QRegularExpression("[^0-9.]"));


    bool ok;
    double amount = numericText.toDouble(&ok);
    if (ok) {
        QString formattedText = locale.toString(amount, 'f', 0);

        // Prevent infinite loop by only updating if needed
        if (formattedText != text) {
            cashAmountInput->blockSignals(true);
            cashAmountInput->setText(formattedText);
            cashAmountInput->blockSignals(false);
        }
        cashAmountLabel->setText(QString("Rp %1").arg(locale.toString(totalOrder - amount, 'f', 0)));
    } else {
        cashAmountLabel->setText("Rp 0");
    }
}

void OrderPaymentPopup::processPayment() {
    // Default Payment ID (Cash)
    int paymentId = 1;

    if (qrPayment->isChecked()) {
        paymentId = 2;  // QR Payment
    } else if (cardPayment->isChecked()) {
        paymentId = 3;  // Card Payment
    } else if (cashPayment->isChecked()) {
        bool valid;
        cashAmountInput->setFocus();
        QString numericText = cashAmountInput->text();
        numericText.remove(QRegularExpression("[^0-9.]"));

        double cashGiven = numericText.toDouble(&valid);
        double grandTotal = orderDetails["grandTotal"].toDouble();

        if (!valid || cashGiven < grandTotal) {
            QMessageBox::warning(this, "Error", "Insufficient cash entered!");
            return;
        }
    } else {
        QMessageBox::warning(this, "Error", "Please select a payment method!");
        return;
    }

    // Prepare JSON order data
    QJsonObject orderData;
    orderData["orderId"] = orderDetails["id"].toInt();
    orderData["paymentId"] = paymentId;

    QJsonDocument jsonDoc(orderData);
    QByteArray jsonData = jsonDoc.toJson();

    // Prepare API request
    QNetworkRequest request(QUrl("http://localhost:8088/api/v1/order"));
    request.setRawHeader("Authorization", "Bearer " + settings.value("authToken").toString().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager->put(request, jsonData);

    // Handle API response
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            if (responseData.isEmpty()) {
                qDebug() << "Empty response received!";
                QMessageBox::warning(this, "Error", "No response from the server.");
                return;
            }

            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonDoc.object();

            qDebug() << "Order processed successfully!";

            // Remove current tab (if applicable)
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


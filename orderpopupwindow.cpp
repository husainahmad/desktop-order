#include "orderpopupwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QTextBrowser>
#include <QJsonObject>
#include <QString>
#include <QJsonArray>
#include <QLocale>
#include <QTimeZone>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <orderprint.h>
#include <orderpaymentpopup.h>

OrderPopupWindow::OrderPopupWindow(const QJsonObject &order, QTabWidget *tabWidget, QWidget *parent)
    : QDialog(parent), networkManager(new QNetworkAccessManager(this)), orderDetails(order), tabWidget(tabWidget) {
    setWindowTitle("Order Details");
    setFixedSize(850, 700);

    locale = QLocale::English;

    QVBoxLayout *layout = new QVBoxLayout(this);

    // HTML viewer to display order details
    htmlViewer = new QTextBrowser(this);
    htmlViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    htmlViewer->setMinimumWidth(600);  // Set a minimum width
    htmlViewer->setMaximumWidth(16777215);  // Ensure no limit

    // Extract data from QJsonObject

    double subTotal = order.value("subTotal").toDouble();
    double discountTotal = order.value("discountTotal").toDouble();
    double grandTotal = order.value("grandTotal").toDouble();
    QJsonArray orderDetails = order.value("orderDetails").toArray();

    QString table = R"(
        <style>
            .scroll-container {
                width: 100%;
                overflow-x: auto;
            }
            table {
                width: 100%;
                min-width: 700px;
                border-collapse: collapse;
                border: 1px solid #ddd;
            }
            th, td {
                border: 1px solid #ddd;
                padding: 12px;
                text-align: left;
                font-size: 14px;
                white-space: nowrap;
            }
            th {
                background-color: #4CAF50;
                color: white;
            }
            tr:nth-child(even) {
                background-color: #f9f9f9;
            }

            /* Narrower Quantity column */
            th:nth-child(3), td:nth-child(3) {
                width: 20px;
                text-align: center;
            }
            /* Optional: shrink Price and Amount if needed */
            th:nth-child(4), td:nth-child(4),
            th:nth-child(5), td:nth-child(5) {
                width: 80px;
                text-align: right;
            }
        </style>
        <div class='scroll-container'>
    )";


    // Extract order metadata
    QString orderNo = order.value("orderNo").toString();
    QString customerName = order.value("customerName").toString();
    QString remark = order.value("remark").toString();


    QString createdAtStr = !order.value("createdAt").isNull() && order.value("createdAt").isString() ? order.value("createdAt").toString() : "";
    QDateTime createdAt;
    if (!createdAtStr.isEmpty()) {
        createdAt = QDateTime::fromString(createdAtStr, Qt::ISODate);
        createdAt.setTimeZone(QTimeZone::utc());  // Assume the input is in UTC
        createdAt = createdAt.toTimeZone(QTimeZone("Asia/Jakarta"));  // Convert to WIB (UTC+7)
    } else {
        createdAt = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone("Asia/Jakarta"));  // Default to WIB
    }

    QString orderDateTime = createdAt.toString("yyyy-MM-dd HH:mm");  // Ensure this is formatted properly

    // Header section with improved styling and remark
    table += "<div style='text-align: center; margin-bottom: 20px; padding: 10px; "
             "border-bottom: 2px solid #333; font-family: Arial, sans-serif;'>"
             "<p style='margin: 5px 0; font-size: 14px;'>"
             "<strong>Order No:</strong> " + orderNo + " | "
                         "<strong>Customer:</strong> " + customerName + " | "
                              "<strong>Date & Time:</strong> " + orderDateTime +
             "</p>"
             "<p style='margin-top: 10px; font-size: 14px; color: #666;'><strong>Remark:</strong> " + remark + "</p>"
                        "</div>";

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

    table += "</table></div>";

    htmlViewer->setHtml(table);
    layout->addWidget(htmlViewer);

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

    // Button layout - horizontal
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    // Print button
    printButton = new QPushButton("Print", this);
    printButton->setStyleSheet(buttonStyle);
    connect(printButton, &QPushButton::clicked, this, &OrderPopupWindow::printOrder);
    buttonLayout->addWidget(printButton);

    // Kitchen Print button
    printKitchenButton = new QPushButton("Kitchen Print", this);
    printKitchenButton->setStyleSheet(buttonStyle);
    connect(printKitchenButton, &QPushButton::clicked, this, &OrderPopupWindow::kitchenPrintOrder);
    buttonLayout->addWidget(printKitchenButton);

    QString status = order.value("status").toString().toUpper();

    // PAY button for CONFIRMED orders
    if (status == "CONFIRMED" || status == "PAID") {
        if (status == "CONFIRMED") {
            payButton = new QPushButton("Pay", this);
            payButton->setStyleSheet(buttonStyle);
            connect(payButton, &QPushButton::clicked, this, &OrderPopupWindow::payOrder);
            buttonLayout->addWidget(payButton);
        }

        voidButton = new QPushButton("Void", this);
        voidButton->setStyleSheet(buttonStyle);
        connect(voidButton, &QPushButton::clicked, this, &OrderPopupWindow::voidOrder);
        buttonLayout->addWidget(voidButton);
    }

    // Close button
    closeButton = new QPushButton("Close", this);
    closeButton->setStyleSheet(buttonStyle);
    connect(closeButton, &QPushButton::clicked, this, &OrderPopupWindow::closeWindow);
    buttonLayout->addWidget(closeButton);

    // Add the horizontal button layout to the main vertical layout
    layout->addLayout(buttonLayout);

}

OrderPopupWindow::~OrderPopupWindow() {
}

void OrderPopupWindow::payOrder() {
    OrderPaymentPopup *popup = new OrderPaymentPopup(this->orderDetails, this->tabWidget, this);

    connect(popup, &QDialog::accepted, this, [=]() {
      this->accept();  // Close OrderPopupWindow only if payment was successful
    });

    popup->exec();  // Show payment popup modally
}

void OrderPopupWindow::voidOrder() {
    QString authToken = configSetting.getValue("authToken").toString().trimmed();

    int orderId = orderDetails["id"].toInt();  // Use toInt()
    QUrl apiUrl(QString(configSetting.getApiEndpoint("order", "void")).arg(orderId));

    QNetworkRequest request(apiUrl);
    request.setRawHeader("Authorization", "Bearer " + authToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Empty body
    QNetworkReply *reply = networkManager->put(request, QByteArray());

    // Handle API response
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();

            if (responseData.isEmpty()) {
                qDebug() << "Empty response received!";
                QMessageBox::warning(this, "Error", "No response from the server.");
            } else {
                qDebug() << "Void successful: " << responseData;
                this->accept();  // Close OrderPopupWindow
            }
        } else {
            qDebug() << "Void order failed: " << reply->errorString();
            QMessageBox::warning(this, "Error", "Failed to void the order. Please try again.");
        }

        reply->deleteLater();
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
    this->accept(); // Close the window
}

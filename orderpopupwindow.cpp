#include "orderpopupwindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QJsonObject>
#include <QString>
#include <QJsonArray>
#include <QLocale>
#include <QTimeZone>


OrderPopupWindow::OrderPopupWindow(const QJsonObject &order, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Order Details");
    setFixedSize(600, 500);

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

    QString table = "<style>"
                    "body, html { width: 100%; margin: 0; padding: 0; }"
                    ".container { width: 100%; padding: 10px; box-sizing: border-box; }"
                    "table { width: 100%; border-collapse: collapse; border: 1px solid #ddd; }"
                    "th, td { border: 1px solid #ddd; padding: 12px; text-align: left; font-size: 14px; }"
                    "th { background-color: #4CAF50; color: white; }"
                    "tr:nth-child(even) { background-color: #f9f9f9; }"
                    "td:last-child, td:nth-child(3), td:nth-child(4) { text-align: right; }"
                    "</style>";

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

    htmlViewer->setHtml(table);
    layout->addWidget(htmlViewer);

    // Print button
    printButton = new QPushButton("Print", this);
    connect(printButton, &QPushButton::clicked, this, &OrderPopupWindow::printOrder);
    layout->addWidget(printButton);

    // Close button
    closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &OrderPopupWindow::closeWindow);
    layout->addWidget(closeButton);
}

OrderPopupWindow::~OrderPopupWindow() {
    // Qt handles child widget cleanup automatically
}
void OrderPopupWindow::printOrder() {
    // TODO
}

void OrderPopupWindow::closeWindow() {
    this->accept(); // Close the window
}

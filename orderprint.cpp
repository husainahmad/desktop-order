#include "orderprint.h"

#include <QJsonObject>
#include <QString>
#include <QJsonArray>
#include <QProcess>

#include <QLocale>
#include <QTimeZone>

OrderPrint::OrderPrint(const QJsonObject &order) : orderDetails(order) {
    locale = QLocale::English;
}

void OrderPrint::sendToReceiptPrinter() {
    QByteArray receiptData;

    // ESC/POS: Reset, Center
    receiptData.append("\x1B\x40");     // Reset printer
    receiptData.append("\x1B\x61\x01"); // Center text

    receiptData.append(" Kopi Harmoni \n");
    receiptData.append("==============================\n");
    receiptData.append("\x1B\x40");

    // Left-align
    receiptData.append("\x1B\x61\x00");
    receiptData.append(" Customer: " + orderDetails["customerName"].toString().toUtf8() + "\n");
    QString orderNo = orderDetails["orderNo"].toString();
    QString orderDateStr = orderDetails["createdAt"].toString();
    QDateTime orderDate = QDateTime::fromString(orderDateStr, Qt::ISODate);
    orderDate.setTimeZone(QTimeZone::utc());
    orderDate = orderDate.toTimeZone(QTimeZone("Asia/Jakarta"));

    receiptData.append("No : " + orderNo.toUtf8() + "\n");
    receiptData.append("Date : " + orderDate.toString("yyyy-MM-dd HH:mm").toUtf8() + "\n\n");
    receiptData.append("--------------------------------\n");
    receiptData.append("Item             Qty  Total\n");
    receiptData.append("--------------------------------\n");

    QJsonArray orderDetailsArray = orderDetails["orderDetails"].toArray();
    for (const QJsonValue &itemValue : orderDetailsArray) {
        QJsonObject itemObj = itemValue.toObject();
        QString productName = itemObj["productName"].toString();
        QJsonArray skus = itemObj["orderDetailSkus"].toArray();

        double totalQty = 0;
        double totalPrice = 0;

        for (const QJsonValue &skuVal : skus) {
            QJsonObject skuObj = skuVal.toObject();
            double qty = skuObj["quantity"].toDouble();
            double price = skuObj["price"].toDouble(); // Ensure this exists
            totalQty += qty;
            totalPrice += skuObj["amount"].toDouble();
        }

        QString line = QString("%1\n")
                           .arg(productName.leftJustified(15));
        receiptData.append(line.toUtf8());

        for (const QJsonValue &skuVal : skus) {
            QJsonObject skuObj = skuVal.toObject();
            QString skuName = skuObj["skuName"].toString();
            double qty = skuObj["quantity"].toDouble();
            double price = skuObj["price"].toDouble();
            double lineTotal = skuObj["amount"].toDouble();

            QString skuLine = QString("  - %1%2%3\n")
                                  .arg(skuName.leftJustified(13))
                                  .arg(QString::number(qty, 'f', 0).rightJustified(4))
                                  .arg(locale.toString(lineTotal, 'f', 0).rightJustified(10));
            receiptData.append(skuLine.toUtf8());
        }
    }

    receiptData.append("--------------------------------\n");

    double grandTotal = orderDetails["grandTotal"].toDouble();
    receiptData.append(QString("Total: Rp %1\n").arg(locale.toString(grandTotal, 'f', 0)).toUtf8());

    QString note = orderDetails["remark"].toString();
    if (!note.isEmpty()) {
        receiptData.append("Note: " + note.toUtf8() + "\n");
    }

    receiptData.append("\n\n\n");         // Add spacing
    receiptData.append("\x1D\x56\x01");   // Cut paper

    // Send to printer
    QProcess process;
    process.start("lp", QStringList() << "-d" << settingConfig.getApiEndpoint("printer", "receipt") << "-o" << "raw");
    process.write(receiptData);
    process.closeWriteChannel();
    process.waitForFinished();
}

void OrderPrint::sendToKitchenPrinter() {
    // 1. Group items by category
    QMap<int, QList<QJsonObject>> categoryMap;
    QJsonArray orderDetailsArray = orderDetails["orderDetails"].toArray();
    for (const QJsonValue &itemValue : orderDetailsArray) {
        QJsonObject itemObj = itemValue.toObject();
        int categoryId = itemObj["categoryId"].toInt(); // Assuming exists
        categoryMap[categoryId].append(itemObj);
    }

    // 2. Print each category group once (default)
    for (auto it = categoryMap.begin(); it != categoryMap.end(); ++it) {
        const QList<QJsonObject> &items = it.value();

        QByteArray receiptData;
        receiptData.append("\x1B\x40");     // Reset
        receiptData.append("\x1B\x61\x01"); // Center
        receiptData.append(" Kopi Harmoni - KP \n");
        receiptData.append("\x1B\x40");
        receiptData.append("\x1B\x61\x00"); // Left align
        receiptData.append(" Customer: " + orderDetails["customerName"].toString().toUtf8() + "\n");

        QString orderNo = orderDetails["orderNo"].toString();
        QString orderDateStr = orderDetails["createdAt"].toString();
        QDateTime orderDate = QDateTime::fromString(orderDateStr, Qt::ISODate);
        orderDate.setTimeZone(QTimeZone::utc());
        orderDate = orderDate.toTimeZone(QTimeZone("Asia/Jakarta"));

        receiptData.append("No : " + orderNo.toUtf8() + "\n");
        receiptData.append("Date : " + orderDate.toString("yyyy-MM-dd HH:mm").toUtf8() + "\n\n");
        receiptData.append("--------------------------------\n");
        receiptData.append("Item                Qty\n");
        receiptData.append("--------------------------------\n");

        for (const QJsonObject &itemObj : items) {
            QString productName = itemObj["productName"].toString();
            QJsonArray skus = itemObj["orderDetailSkus"].toArray();

            double totalQty = 0;
            for (const QJsonValue &skuVal : skus) {
                totalQty += skuVal.toObject()["quantity"].toDouble();
            }

            // Print product line
            QString line = QString("%1\n")
                               .arg(productName.leftJustified(18));
            receiptData.append(line.toUtf8());

            // Print SKU details indented
            for (const QJsonValue &skuVal : skus) {
                QJsonObject skuObj = skuVal.toObject();
                QString skuName = skuObj["skuName"].toString(); // Adjust based on your actual key
                double skuQty = skuObj["quantity"].toDouble();

                QString skuLine = QString("  - %1 %2x\n")
                                      .arg(skuName.leftJustified(16))
                                      .arg(skuQty, 2, 'f', 0);
                receiptData.append(skuLine.toUtf8());
            }
        }

        if (!orderDetails["remark"].toString().isEmpty()) {
            receiptData.append("Note: " + orderDetails["remark"].toString().toUtf8() + "\n");
        }

        receiptData.append("\n\n\n");       // Spacing
        receiptData.append("\x1D\x56\x01"); // Cut paper

        // Send to printer
        QProcess process;
        process.start("lp", QStringList() << "-d" << settingConfig.getApiEndpoint("printer", "kitchen") << "-o" << "raw");
        process.write(receiptData);
        process.closeWriteChannel();
        process.waitForFinished();
    }
}

void OrderPrint::sendSettlementToReceiptPrinter() {
    QByteArray receiptData;

    // ESC/POS: Reset & Center
    receiptData.append("\x1B\x40");     // Initialize
    receiptData.append("\x1B\x61\x01"); // Center alignment
    receiptData.append(" DAILY SETTLEMENT \n");
    receiptData.append("==============================\n");


    int totalOrders = orderDetails["totalOrders"].toInt();
    double totalSales = orderDetails["totalSales"].toDouble();
    double totalDiscounts = orderDetails["totalDiscounts"].toDouble();
    double totalTax = orderDetails["totalTax"].toDouble();
    double totalNetSales = orderDetails["totalNetSales"].toDouble();

    receiptData.append(QString("Total Orders    : %1\n").arg(totalOrders).toUtf8());
    receiptData.append(QString("Total Sales     : %1\n").arg(locale.toString(totalSales, 'f', 0)).toUtf8());
    receiptData.append(QString("Discounts       : %1\n").arg(locale.toString(totalDiscounts, 'f', 0)).toUtf8());
    receiptData.append(QString("Tax             : %1\n").arg(locale.toString(totalTax, 'f', 0)).toUtf8());
    receiptData.append("--------------------------------\n");
    receiptData.append(QString("Net Sales       : %1\n").arg(locale.toString(totalNetSales, 'f', 0)).toUtf8());

    receiptData.append("==============================\n");
    receiptData.append("Payment Breakdown\n");
    receiptData.append("--------------------------------\n");

    QJsonObject paymentBreakdown = orderDetails["paymentBreakdown"].toObject();
    for (const QString &key : paymentBreakdown.keys()) {
        double amount = paymentBreakdown[key].toDouble();
        receiptData.append(QString("%1 : %2\n")
                               .arg(key.leftJustified(16))
                               .arg(locale.toString(amount, 'f', 0).rightJustified(10))
                               .toUtf8());
    }

    receiptData.append("==============================\n");
    receiptData.append("Printed on: " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm").toUtf8() + "\n");
    receiptData.append("\n\n\n");
    receiptData.append("\x1D\x56\x01"); // Cut paper

    // Send to printer
    QProcess process;
    process.start("lp", QStringList() << "-d" << settingConfig.getApiEndpoint("printer", "receipt") << "-o" << "raw");
    process.write(receiptData);
    process.closeWriteChannel();
    process.waitForFinished();
}

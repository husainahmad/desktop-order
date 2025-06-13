#include "orderprint.h"

#include <QJsonObject>
#include <QString>
#include <QJsonArray>
#include <QProcess>

#include <QLocale>
#include <QTimeZone>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

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

        QString line = QString("%1\n")
                           .arg(productName.leftJustified(15));
        receiptData.append(line.toUtf8());

        for (const QJsonValue &skuVal : skus) {
            QJsonObject skuObj = skuVal.toObject();
            QString skuName = skuObj["skuName"].toString();
            double qty = skuObj["quantity"].toDouble();
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

    QString paymentName = "UNPAID";

    if (orderDetails.contains("orderPayment") && orderDetails["orderPayment"].isObject()) {
        QJsonObject orderPayment = orderDetails["orderPayment"].toObject();
        if (!orderPayment.isEmpty() && orderPayment.contains("paymentId")) {
            int paymentId = orderPayment["paymentId"].toInt();

            switch (paymentId) {
            case 1:
                paymentName = "Cash";
                break;
            case 2:
                paymentName = "QR Payment";
                break;
            case 3:
                paymentName = "Card Payment";
                break;
            default:
                paymentName = "UNPAID";
                break;
            }
        }
    }

    receiptData.append("Payment: " + paymentName.toUtf8() + "\n");


    receiptData.append(QString("Total: Rp %1\n")
                           .arg(locale.toString(grandTotal, 'f', 0)).toUtf8());

    QString note = orderDetails["remark"].toString();
    if (!note.isEmpty()) {
        receiptData.append("Note: " + note.toUtf8() + "\n");
    }

    receiptData.append("\x1B\x61\x01");
    receiptData.append("Terima kasih\n");
    receiptData.append("Printed on: " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm").toUtf8() + "\n");
    receiptData.append("\n\n\n");         // Add spacing
    receiptData.append("\x1D\x56\x01");   // Cut paper

    #ifdef Q_OS_WIN
        if (!sendRawDataToPrinter(settingConfig.getApiEndpoint("printer", "receipt"), receiptData)) {
            qDebug() << "Failed to send data to printer on Windows.";
        }
    #else
        // Send to printer
        QProcess process;
        process.start("lp", QStringList() << "-d" << settingConfig.getApiEndpoint("printer", "receipt") << "-o" << "raw");
        process.write(receiptData);
        process.closeWriteChannel();
        process.waitForFinished();
    #endif

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
;
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
        receiptData.append("Printed on: " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm").toUtf8() + "\n");
        receiptData.append("\n\n\n");       // Spacing
        receiptData.append("\x1D\x56\x01"); // Cut paper

        #ifdef Q_OS_WIN
                if (!sendRawDataToPrinter(settingConfig.getApiEndpoint("printer", "kitchen"), receiptData)) {
                    qDebug() << "Failed to send data to printer on Windows.";
                }
        #else
                // Send to printer
                QProcess process;
                process.start("lp", QStringList() << "-d" << settingConfig.getApiEndpoint("printer", "kitchen") << "-o" << "raw");
                process.write(receiptData);
                process.closeWriteChannel();
                process.waitForFinished();
        #endif
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

    #ifdef Q_OS_WIN
        if (!sendRawDataToPrinter(settingConfig.getApiEndpoint("printer", "receipt"), receiptData)) {
            qDebug() << "Failed to send data to printer on Windows.";
        }
    #else
    // Send to printer
        QProcess process;
        process.start("lp", QStringList() << "-d" << settingConfig.getApiEndpoint("printer", "receipt") << "-o" << "raw");
        process.write(receiptData);
        process.closeWriteChannel();
        process.waitForFinished();
    #endif
}

#ifdef Q_OS_WIN
bool OrderPrint::sendRawDataToPrinter(const QString &printerName, const QByteArray &data) {
    HANDLE hPrinter;
    DOC_INFO_1A docInfo;
    DWORD bytesWritten;

    if (!OpenPrinterA((LPSTR)printerName.toUtf8().constData(), &hPrinter, nullptr)) {
        qDebug() << "Failed to open printer:" << printerName;
        return false;
    }

    docInfo.pDocName = (LPSTR)"POS Print Job";
    docInfo.pOutputFile = nullptr;
    docInfo.pDatatype = (LPSTR)"RAW";

    if (StartDocPrinterA(hPrinter, 1, (LPBYTE)&docInfo) == 0) {
        ClosePrinter(hPrinter);
        qDebug() << "StartDocPrinter failed";
        return false;
    }

    if (!StartPagePrinter(hPrinter)) {
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);
        qDebug() << "StartPagePrinter failed";
        return false;
    }

    if (!WritePrinter(hPrinter, (LPVOID)data.constData(), static_cast<DWORD>(data.size()), &bytesWritten)) {
        qDebug() << "WritePrinter failed";
        EndPagePrinter(hPrinter);
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);
        return false;
    }

    EndPagePrinter(hPrinter);
    EndDocPrinter(hPrinter);
    ClosePrinter(hPrinter);

    return true;
}
#endif


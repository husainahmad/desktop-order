#ifndef ORDERPOPUPWINDOW_H
#define ORDERPOPUPWINDOW_H

#include <QDialog>
#include <QTextBrowser>
#include <QJsonObject>
#include <QLocale>
#include <QTabWidget>
#include <QNetworkAccessManager>
#include "setting.h"

class QTextBrowser;
class QPushButton;

class OrderPopupWindow : public QDialog {
    Q_OBJECT

public:
    explicit OrderPopupWindow(const QJsonObject &order, QTabWidget *tabWidget, QWidget *parent = nullptr);
    ~OrderPopupWindow();

private slots:
    void printOrder();
    void kitchenPrintOrder();
    void closeWindow();
    void payOrder();
    void voidOrder();
private:
    QTextBrowser *htmlViewer;
    QPushButton *printButton;
    QPushButton *printKitchenButton;
    QPushButton *closeButton;
    QPushButton *payButton;
    QPushButton *voidButton;
    QLocale locale;
    QNetworkAccessManager *networkManager;
    QJsonObject orderDetails;
    QTabWidget *tabWidget;
    Setting configSetting;
};

#endif // ORDERPOPUPWINDOW_H

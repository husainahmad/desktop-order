#ifndef ORDERPOPUPWINDOW_H
#define ORDERPOPUPWINDOW_H

#include <QDialog>
#include <QJsonObject>
#include <QLocale>
#include <QTabWidget>
#include "setting.h"

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
    QPushButton *printButton;
    QPushButton *printKitchenButton;
    QPushButton *closeButton;
    QPushButton *payButton;
    QPushButton *voidButton;
    QLocale locale;
    QJsonObject orderDetails;
    QTabWidget *tabWidget;
    Setting configSetting;
};

#endif // ORDERPOPUPWINDOW_H
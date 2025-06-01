#ifndef ORDERPOPUPWINDOW_H
#define ORDERPOPUPWINDOW_H

#include <QDialog>
#include <QTextBrowser>
#include <QJsonObject>
#include <QLocale>
#include "setting.h"

class QTextBrowser;
class QPushButton;

class OrderPopupWindow : public QDialog {
    Q_OBJECT

public:
    explicit OrderPopupWindow(const QJsonObject &order, QWidget *parent = nullptr);
    ~OrderPopupWindow();

private slots:
    void printOrder();
    void kitchenPrintOrder();
    void closeWindow();
private:
    QTextBrowser *htmlViewer;
    QPushButton *printButton;
    QPushButton *printKitchenButton;
    QPushButton *closeButton;
    QLocale locale;
    QJsonObject orderDetails;
    Setting configSetting;

};

#endif // ORDERPOPUPWINDOW_H

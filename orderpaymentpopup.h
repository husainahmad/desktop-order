#ifndef ORDERPAYMENTPOPUP_H
#define ORDERPAYMENTPOPUP_H

#include <QDialog>
#include <QJsonObject>
#include <QLabel>
#include <QTabWidget>
#include <QSettings>
#include <QGridLayout>
#include <QButtonGroup>
#include "setting.h"

class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QLocale;

class OrderPaymentPopup : public QDialog {
    Q_OBJECT

public:
    explicit OrderPaymentPopup(const QJsonObject &order, QTabWidget *tabWidget, QWidget *parent = nullptr);
    ~OrderPaymentPopup();

private slots:
    void processPayment();
    void selectPaymentMethod(int id);
    void onCashButtonClicked();
    void clearCashAmount();
    void setExactCash();

private:
    void updateCashDisplay();

    QJsonObject orderDetails;
    QButtonGroup *paymentGroup;
    QPushButton *qrPaymentBtn;
    QPushButton *cardPaymentBtn;
    QPushButton *cashPaymentBtn;
    QPushButton *payButton;
    QLocale locale;
    double totalOrder;
    QTabWidget *tabWidget;
    Setting configSetting;

    // Cash payment panel widgets
    QWidget *cashPanel;
    QLabel *cashTotalText;
    QLabel *cashReceivedText;
    QLabel *cashChangeText;
    QPushButton *clearButton;
    QPushButton *exactButton;
    double cashGiven;
    double cashChange;
    double subTotal;
    QTableWidgetItem *totalSummaryItem;

    QString customerName;
    QString remark;
    int serviceTypeId;
};

#endif // ORDERPAYMENTPOPUP_H
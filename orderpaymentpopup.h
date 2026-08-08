#ifndef ORDERPAYMENTPOPUP_H
#define ORDERPAYMENTPOPUP_H

#include <QDialog>
#include <QJsonObject>
#include <QLabel>
#include <QTabWidget>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QGridLayout>
#include <QButtonGroup>
#include "setting.h"

class QPushButton;
class QTextBrowser;
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
    void payCash();

private:
    void updateCashDisplay();

    QNetworkAccessManager *networkManager;
    QJsonObject orderDetails;
    QButtonGroup *paymentGroup;
    QPushButton *qrPaymentBtn;
    QPushButton *cardPaymentBtn;
    QPushButton *cashPaymentBtn;
    QTextBrowser *totalHtmlWidget;
    QPushButton *payButton;
    QPushButton *cancelButton;
    QLocale locale;
    double totalOrder;
    QTabWidget *tabWidget;
    Setting configSetting;

    // Cash payment panel widgets
    QWidget *cashPanel;
    QLabel *cashTotalText;
    QLabel *cashReceivedText;
    QLabel *cashChangeText;
    QGridLayout *cashGrid;
    QPushButton *bayarButton;
    QPushButton *clearButton;
    QPushButton *exactButton;
    double cashGiven;
    double cashChange;
};

#endif // ORDERPAYMENTPOPUP_H

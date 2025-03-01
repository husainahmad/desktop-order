#ifndef ORDERPAYMENTPOPUP_H
#define ORDERPAYMENTPOPUP_H

#include <QDialog>
#include <QJsonObject>
#include <QLabel>
#include <QTabWidget>
#include <QNetworkAccessManager>
#include <QSettings>

class QRadioButton;
class QLineEdit;
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
    void updateUI();
    void updateCashLabel(const QString &text);

private:
    QNetworkAccessManager *networkManager;
    QJsonObject orderDetails;
    QRadioButton *qrPayment;
    QRadioButton *cardPayment;
    QRadioButton *cashPayment;
    QLineEdit *cashAmountInput;
    QTextBrowser *totalHtmlWidget;
    QPushButton *payButton;
    QPushButton *cancelButton;
    QLocale locale;
    QLabel *cashAmountLabel;
    double totalOrder;
    QTabWidget *tabWidget;
    QSettings settings;

};

#endif // ORDERPAYMENTPOPUP_H

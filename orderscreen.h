#ifndef ORDERSCREEN_H
#define ORDERSCREEN_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QLabel>

namespace Ui {
class OrderScreen;
}

class OrderScreen : public QWidget
{
    Q_OBJECT

public:
    explicit OrderScreen(QWidget *parent = nullptr);
    ~OrderScreen();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void addNewTabWithOrderForm();
    void addProductToList();
    void fetchDataFromAPI();
    void parseJsonResponse(const QByteArray &responseData);
    void onTabChanged(int index);
    void onDateChanged(const QDate &selectedDate);

private:
    Ui::OrderScreen *ui;
    QTabWidget *tabWidget;
    QTableWidget *tableWidget;
    QWidget *firstTab;
    QLabel *titleLabel;
    QPushButton *summaryButton;

    // Input fields for order details
    QLineEdit *storeIdInput;
    QLineEdit *storeNameInput;
    QLineEdit *customerNameInput;
    QLineEdit *remarkInput;
    QListWidget *productListWidget;

    // Input fields for adding a product
    QLineEdit *productIdInput;
    QLineEdit *productNameInput;
    QLineEdit *skuIdInput;
    QLineEdit *skuNameInput;
    QLineEdit *quantityInput;
    QLineEdit *productRemarkInput;

    QVBoxLayout *summaryLayout;
    QWidget *summaryWidget;
    QVBoxLayout *ordersLayout;
    QWidget *ordersWidget;

    QNetworkAccessManager *networkManager;
    QSettings settings;
    QLocale locale;
};

#endif // ORDERSCREEN_H

#ifndef ORDERFORM_H
#define ORDERFORM_H

#include <QWidget>
#include <QSplitter>
#include <QBoxLayout>
#include <QLineEdit>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QTreeView>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QTextEdit>
#include <QLineEdit>
#include "product.h"
#include "order.h"
#include <QLocale>

namespace Ui {
class OrderForm;
}

class OrderForm : public QWidget
{
    Q_OBJECT

public:
    explicit OrderForm(QTabWidget *tabWidget, QWidget *parent = nullptr);
    ~OrderForm();

private slots:
    void fetchDataFromAPI();
    void onDataReceived(QNetworkReply *reply);
    void updateCategoryLeftPanel(const QJsonArray &dataArray);
    void fetchDataDetailProduct(QString id);
    void onDataDetailProductReceived(QNetworkReply *reply);
    void updateProductLeftTopPanel(const QJsonArray &dataArray);
    void filterProducts(const QString &query);
    bool checkSku(const Sku &sku, const bool &add, OrderItem &orderItem);
    void removeSku(const int &productId, const Sku &sku);
    void onConfirmButtonClicked();
    void updateOrderData(const Product &product, const Sku &sku, const bool &add);
    QList<Sku> getSkuFromItem(const QJsonObject &object);
    QWidget* createProductGroupWidget(const Product &product);
    void updateCurrentTabName(const QString &newName);
private:
    Ui::OrderForm *ui;
    QTabWidget *tabWidget;
    QNetworkAccessManager *networkManager;
    QWidget *bottomLeftWidget;
    QGridLayout *buttonGridLayout;
    QList<QWidget*> productWidgets;
    QGridLayout *gridLayout;
    QList<Product> products;
    QLineEdit *searchBox;
    QSettings settings;
    QTreeView *treeView;
    QStandardItemModel *treeModel;
    Order order;
    QVBoxLayout *cartLayout;
    QTextEdit *remarkText;
    QLineEdit *customerNameText;
    QLineEdit *discountText;
    QLineEdit *subTotalText;
    QLineEdit *totalText;
    void populateOrderOnRightPanel();
    QLocale locale;
signals:
    void updateQuantity(const Product &product, const Sku &sku, const bool &add);
};

#endif // ORDERFORM_H

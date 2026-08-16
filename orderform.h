#ifndef ORDERFORM_H
#define ORDERFORM_H

#include <QWidget>
#include <QSplitter>
#include <QBoxLayout>
#include <QLineEdit>
#include <QHash>
#include <QTabWidget>

#include <QHBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QTextEdit>
#include <QScrollArea>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QResizeEvent>
#include "product.h"
#include "order.h"
#include <QLocale>
#include "setting.h"
#include "ordertabbutton.h"

class OrderCartWidget;

namespace Ui {
class OrderForm;
}

class OrderForm : public QWidget
{
    Q_OBJECT

public:
    explicit OrderForm(QTabWidget *tabWidget, QWidget *parent = nullptr);
    ~OrderForm();
    void updateOrderData(const Product &product, const Sku &sku, const bool &add);
    void setTabButtonWidget(OrderTabButton *tabButton, const QString &orderTitle);
    void updateTabSubtitle();
    int getSkuQuantity(int productId, int skuId) const;

public slots:
    void updateQuantity(const Product &product, const Sku &sku, bool add);

private slots:
    void fetchDataFromAPI();
    void onDataReceived(const QJsonObject &response);
    void updateCategoryLeftPanel(const QJsonArray &dataArray);
    void fetchDataDetailProduct(QString id);
    void updateProductLeftTopPanel(const QJsonArray &dataArray);
    void filterProducts(const QString &query);
    bool checkSku(const Sku &sku, const bool &add, OrderItem &orderItem);
    void onConfirmButtonClicked();

    QList<Sku> getSkuFromItem(const QJsonObject &object);
    QWidget* createProductGroupWidget(const Product &product);
    void updateCurrentTabName(const QString &newName);
    void printReceipt();
private:
    Ui::OrderForm *ui;
    QTabWidget *tabWidget;
    QWidget *bottomLeftWidget;
    QHBoxLayout *buttonGridLayout;
    QScrollArea *categoryScrollArea;
    QList<QPushButton *> categoryButtons;
    QHash<QPushButton *, QString> categoryButtonIds;
    QPushButton *activeCategoryButton = nullptr;
    QString activeCategoryId;
    QList<QWidget*> productWidgets;
    QGridLayout *gridLayout;
    QList<Product> products;
    QLineEdit *searchBox;
    Order order;
    QVBoxLayout *cartLayout;
    QHash<int, OrderCartWidget*> cartWidgets;
    QTextEdit *remarkText;
    QLineEdit *customerNameText;
    QLineEdit *discountText;
    QLineEdit *totalText;
    QButtonGroup *serviceTypeGroup;
    QRadioButton *dineInBtn;
    QRadioButton *takeawayBtn;
    void populateOrderOnRightPanel();
    void layoutCategoryButtons();
    void setActiveCategory(QPushButton *button, const QString &id);
    QLocale locale;
    Setting settingConfig;
    QString tabName;
    OrderTabButton *tabButton = nullptr;

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // ORDERFORM_H
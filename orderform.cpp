#include "orderform.h"
#include "ui_orderform.h"
#include "touchutils.h"
#include "screenutils.h"
#include "apiclient.h"
#include <QLabel>
#include <QTextEdit>
#include <QTableWidgetItem>
#include <QScrollArea>
#include <QPushButton>
#include <QLineEdit>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <product.h>
#include <cartitemwidget.h>
#include <ordercartwidget.h>
#include "orderitem.h"
#include "orderitemsku.h"
#include "productimage.h"
#include "productwidget.h"
#include "skuwidget.h"
#include "tokenmanager.h"
#include <QLocale>
#include <QMessageBox>
#include <toast.h>
#include <orderpaymentpopup.h>
#include <QProcess>
#include <orderprint.h>

OrderForm::OrderForm(QTabWidget *tabWidget, QWidget *parent)
    : QWidget(parent), ui(new Ui::OrderForm), tabWidget(tabWidget)
{
    ui->setupUi(this);

    locale = QLocale::English;

    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical, this);

    QWidget *topLeftContainer = new QWidget();
    QVBoxLayout *topLeftLayout = new QVBoxLayout(topLeftContainer);

    QWidget *searchWidget = new QWidget();
    QHBoxLayout *searchLayout = new QHBoxLayout(searchWidget);

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Search products...");
    searchBox->setFixedHeight(ScreenUtils::px(38));

    connect(searchBox, &QLineEdit::textChanged, this, &OrderForm::filterProducts);

    QPushButton *searchButton = new QPushButton("Search", this);
    searchButton->setObjectName("primaryButton");
    searchButton->setFixedSize(ScreenUtils::px(90), ScreenUtils::px(38));

    searchLayout->addWidget(searchBox);
    searchLayout->addWidget(searchButton);
    searchWidget->setLayout(searchLayout);

    topLeftLayout->addWidget(searchWidget);

    QWidget *topLeftWidget = new QWidget();
    gridLayout = new QGridLayout(topLeftWidget);
    gridLayout->setContentsMargins(8, 8, 8, 8);
    gridLayout->setSpacing(12);

    topLeftWidget->setLayout(gridLayout);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(topLeftWidget);
    TouchUtils::enableTouchScrolling(scrollArea);

    topLeftLayout->addWidget(scrollArea);
    topLeftContainer->setLayout(topLeftLayout);

    bottomLeftWidget = new QWidget();
    buttonGridLayout = new QGridLayout(bottomLeftWidget);
    QScrollArea *bottomScrollArea = new QScrollArea();
    bottomScrollArea->setWidgetResizable(true);
    bottomScrollArea->setWidget(bottomLeftWidget);
    TouchUtils::enableTouchScrolling(bottomScrollArea);

    leftSplitter->addWidget(topLeftContainer);
    leftSplitter->addWidget(bottomScrollArea);

    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);

    QLabel *summaryLabel = new QLabel("Order Summary", this);
    summaryLabel->setObjectName("sectionHeader");
    rightLayout->addWidget(summaryLabel);

    QWidget *cartPanel = new QWidget();
    cartPanel->setObjectName("cardPanel");
    cartLayout = new QVBoxLayout(cartPanel);
    cartLayout->setContentsMargins(8, 8, 8, 8);
    cartLayout->setSpacing(8);

    QScrollArea *rightScrollArea = new QScrollArea();
    rightScrollArea->setWidgetResizable(true);
    rightScrollArea->setWidget(cartPanel);
    TouchUtils::enableTouchScrolling(rightScrollArea);

    rightLayout->addWidget(rightScrollArea, 9);

    QWidget *summaryPanel = new QWidget();
    summaryPanel->setObjectName("cardPanel");
    QGridLayout *summaryGridLayout = new QGridLayout(summaryPanel);
    summaryGridLayout->setContentsMargins(12, 10, 12, 10);
    summaryGridLayout->setSpacing(8);

    summaryGridLayout->addWidget(new QLabel("Total"), 0, 0);
    totalText = new QLineEdit();
    totalText->setPlaceholderText("Total");
    totalText->setAlignment(Qt::AlignRight);
    totalText->setFixedHeight(ScreenUtils::px(34));
    totalText->setObjectName("totalDisplay");
    summaryGridLayout->addWidget(totalText, 0, 1);
    summaryPanel->setLayout(summaryGridLayout);

    rightLayout->addWidget(summaryPanel);

    customerNameText = new QLineEdit(this);
    customerNameText->setVisible(false);
    connect(customerNameText, &QLineEdit::textChanged, this, &OrderForm::updateCurrentTabName);

    discountText = new QLineEdit(this);
    discountText->setVisible(false);

    remarkText = new QTextEdit(this);
    remarkText->setVisible(false);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *confirmButton = new QPushButton("Confirm", this);
    QPushButton *printButton = new QPushButton("Print", this);

    confirmButton->setFixedSize(ScreenUtils::px(120), ScreenUtils::px(44));
    printButton->setFixedSize(ScreenUtils::px(120), ScreenUtils::px(44));

    confirmButton->setObjectName("successButton");
    printButton->setObjectName("primaryButton");

    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(printButton);
    buttonLayout->setAlignment(Qt::AlignCenter);

    QWidget *buttonContainer = new QWidget();
    buttonContainer->setLayout(buttonLayout);

    rightLayout->addWidget(buttonContainer);

    rightPanel->setLayout(rightLayout);

    mainSplitter->addWidget(leftSplitter);
    mainSplitter->addWidget(rightPanel);

    mainSplitter->setStretchFactor(0, 7);
    mainSplitter->setStretchFactor(1, 3);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(mainSplitter);
    this->setLayout(layout);

    fetchDataFromAPI();

    connect(confirmButton, &QPushButton::clicked, this, &OrderForm::onConfirmButtonClicked);
    connect(printButton, &QPushButton::clicked, this, &OrderForm::printReceipt);
}

void OrderForm::fetchDataFromAPI() {
    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/category_cache.json";
    QFile cacheFile(cachePath);
    if (cacheFile.exists() && cacheFile.open(QIODevice::ReadOnly)) {
        QByteArray data = cacheFile.readAll();
        cacheFile.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray dataArray = doc.object()["data"].toArray();
        updateCategoryLeftPanel(dataArray);
        return;
    }

    const QUrl url(settingConfig.getApiEndpoint("menu", "category") + "/tier");
    ApiClient::instance().get(url, [this](const QJsonObject &response) {
        onDataReceived(response);
    }, [](const QString &message, int) {
        qDebug() << "Category API error:" << message;
    });
}

void OrderForm::fetchDataDetailProduct(QString id) {
    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
    + QString("/product_cache_%1.json").arg(id);
    QFile cacheFile(cachePath);
    if (cacheFile.exists() && cacheFile.open(QIODevice::ReadOnly)) {
        QByteArray data = cacheFile.readAll();
        cacheFile.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray dataArray = doc.object()["data"].toArray();
        updateProductLeftTopPanel(dataArray);
        return;
    }

    const QUrl url(QString(settingConfig.getApiEndpoint("menu","product") + "/category/%1/price").arg(id));
    ApiClient::instance().get(url, [this, id](const QJsonObject &response) {
        QJsonArray dataArray = response["data"].toArray();

        QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                            + QString("/product_cache_%1.json").arg(id);
        QFile cacheFile(cachePath);
        if (cacheFile.open(QIODevice::WriteOnly)) {
            cacheFile.write(QJsonDocument(response).toJson());
            cacheFile.close();
        }

        updateProductLeftTopPanel(dataArray);
    }, [](const QString &message, int) {
        qDebug() << "Product API error:" << message;
    });
}


void OrderForm::onDataReceived(const QJsonObject &response) {
    QJsonArray dataArray = response["data"].toArray();

    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/category_cache.json";
    QFile cacheFile(cachePath);
    if (cacheFile.open(QIODevice::WriteOnly)) {
        cacheFile.write(QJsonDocument(response).toJson());
        cacheFile.close();
    }

    updateCategoryLeftPanel(dataArray);
}

void OrderForm::updateCategoryLeftPanel(const QJsonArray &dataArray) {
    while (QLayoutItem *child = buttonGridLayout->takeAt(0)) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    int row = 0, col = 0;
    const int maxColumns = 5;
    QString firstCatId;
    const int buttonWidth = ScreenUtils::px(140);
    const int buttonHeight = ScreenUtils::px(40);

    for (const QJsonValue &value : dataArray) {
        QJsonObject item = value.toObject();
        QString name = item["name"].toString();
        QString id = QString::number(item["id"].toInt());

        QPushButton *button = new QPushButton(name);
        button->setObjectName("primaryButton");
        button->setFixedSize(buttonWidth, buttonHeight);

        connect(button, &QPushButton::clicked, this, [this, id]() {
            this->fetchDataDetailProduct(id);
        });

        if (row == 0 && col == 0) {
            firstCatId = id;
        }

        buttonGridLayout->addWidget(button, row, col);

        if (++col >= maxColumns) {
            col = 0;
            row++;
        }
    }

    if (!firstCatId.isEmpty()) {
        this->fetchDataDetailProduct(firstCatId);
    }
}

void OrderForm::updateProductLeftTopPanel(const QJsonArray &dataArray) {
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    productWidgets.clear();
    products.clear();

    int row = 0, col = 0;
    int maxColumns = 3;

    for (const QJsonValue &value : dataArray) {
        QJsonObject item = value.toObject();
        QString name = item["name"].toString();
        int id = item["id"].toInt();
        int categoryId = item["categoryId"].toInt();
        QString imagePath = ":/assets/images/pizza.png";
        QJsonObject imageItem = item["productImage"].toObject();

        ProductImage productImage(
            imageItem["id"].toInt(),
            imageItem["fileName"].toString(),
            imageItem["imageBlob"].toString(),
            imageItem["mimeType"].toString()
            );

        Product product(id, name, categoryId, imagePath, getSkuFromItem(item), productImage);

        products.append(product);

        QWidget *productWidget = createProductGroupWidget(product);

        gridLayout->addWidget(productWidget, row, col);
        productWidgets.append(productWidget);

        col++;
        if (col >= maxColumns) {
            col = 0;
            row++;
        }
    }
}

void OrderForm::printReceipt() {

    QJsonObject jsonObject;

    jsonObject["id"] = order.id;
    jsonObject["customerId"] = order.customerId;
    jsonObject["customerName"] = customerNameText->text();
    jsonObject["remark"] = remarkText->toPlainText();
    jsonObject["subTotal"] = order.subTotal;
    jsonObject["subTotalTax"] = order.subTotalTax;
    jsonObject["subTotalDiscount"] = order.subTotalDiscount;
    jsonObject["grandTotal"] = order.grandTotal;

    QJsonArray orderItemsArray;

    for (const OrderItem &item : order.orderItems) {
        QJsonObject itemObj;
        itemObj["productName"] = item.productName;
        itemObj["categoryId"] = item.categoryId;

        QJsonArray skusArray;
        for (const OrderItemSku &sku : item.orderItemSkus) {
            QJsonObject skuObj;
            skuObj["quantity"] = sku.quantity;
            skuObj["skuName"] = sku.skuName;
            skuObj["subTotal"] = sku.subTotal;
            skuObj["price"] = sku.price;

            skusArray.append(skuObj);
        }

        itemObj["orderDetailSkus"] = skusArray;
        orderItemsArray.append(itemObj);
    }

    jsonObject["orderDetails"] = orderItemsArray;

    OrderPrint printer(jsonObject);
    printer.sendToKitchenPrinter();
}

QList<Sku> OrderForm::getSkuFromItem(const QJsonObject &object) {
    QList<Sku> skus;

    QJsonArray childItems = object["skus"].toArray();
    for (const QJsonValue &value : childItems) {
        QJsonObject item = value.toObject();
        QJsonObject itemPrice = item["tierPrice"].toObject();

        Sku sku(item["id"].toInt(), item["name"].toString(), itemPrice["price"].toDouble());
        skus.append(sku);
    }

    return skus;
}

QWidget* OrderForm::createProductGroupWidget(const Product &product) {
    QWidget *productGroupWidget = new QWidget();
    QHBoxLayout *productGroupLayout = new QHBoxLayout(productGroupWidget);

    productGroupLayout->setContentsMargins(0, 0, 0, 0);
    productGroupLayout->setSpacing(0);

    SkuWidget *skuWidget = new SkuWidget(product, this);
    skuWidget->setObjectName("skuCard");

    productGroupLayout->addWidget(new ProductWidget(product, productGroupWidget), 3);
    productGroupLayout->addWidget(skuWidget, 7);

    connect(skuWidget, &SkuWidget::updateQuantity,
            this, &OrderForm::updateQuantity);

    productGroupWidget->setLayout(productGroupLayout);

    return productGroupWidget;
}

void OrderForm::onConfirmButtonClicked() {
    QJsonArray orderItemsArray;
    double subTotal = 0;

    for (const OrderItem &orderItem : order.orderItems) {
        QJsonObject orderItemObj;
        orderItemObj["productId"] = orderItem.productId;
        orderItemObj["productName"] = orderItem.productName;
        orderItemObj["categoryId"] = orderItem.categoryId;

        QJsonArray skusArray;
        for (const OrderItemSku &sku : orderItem.orderItemSkus) {
            QJsonObject skuObj;
            skuObj["skuId"] = sku.skuId;
            skuObj["skuName"] = sku.skuName;
            skuObj["quantity"] = sku.quantity;
            skuObj["price"] = sku.price;
            skuObj["amount"] = sku.subTotal;
            subTotal += sku.subTotal;
            skusArray.append(skuObj);
        }

        orderItemObj["orderDetailSkus"] = skusArray;
        orderItemsArray.append(orderItemObj);
    }

    QJsonObject orderData;
    orderData["subTotal"] = subTotal;
    orderData["orderDetails"] = orderItemsArray;

    OrderPaymentPopup popup(orderData, this->tabWidget);
    popup.exec();
}

void OrderForm::populateOrderOnRightPanel() {
    cartLayout->setAlignment(Qt::AlignTop);

    for (auto it = cartWidgets.begin(); it != cartWidgets.end();) {
        bool exists = false;
        for (const OrderItem &orderItem : order.orderItems) {
            if (orderItem.productId == it.key()) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            cartLayout->removeWidget(it.value());
            it.value()->deleteLater();
            it = cartWidgets.erase(it);
        } else {
            ++it;
        }
    }

    double subTotal = 0;
    for (const OrderItem &orderItem : order.orderItems) {
        OrderCartWidget *cartWidget = cartWidgets.value(orderItem.productId, nullptr);
        if (!cartWidget) {
            cartWidget = new OrderCartWidget(orderItem, this, this);
            cartWidgets.insert(orderItem.productId, cartWidget);
            cartLayout->addWidget(cartWidget);
            connect(cartWidget, &OrderCartWidget::updateQuantity, this, &OrderForm::updateQuantity);
        } else {
            cartWidget->updateItem(orderItem);
        }
        subTotal += cartWidget->getTotal();
    }

    double discount = discountText->text().toDouble();
    totalText->setText(locale.toString(subTotal - discount, 'f', 0));

    updateTabSubtitle();
}

bool OrderForm::checkSku(const Sku &sku, const bool &add, OrderItem &orderItem) {
    for (auto it = orderItem.orderItemSkus.begin(); it != orderItem.orderItemSkus.end(); ++it) {
        if (it->skuId == sku.id) {
            if (add) {
                it->quantity++;
            } else {
                it->quantity--;
            }

            it->subTotal = it->quantity * it->price;

            if (it->quantity <= 0) {
                orderItem.orderItemSkus.erase(it);
            }

            return true;
        }
    }

    if (add) {
        orderItem.orderItemSkus.append(OrderItemSku(sku.id, sku.name, 1, sku.price, sku.price));
        return true;
    }

    return false;
}

void OrderForm::setTabButtonWidget(OrderTabButton *tabButton, const QString &orderTitle) {
    this->tabButton = tabButton;
    this->tabName = orderTitle;
    updateTabSubtitle();
}

void OrderForm::updateTabSubtitle() {
    if (!tabButton) {
        return;
    }

    int totalQuantity = 0;
    double subTotal = 0;
    for (const OrderItem &orderItem : order.orderItems) {
        for (const OrderItemSku &sku : orderItem.orderItemSkus) {
            totalQuantity += sku.quantity;
            subTotal += sku.subTotal;
        }
    }

    double discount = discountText->text().toDouble();
    double total = subTotal - discount;

    QString totalStr;
    if (total >= 1000) {
        totalStr = locale.toString(total / 1000.0, 'f', 1) + "K";
    } else {
        totalStr = locale.toString(total, 'f', 0);
    }

    tabButton->setSubtitle(QString("%1 items   Rp %2").arg(totalQuantity).arg(totalStr));
}

void OrderForm::updateCurrentTabName(const QString &newName) {
    if (!tabButton) {
        return;
    }

    if (newName.isEmpty()) {
        tabButton->setTitle(tabName);
    } else {
        tabButton->setTitle(tabName + " - " + newName);
    }
}

void OrderForm::updateOrderData(const Product &product, const Sku &sku, const bool &add) {
    bool found = false;

    for (int i = 0; i < order.orderItems.size(); ++i) {
        if (order.orderItems[i].productId == product.id) {
            checkSku(sku, add, order.orderItems[i]);

            if (order.orderItems[i].orderItemSkus.isEmpty()) {
                order.orderItems.removeAt(i);
            }

            found = true;
            break;
        }
    }

    if (!found) {
        QList<OrderItemSku> orderItemSkus = {
            OrderItemSku(sku.id, sku.name, 1, sku.price, sku.price)};
        OrderItem orderItem(product.id, product.name, product.categoryId, orderItemSkus);
        order.orderItems.append(orderItem);
    }

    populateOrderOnRightPanel();
}


void OrderForm::updateQuantity(const Product &product, const Sku &sku, bool add) {
    updateOrderData(product, sku, add);
}


void OrderForm::filterProducts(const QString &query) {
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    int row = 0, col = 0;
    int maxColumns = 4;

    for (const Product &product : products) {
        if (product.name.contains(query, Qt::CaseInsensitive)) {
            QWidget *productWidget = createProductGroupWidget(product);
            gridLayout->addWidget(productWidget, row, col);

            col++;
            if (col >= maxColumns) {
                col = 0;
                row++;
            }
        }
    }
}

OrderForm::~OrderForm() {
    delete ui;
}
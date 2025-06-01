#include "orderform.h"
#include "ui_orderform.h"
#include <QLabel>
#include <QTextEdit>
#include <QTableWidgetItem>
#include <QScrollArea>
#include <QPushButton>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QTextBrowser>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <product.h>
#include <cartitemwidget.h>
#include <ordercartwidget.h>
#include "orderitem.h"
#include "orderitemsku.h"
#include "productimage.h"
#include "productwidget.h"
#include "skuwidget.h"
#include <QLocale>
#include <QMessageBox>
#include <toast.h>
#include <orderpaymentpopup.h>
#include <QProcess>
#include <orderprint.h>

OrderForm::OrderForm(QTabWidget *tabWidget, QWidget *parent)
    : QWidget(parent), ui(new Ui::OrderForm), tabWidget(tabWidget), networkManager(new QNetworkAccessManager(this)), order()
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
    searchBox->setPlaceholderText("🔍 Search products...");
    searchBox->setFixedHeight(35);
    searchBox->setStyleSheet("padding: 5px; font-size: 14px; border-radius: 5px;");

    connect(searchBox, &QLineEdit::textChanged, this, &OrderForm::filterProducts);

    QPushButton *searchButton = new QPushButton("Search", this);
    searchButton->setFixedSize(90, 35);
    searchButton->setStyleSheet("background-color: #007bff; color: white; font-size: 14px; border-radius: 5px;");

    searchButton->setFixedSize(80, 40);

    searchLayout->addWidget(searchBox);
    searchLayout->addWidget(searchButton);
    searchWidget->setLayout(searchLayout);

    topLeftLayout->addWidget(searchWidget);  // Add search bar to the top

    QWidget *topLeftWidget = new QWidget();
    gridLayout = new QGridLayout(topLeftWidget);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(0);

    topLeftWidget->setLayout(gridLayout);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(topLeftWidget);

    topLeftLayout->addWidget(scrollArea);  // Add scrollable product grid below the search box
    topLeftContainer->setLayout(topLeftLayout);

    bottomLeftWidget = new QWidget();
    buttonGridLayout = new QGridLayout(bottomLeftWidget);
    QScrollArea *bottomScrollArea = new QScrollArea();
    bottomScrollArea->setWidgetResizable(true);
    bottomScrollArea->setWidget(bottomLeftWidget);

    leftSplitter->addWidget(topLeftContainer);
    leftSplitter->addWidget(bottomScrollArea);

    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);

    QLabel *summaryLabel = new QLabel("📝 Order Summary", this);
    summaryLabel->setStyleSheet("font-weight: bold; font-size: 16px;");

    QFont boldFont;
    boldFont.setBold(true);
    boldFont.setPointSize(14);
    summaryLabel->setFont(boldFont);
    rightLayout->addWidget(summaryLabel);

    QWidget *cartPanel = new QWidget();
    cartLayout = new QVBoxLayout(cartPanel);

    // Ensure cartPanel has a layout to hold widgets
    cartPanel->setLayout(cartLayout);

    QScrollArea *rightScrollArea = new QScrollArea();
    rightScrollArea->setWidgetResizable(true);
    rightScrollArea->setWidget(cartPanel);

    cartLayout->setContentsMargins(0, 0, 0, 0);
    cartLayout->setSpacing(0);

    rightLayout->addWidget(rightScrollArea, 9);

    QWidget *summaryPanel = new QWidget();
    QGridLayout *summaryGridLayout = new QGridLayout(summaryPanel);
    summaryGridLayout->setContentsMargins(4, 2, 4, 2);
    summaryGridLayout->setSpacing(2);

    summaryGridLayout->addWidget(new QLabel("Customer Name"), 0, 0);
    customerNameText = new QLineEdit();
    customerNameText->setPlaceholderText("Customer Name");
    customerNameText->setFixedHeight(30);

    connect(customerNameText, &QLineEdit::textChanged, this, &OrderForm::updateCurrentTabName);

    summaryGridLayout->addWidget(customerNameText, 0, 1);

    summaryGridLayout->addWidget(new QLabel("Sub Total"), 1, 0);
    subTotalText = new QLineEdit();
    subTotalText->setPlaceholderText("Sub Total");
    subTotalText->setAlignment(Qt::AlignRight);
    subTotalText->setFixedHeight(30);
    summaryGridLayout->addWidget(subTotalText, 1, 1);

    summaryGridLayout->addWidget(new QLabel("Discount"), 2, 0);
    discountText = new QLineEdit();
    discountText->setPlaceholderText("Discount");
    discountText->setAlignment(Qt::AlignRight);
    discountText->setFixedHeight(30);
    summaryGridLayout->addWidget(discountText, 2, 1);

    summaryGridLayout->addWidget(new QLabel("Total"), 3, 0);
    totalText = new QLineEdit();
    totalText->setPlaceholderText("Total");
    totalText->setAlignment(Qt::AlignRight);
    totalText->setFixedHeight(30);
    summaryGridLayout->addWidget(totalText, 3, 1);
    summaryPanel->setLayout(summaryGridLayout);

    rightLayout->addWidget(summaryPanel);

    remarkText = new QTextEdit();
    remarkText->setPlaceholderText("Additional notes...");
    remarkText->setFixedHeight(80);
    rightLayout->addWidget(remarkText, 1);

    // Add horizontal layout with two buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *confirmButton = new QPushButton("✔ Confirm", this);
    QPushButton *printButton = new QPushButton("🖨 Print", this);
    ;

    // Adjust button sizes
    confirmButton->setFixedSize(100, 40);
    printButton->setFixedSize(100, 40);

    confirmButton->setStyleSheet("background-color: #28a745; color: white; font-weight: bold;");
    printButton->setStyleSheet("background-color: #007bff; color: white; font-weight: bold;");

    // Add buttons to the horizontal layout
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(printButton);
    buttonLayout->setAlignment(Qt::AlignCenter); // Center the buttons

    // Create a container widget for the button layout
    QWidget *buttonContainer = new QWidget();
    buttonContainer->setLayout(buttonLayout);

    // Add the button container to the right layout
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

    connect(this, &OrderForm::updateQuantity, this, &OrderForm::updateOrderData);
    connect(confirmButton, &QPushButton::clicked, this, &OrderForm::onConfirmButtonClicked);
    connect(discountText, &QLineEdit::textChanged, this, &OrderForm::populateOrderOnRightPanel);
    connect(printButton, &QPushButton::clicked, this, &OrderForm::printReceipt);
}

void OrderForm::fetchDataFromAPI() {

    QNetworkRequest requestCategory(QUrl(settingConfig.getApiEndpoint("menu", "category") + "/tier"));
    requestCategory.setRawHeader("Authorization", "Bearer " + settingConfig.getValue("authToken").toString().toUtf8());
    requestCategory.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* replyCategory = networkManager->get(requestCategory);

    connect(replyCategory, &QNetworkReply::finished, this, [this, replyCategory]() {
        this->onDataReceived(replyCategory);
    });
}

void OrderForm::fetchDataDetailProduct(QString id) {
    QNetworkRequest requestDetailProduct(QUrl(QString(settingConfig.getApiEndpoint("menu","product") + "/category/%1/price").arg(id)));
    requestDetailProduct.setRawHeader("Authorization", "Bearer " + settingConfig.getValue("authToken").toString().toUtf8());
    requestDetailProduct.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* replyDetailProduct = networkManager->get(requestDetailProduct);

    connect(replyDetailProduct, &QNetworkReply::finished, this, [this, replyDetailProduct]() {
        this->onDataDetailProductReceived(replyDetailProduct);
    });
}

void OrderForm::onDataReceived(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "API Error: " << reply->errorString();
        return;
    }

    QByteArray responseData = reply->readAll();
    if (responseData.isEmpty()) {
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray dataArray = jsonObj["data"].toArray();

    updateCategoryLeftPanel(dataArray);
    reply->deleteLater();
}

void OrderForm::updateCategoryLeftPanel(const QJsonArray &dataArray) {
    // 🔄 Clear existing category buttons efficiently
    while (QLayoutItem *child = buttonGridLayout->takeAt(0)) {
        if (child->widget()) {
            delete child->widget();  // ✅ Delete the widget safely
        }
        delete child; // ❌ Avoid double deletion issue
    }

    int row = 0, col = 0;
    const int maxColumns = 5;  // 🔢 Define grid layout columns
    QString firstCatId;
    const int buttonWidth = 140;
    const int buttonHeight = 75;

    for (const QJsonValue &value : dataArray) {
        QJsonObject item = value.toObject();
        QString name = item["name"].toString();
        QString id = QString::number(item["id"].toInt());

        // 🆕 Create a new category button
        QPushButton *button = new QPushButton(name);
        button->setFixedSize(buttonWidth, buttonHeight);
        button->setStyleSheet("background-color: #007bff; color: white; font-size: 14px; font-weight: bold; border-radius: 5px;");

        connect(button, &QPushButton::clicked, this, [this, id]() {
            this->fetchDataDetailProduct(id);
        });

        // 📌 Select the first category by default
        if (row == 0 && col == 0) {
            firstCatId = id;
        }

        buttonGridLayout->addWidget(button, row, col);

        // 🔄 Manage Grid Layout Alignment
        if (++col >= maxColumns) {
            col = 0;
            row++;
        }

        // 🏃 Keep UI Responsive for Large Data
        if (row % 5 == 0) {
            QApplication::processEvents();
        }
    }

    // 📥 Load first category automatically if available
    if (!firstCatId.isEmpty()) {
        this->fetchDataDetailProduct(firstCatId);
    }
}

void OrderForm::onDataDetailProductReceived(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "API Error: " << reply->errorString();
        return;
    }

    QByteArray responseData = reply->readAll();
    if (responseData.isEmpty()) {
        return;
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray dataArray = jsonObj["data"].toArray();

    this->updateProductLeftTopPanel(dataArray);

    reply->deleteLater();
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

    // Add basic order fields
    jsonObject["id"] = order.id;
    jsonObject["customerId"] = order.customerId;
    jsonObject["customerName"] = customerNameText->text();
    jsonObject["remark"] = remarkText->toPlainText();
    jsonObject["subTotal"] = order.subTotal;
    jsonObject["subTotalTax"] = order.subTotalTax;
    jsonObject["subTotalDiscount"] = order.subTotalDiscount;
    jsonObject["grandTotal"] = order.grandTotal;

    // Prepare orderItems array
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

            // Add more fields from SKU if needed
            skusArray.append(skuObj);
        }

        itemObj["orderDetailSkus"] = skusArray;
        orderItemsArray.append(itemObj);
    }

    // Add orderItems array to jsonObject
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

    productGroupLayout->addWidget(new ProductWidget(product, productGroupWidget), 3);
    productGroupLayout->addWidget(new SkuWidget(product, this), 7);
    productGroupWidget->setLayout(productGroupLayout);

    return productGroupWidget;
}

void OrderForm::onConfirmButtonClicked() {
    QJsonArray orderItemsArray;
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
            skusArray.append(skuObj);
        }

        orderItemObj["orderDetailSkus"] = skusArray;
        orderItemsArray.append(orderItemObj);
    }

    QJsonObject orderData;

    orderData["storeServiceTypesId"] = 1;
    orderData["customerName"] = customerNameText->text();
    orderData["customerId"] = -1;
    orderData["remark"] = remarkText->toPlainText();
    orderData["orderDetails"] = orderItemsArray;
    orderData["discount"] = discountText->text();

    QJsonDocument jsonDoc(orderData);
    QByteArray jsonData = jsonDoc.toJson();

    QNetworkRequest request(QUrl(settingConfig.getApiEndpoint("order","confirm")));
    request.setRawHeader("Authorization", "Bearer " + settingConfig.getValue("authToken").toString().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {

            QByteArray responseData = reply->readAll();
            if (responseData.isEmpty()) {
                return;
            }

            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonDoc.object();

            OrderPaymentPopup popup(jsonObj["data"].toObject(), this->tabWidget);
            popup.exec();

        } else {
            qDebug() << "Order placement failed: " << reply->errorString();
        }
        reply->deleteLater();
    });
}

void OrderForm::populateOrderOnRightPanel() {
    QWidget *cartItemWidget = new QWidget();

    cartLayout->setAlignment(Qt::AlignTop);

    QLayoutItem *child;
    while ((child = cartLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    double subTotal = 0;
    for (const OrderItem &orderItem: order.orderItems) {
        OrderCartWidget *orderCartItem = new OrderCartWidget(orderItem, this, cartItemWidget);
        if (orderCartItem->getTotal()) {
            subTotal += (orderCartItem->getTotal());
        }
        cartLayout->addWidget(orderCartItem);
    }

    subTotalText->setText(locale.toString(subTotal, 'f', 0));

    double discount = discountText->text().toDouble();
    totalText->setText(locale.toString(subTotal - discount, 'f', 0));

}

bool OrderForm::checkSku(const Sku &sku, const bool &add, OrderItem &orderItem) {

    for (OrderItemSku &orderItemSku : orderItem.orderItemSkus) {
        if (sku.id == orderItemSku.skuId) {
            if (add) {
                orderItemSku.quantity++;
            } else {
                orderItemSku.quantity--;
            }

            double total = orderItemSku.quantity*orderItemSku.price;
            orderItemSku.subTotal = total;

            if (orderItemSku.quantity==0) {
                removeSku(orderItem.productId, sku);
            }

            return true;
        }
    }
    return false;
}

void OrderForm::updateCurrentTabName(const QString &newName) {
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex != -1) {  // Ensure a valid tab is selected
        tabWidget->setTabText(currentIndex, "New Order - " + newName);
    }
}

void OrderForm::removeSku(const int &productId, const Sku &sku) {
    bool isRemoveItem = false; // Initialize the flag

    for (OrderItem &orderItem : order.orderItems) {
        if (orderItem.productId == productId) {
            if (orderItem.orderItemSkus.size() > 1) {
                // Remove the SKU from the order item
                orderItem.orderItemSkus.erase(
                    std::remove_if(orderItem.orderItemSkus.begin(), orderItem.orderItemSkus.end(),
                                   [&sku](const OrderItemSku &dsku) {
                                       return dsku.skuId == sku.id;
                                   }),
                    orderItem.orderItemSkus.end());
            } else {
                isRemoveItem = true; // Mark for deletion
            }
        }
    }

    if (isRemoveItem) {
        order.orderItems.erase(std::remove_if(order.orderItems.begin(), order.orderItems.end(),
                                              [productId](const OrderItem &item) { return item.productId == productId; }),
                               order.orderItems.end());
    }

    populateOrderOnRightPanel();
}


void OrderForm::updateOrderData(const Product &product, const Sku &sku,
                                const bool &add) {
    QList<OrderItemSku> orderItemSkus;
    bool found;

    for (OrderItem &orderItem : order.orderItems) {
        if (orderItem.productId == product.id) {
            if (!checkSku(sku, add, orderItem)) {
                OrderItemSku orderItemSku(sku.id, sku.name, 1, sku.price, sku.price*1);
                orderItem.orderItemSkus.append(orderItemSku);
            }
            found = true;
            break;
        }
    }

    if (order.orderItems.isEmpty() || !found) {
        OrderItemSku orderItemSku(sku.id, sku.name, 1, sku.price, sku.price*1);
        orderItemSkus.append(orderItemSku);
        OrderItem orderItem(product.id, product.name, product.categoryId, orderItemSkus);
        order.orderItems.append(orderItem);
    }

    populateOrderOnRightPanel();
}

void OrderForm::filterProducts(const QString &query) {
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    int row = 0, col = 0;
    int maxColumns = 3;

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

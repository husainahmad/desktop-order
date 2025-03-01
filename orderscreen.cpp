#include "orderscreen.h"
#include "orderform.h"
#include "ui_orderscreen.h"
#include "ordersummary.h"
#include "ordertablewidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QCloseEvent>

OrderScreen::OrderScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderScreen), networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    locale = QLocale::English;

    setWindowTitle("Daily Order");
    resize(600, 400);

    // Create main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Create a header section
    QWidget *headerWidget = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(10, 10, 10, 10);

    headerLayout->addStretch();  // Pushes everything to the left
    headerWidget->setLayout(headerLayout);

    // Add header to the main layout
    mainLayout->addWidget(headerWidget);

    // Create QTabWidget
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabBar::tab {"
        "   height: 25px;"
        "   width: 120px;"
        "   font-size: 16px;"
        "   padding: 10px;"
        "   background-color: #007bff;"
        "   color: white;"
        "   border-radius: 5px;"
        "}"
        "QTabBar::tab:selected {"
        "   background-color: #0056b3;"
        "   font-weight: bold;"
        "}"
        "QTabBar::tab:hover {"
        "   background-color: #3399ff;"
        "}"
        );

    mainLayout->addWidget(tabWidget);

    firstTab = new QWidget();
    QVBoxLayout *firstTabLayout = new QVBoxLayout(firstTab);

    ordersWidget = new QWidget(this);
    ordersLayout = new QVBoxLayout(ordersWidget);
    ordersWidget->setLayout(ordersLayout);

    firstTabLayout->addWidget(ordersWidget, 7);

    summaryWidget = new QWidget(this);
    summaryLayout = new QVBoxLayout(summaryWidget);
    summaryWidget->setLayout(summaryLayout);

    firstTabLayout->addWidget(summaryWidget, 3);
    firstTab->setLayout(firstTabLayout);

    tabWidget->addTab(firstTab, "Orders");

    // Buttons for adding and removing tabs
    QPushButton *addTabButton = new QPushButton("➕ Add Order", this);

    // Set fixed height for better appearance
    addTabButton->setFixedHeight(40);

    // Style the buttons with Qt StyleSheet
    QString buttonStyle = "QPushButton {"
                          "background-color: #4CAF50; color: white;"
                          "border-radius: 8px; padding: 10px; font-size: 14px;"
                          "border: 1px solid #388E3C;"
                          "}"
                          "QPushButton:hover {"
                          "background-color: #45a049;"
                          "}";

    addTabButton->setStyleSheet(buttonStyle);

    QWidget *tabButtonWidget = new QWidget();
    QHBoxLayout *tabButtonLayout = new QHBoxLayout(tabButtonWidget);
    tabButtonLayout->addWidget(addTabButton);

    tabButtonWidget->setLayout(tabButtonLayout);

    mainLayout->addWidget(tabButtonWidget);

    // Connect button signals to slots
    connect(addTabButton, &QPushButton::clicked, this, &OrderScreen::addNewTabWithOrderForm);
    connect(tabWidget, &QTabWidget::currentChanged, this, &OrderScreen::onTabChanged);  // 👈 Connect tab change event

    setLayout(mainLayout);

    fetchDataFromAPI();
}

void OrderScreen::fetchDataFromAPI() {
    QNetworkRequest requestOrder(QUrl(QString("http://localhost:8088/api/v1/order")));
    requestOrder.setRawHeader("Authorization", "Bearer " + settings.value("authToken").toString().toUtf8());
    requestOrder.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* replyOrder = networkManager->get(requestOrder);

    connect(replyOrder, &QNetworkReply::finished, this, [this, replyOrder]() {
        if (replyOrder->error() == QNetworkReply::NoError) {
            QByteArray responseData = replyOrder->readAll();
            parseJsonResponse(responseData);
        } else {
            qDebug() << "API Request Failed: " << replyOrder->errorString();
        }
        replyOrder->deleteLater();
    });
}

void OrderScreen::onDateChanged(const QDate &selectedDate) {
    QString selectedDateStr = selectedDate.toString("yyyy-MM-dd");
    qDebug() << "Date Selected:" << selectedDateStr;

    // TODO: Update table contents based on the selected date
    // fetchDataFromAPI(selectedDateStr);  // Uncomment if API filtering is implemented
}

void OrderScreen::parseJsonResponse(const QByteArray &responseData) {
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (!jsonDoc.isObject()) {
        qDebug() << "Invalid JSON Response!";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray dataArray = jsonObj["data"].toArray();

    OrderTableWidget *orderWidget = new OrderTableWidget(dataArray, this);

    QLayoutItem *childOrder;
    while ((childOrder = ordersLayout->takeAt(0)) != nullptr) {
        delete childOrder->widget();
        delete childOrder;
    }

    ordersLayout->addWidget(orderWidget);

    // // Create OrderSummary widget
    OrderSummary *summaryWidget = new OrderSummary(dataArray, this);

    QLayoutItem *child;
    while ((child = summaryLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    summaryLayout->addWidget(summaryWidget);

}

void OrderScreen::addNewTabWithOrderForm() {
    OrderForm *newTab = new OrderForm(tabWidget);
    QString tabName = QString("New Order");
    int newTabIndex = tabWidget->addTab(newTab, tabName);

    // Get the tab bar
    QTabBar *tabBar = tabWidget->tabBar();

    // Create a close button
    QPushButton *closeButton = new QPushButton("❌");
    closeButton->setFixedSize(20, 20);
    closeButton->setStyleSheet("QPushButton { border: none; color: red; font-size: 12px; }"
                               "QPushButton:hover { color: darkred; }");

    // Remove the tab when close button is clicked
    connect(closeButton, &QPushButton::clicked, [this, newTabIndex]() {
        tabWidget->removeTab(newTabIndex);
    });

    tabBar->setTabButton(newTabIndex, QTabBar::RightSide, closeButton);

    tabWidget->setCurrentIndex(newTabIndex);
}

void OrderScreen::onTabChanged(int index) {
    if (index == 0) {  // If the first tab (Orders) is selected
        qDebug() << "Orders tab selected. Refreshing data...";
        fetchDataFromAPI();
    }
}

void OrderScreen::addProductToList() {
    QString product = productNameInput->text() + " (Qty: " + quantityInput->text() + ")";
    productListWidget->addItem(product);
}

void OrderScreen::closeEvent(QCloseEvent *event) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Exit Confirmation");
    msgBox.setText("Do you really want to close?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Warning);

    if (msgBox.exec() == QMessageBox::Yes) {
        event->accept();
    } else {
        event->ignore();
    }
}
OrderScreen::~OrderScreen()
{
    delete ui;
}

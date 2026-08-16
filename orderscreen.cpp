#include "orderscreen.h"
#include "orderform.h"
#include "ordertabbutton.h"
#include "ui_orderscreen.h"
#include "ordersummary.h"
#include "ordertablewidget.h"
#include "orderprint.h"
#include "dailyreportscreen.h"
#include "salesreportscreen.h"
#include "cacheutils.h"
#include "tokenmanager.h"
#include "screenutils.h"
#include "apiclient.h"
#include "settingscreen.h"
#include "loginscreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QDateTime>
#include <QToolBar>
#include <QAction>
#include <functional>
#include <QTableWidget>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QCloseEvent>
#include <QUrlQuery>

OrderScreen::OrderScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OrderScreen)
{
    ui->setupUi(this);

    locale = QLocale::English;

    setWindowTitle("Daily Order");
    resize(ScreenUtils::fittedSize(1280, 800, 1.0, 1.0));
    setMinimumSize(1024, 640);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // ======================= Header bar =======================
    QWidget *headerWidget = new QWidget(this);
    headerWidget->setObjectName("headerBar");
    headerWidget->setFixedHeight(ScreenUtils::px(76));
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(24, 12, 24, 12);
    headerLayout->setSpacing(14);

    QLabel *headerLogo = new QLabel(headerWidget);
    QPixmap headerLogoPixmap(":/assets/images/pizza.png");
    headerLogo->setPixmap(headerLogoPixmap.scaled(ScreenUtils::px(44), ScreenUtils::px(44), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    headerLogo->setFixedSize(ScreenUtils::px(44), ScreenUtils::px(44));

    QWidget *brandBox = new QWidget(headerWidget);
    QVBoxLayout *brandBoxLayout = new QVBoxLayout(brandBox);
    brandBoxLayout->setContentsMargins(0, 0, 0, 0);
    brandBoxLayout->setSpacing(2);

    QString brandName = settingConfig.getValue("userDetail.brand.name", "Pizza").toString();
    QString storeName = settingConfig.getValue("userDetail.store.name", "My Store").toString();
    QLabel *titleLabel = new QLabel(brandName + " Order", brandBox);
    titleLabel->setObjectName("headerTitle");
    QLabel *subLabel = new QLabel(storeName, brandBox);
    subLabel->setObjectName("headerSub");
    brandBoxLayout->addWidget(titleLabel);
    brandBoxLayout->addWidget(subLabel);

    headerLayout->addWidget(headerLogo);
    headerLayout->addWidget(brandBox);
    headerLayout->addStretch();

    QLabel *dateTimeLabel = new QLabel(
        QDateTime::currentDateTime().toString("dddd, dd MMMM yyyy  |  HH:mm"), headerWidget);
    dateTimeLabel->setObjectName("headerMeta");
    headerLayout->addWidget(dateTimeLabel);

    // ======================= Header toolbar =======================
    QToolBar *headerToolBar = new QToolBar(headerWidget);
    headerToolBar->setMovable(false);
    headerToolBar->setFloatable(false);
    headerToolBar->setIconSize(QSize(ScreenUtils::px(16), ScreenUtils::px(16)));
    headerToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto addHeaderAction = [this, headerToolBar](const QString &text, QStyle::StandardPixmap icon, const std::function<void()> &slot) {
        QAction *action = headerToolBar->addAction(text);
        action->setIcon(style()->standardIcon(icon));
        connect(action, &QAction::triggered, this, slot);
    };

    addHeaderAction("Refresh", QStyle::SP_BrowserReload, [this]() {
        CacheUtils::clearAppCache();
        fetchDataFromAPI();
    });
    addHeaderAction("Daily Report", QStyle::SP_FileDialogDetailedView, [this]() { onDailyReportClicked(); });
    addHeaderAction("Sales Report", QStyle::SP_FileDialogContentsView, [this]() { onSalesReportClicked(); });
    addHeaderAction("Settlement", QStyle::SP_FileDialogInfoView, [this]() { onSettlementClicked(); });

    headerToolBar->addSeparator();

    addHeaderAction("Settings", QStyle::SP_FileDialogListView, [this]() { onSettingsClicked(); });

    headerLayout->addWidget(headerToolBar);

    logoutButton = new QPushButton("Logout", headerWidget);
    logoutButton->setObjectName("navButton");
    logoutButton->setFixedHeight(ScreenUtils::px(38));
    headerLayout->addWidget(logoutButton);

    mainLayout->addWidget(headerWidget);

    // ======================= Tabs =======================
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget, 1);

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

    // Trailing "Add Order" tab-style button (always the last tab)
    OrderTabButton *addOrderButton = new OrderTabButton();
    addOrderButton->setTitle("+ Add Order");
    addOrderButton->setSubtitle("New order");
    addOrderButton->setSelectable(false);
    addOrderButton->closeButton->hide();
    tabWidget->addTab(new QWidget(), "");
    const int addOrderTabIndex = tabWidget->count() - 1;
    tabWidget->tabBar()->setTabButton(addOrderTabIndex, QTabBar::LeftSide, addOrderButton);
    connect(addOrderButton, &OrderTabButton::clicked, this, &OrderScreen::onOrderClicked);
    connect(tabWidget->tabBar(), &QTabBar::tabBarClicked, this, [this](int index) {
        if (index == tabWidget->count() - 1) {
            onOrderClicked();
        }
    });

    connect(logoutButton, &QPushButton::clicked, this, &OrderScreen::onLogoutClicked);

    connect(tabWidget, &QTabWidget::currentChanged, this, &OrderScreen::onTabChanged);

    setLayout(mainLayout);

    fetchDataFromAPI();
}

void OrderScreen::fetchDataFromAPI() {
    const QUrl url(settingConfig.getApiEndpoint("order","daily"));
    ApiClient::instance().get(url, [this](const QJsonObject &response) {
        parseJsonResponse(QJsonDocument(response).toJson(QJsonDocument::Compact));
    }, [](const QString &message, int) {
        qDebug() << "Order API request failed:" << message;
    });
}

void OrderScreen::parseJsonResponse(const QByteArray &responseData) {
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (!jsonDoc.isObject()) {
        qDebug() << "Invalid JSON Response!";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray dataArray = jsonObj["data"].toArray();

    OrderTableWidget *orderWidget = new OrderTableWidget(dataArray, tabWidget, this);

    QLayoutItem *childOrder;
    while ((childOrder = ordersLayout->takeAt(0)) != nullptr) {
        delete childOrder->widget();
        delete childOrder;
    }

    ordersLayout->addWidget(orderWidget);

    OrderSummary *summaryWidget = new OrderSummary(dataArray, this);

    QLayoutItem *child;
    while ((child = summaryLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    summaryLayout->addWidget(summaryWidget);
}

void OrderScreen::onSettlementClicked() {
    QDate currentDate = QDate::currentDate();
    QString startDateTime = currentDate.toString("yyyy-MM-dd") + "T00:00:00";
    QString endDateTime   = currentDate.toString("yyyy-MM-dd") + "T23:59:59";

    QUrl url(settingConfig.getApiEndpoint("reports", "settlement"));
    QUrlQuery query;
    query.addQueryItem("start", startDateTime);
    query.addQueryItem("end", endDateTime);
    url.setQuery(query);

    ApiClient::instance().get(url, [](const QJsonObject &response) {
        QJsonObject settlementData = response["data"].toObject();
        OrderPrint printer(settlementData);
        printer.sendSettlementToReceiptPrinter();
    }, [this](const QString &message, int) {
        qDebug() << "Settlement API error:" << message;
        QMessageBox::warning(this, "Settlement Error", "Failed to fetch settlement report.");
    });
}


void OrderScreen::onOrderClicked() {
    orderNumberCounter++;
    QString orderTitle = QString("Order #%1").arg(orderNumberCounter, 3, 10, QLatin1Char('0'));

    OrderForm *newTab = new OrderForm(tabWidget);
    int newTabIndex = tabWidget->insertTab(tabWidget->count() - 1, newTab, "");

    OrderTabButton *tabButton = new OrderTabButton();
    tabButton->setTitle(orderTitle);
    tabButton->setSubtitle("0 items   Rp 0");
    tabWidget->tabBar()->setTabButton(newTabIndex, QTabBar::LeftSide, tabButton);

    connect(tabButton->closeButton, &QPushButton::clicked, this, [this, tabButton, orderTitle, newTab]() {
        int index = -1;
        for (int i = 0; i < tabWidget->count(); ++i) {
            if (tabWidget->tabBar()->tabButton(i, QTabBar::LeftSide) == tabButton) {
                index = i;
                break;
            }
        }
        if (index == -1) {
            return;
        }

        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Close Order");
        msgBox.setText(QString("Do you really want to close %1?").arg(orderTitle));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if (msgBox.exec() == QMessageBox::Yes) {
            tabWidget->removeTab(index);
            newTab->deleteLater();
        }
    });

    newTab->setTabButtonWidget(tabButton, orderTitle);

    tabWidget->setCurrentIndex(newTabIndex);
}

void OrderScreen::onTabChanged(int index) {
    if (index == 0) {
        fetchDataFromAPI();
    }
}

void OrderScreen::closeEvent(QCloseEvent *event) {
    if (exitConfirmed) {
        event->accept();
        return;
    }

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

void OrderScreen::onDailyReportClicked() {
    DailyReportScreen *dailyReportScreen = new DailyReportScreen();
    dailyReportScreen->setAttribute(Qt::WA_DeleteOnClose);
    dailyReportScreen->show();
}

void OrderScreen::onSalesReportClicked() {
    SalesReportScreen *salesReportScreen = new SalesReportScreen();
    salesReportScreen->setAttribute(Qt::WA_DeleteOnClose);
    salesReportScreen->show();
}

void OrderScreen::onSettingsClicked() {
    SettingsScreen *settings = new SettingsScreen();
    settings->setAttribute(Qt::WA_DeleteOnClose);

    connect(settings, &SettingsScreen::backRequested, this, [this]() {
        this->showFullScreen();
    });

    connect(settings, &SettingsScreen::loggedOut, this, [this]() {
        exitConfirmed = true;
        this->close();
        LoginScreen *loginScreen = new LoginScreen();
        loginScreen->show();
    });

    this->hide();
    settings->showFullScreen();
}

void OrderScreen::onLogoutClicked() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Logout");
    msgBox.setText("Are you sure you want to logout?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes) {
        TokenManager::instance().clearTokens();
        CacheUtils::clearAppCache();

        exitConfirmed = true;
        this->close();
        LoginScreen *loginScreen = new LoginScreen();
        loginScreen->show();
    }
}

OrderScreen::~OrderScreen()
{
    delete ui;
}
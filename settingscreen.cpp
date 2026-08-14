#include "settingscreen.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFormLayout>
#include <QIntValidator>
#include <QPixmap>
#include <QUrl>
#include <QMessageBox>
#include <QApplication>
#include <QTcpSocket>
#include <QProcess>

#include "screenutils.h"
#include "tokenmanager.h"
#include "cacheutils.h"
#include "orderprint.h"
#include "loginscreen.h"

SettingsScreen::SettingsScreen(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Settings");
    resize(ScreenUtils::fittedSize(ScreenUtils::px(900), ScreenUtils::px(760), 0.9, 0.95));
    setMinimumSize(ScreenUtils::px(720), ScreenUtils::px(560));

    buildUi();
    loadConfig();
}

SettingsScreen::~SettingsScreen() {}

void SettingsScreen::buildUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // ======================= Header bar =======================
    QWidget *header = new QWidget(this);
    header->setObjectName("headerBar");
    header->setFixedHeight(ScreenUtils::px(64));
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 10, 16, 10);
    headerLayout->setSpacing(12);

    QLabel *headerLogo = new QLabel(header);
    QPixmap headerLogoPixmap(":/assets/images/pizza.png");
    headerLogo->setPixmap(headerLogoPixmap.scaled(ScreenUtils::px(36), ScreenUtils::px(36), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    headerLogo->setFixedSize(ScreenUtils::px(36), ScreenUtils::px(36));

    QLabel *titleLabel = new QLabel("Settings", header);
    titleLabel->setObjectName("headerTitle");

    headerLayout->addWidget(headerLogo);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    backButton = new QPushButton("← Back", header);
    backButton->setObjectName("navButton");
    backButton->setFixedHeight(ScreenUtils::px(34));
    headerLayout->addWidget(backButton);

    mainLayout->addWidget(header);

    // ======================= Scrollable body =======================
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    QWidget *body = new QWidget(scrollArea);
    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(8, 8, 8, 8);
    bodyLayout->setSpacing(16);

    // User info
    QWidget *userCard = new QWidget(body);
    userCard->setObjectName("cardPanel");
    QVBoxLayout *userLayout = new QVBoxLayout(userCard);
    userLayout->setContentsMargins(20, 16, 20, 16);
    userLayout->setSpacing(4);

    usernameValue = new QLabel(userCard);
    usernameValue->setStyleSheet(ScreenUtils::qss("font-size: 18px; font-weight: 700; color: #0f172a;"));
    storeValue = new QLabel(userCard);
    storeValue->setStyleSheet(ScreenUtils::qss("font-size: 14px; color: #64748b;"));
    brandValue = new QLabel(userCard);
    brandValue->setStyleSheet(ScreenUtils::qss("font-size: 14px; color: #64748b;"));

    userLayout->addWidget(usernameValue);
    userLayout->addWidget(storeValue);
    userLayout->addWidget(brandValue);
    bodyLayout->addWidget(userCard);

    // ======================= Server card =======================
    QWidget *serverCard = new QWidget(body);
    serverCard->setObjectName("cardPanel");
    QVBoxLayout *serverLayout = new QVBoxLayout(serverCard);
    serverLayout->setContentsMargins(20, 16, 20, 16);
    serverLayout->setSpacing(12);

    QLabel *serverTitle = new QLabel("Server", serverCard);
    serverTitle->setObjectName("sectionHeaderAccent");

    hostInput = new QLineEdit(serverCard);
    hostInput->setFixedHeight(ScreenUtils::px(40));
    hostInput->setPlaceholderText("Host (e.g. 103.150.197.7)");

    portInput = new QLineEdit(serverCard);
    portInput->setFixedHeight(ScreenUtils::px(40));
    portInput->setPlaceholderText("Port (default 8080)");
    portInput->setValidator(new QIntValidator(1, 65535, this));

    QPushButton *serverSaveButton = new QPushButton("Save Server", serverCard);
    serverSaveButton->setObjectName("primaryButton");
    serverSaveButton->setFixedHeight(ScreenUtils::px(42));

    serverStatusLabel = new QLabel(serverCard);
    serverStatusLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #64748b;"));
    serverStatusLabel->setWordWrap(true);

    serverLayout->addWidget(serverTitle);
    serverLayout->addWidget(hostInput);
    serverLayout->addWidget(portInput);
    serverLayout->addWidget(serverSaveButton);
    serverLayout->addWidget(serverStatusLabel);
    bodyLayout->addWidget(serverCard);

    // ======================= Printer card =======================
    QWidget *printerCard = new QWidget(body);
    printerCard->setObjectName("cardPanel");
    QVBoxLayout *printerLayout = new QVBoxLayout(printerCard);
    printerLayout->setContentsMargins(20, 16, 20, 16);
    printerLayout->setSpacing(12);

    QLabel *printerTitle = new QLabel("Printer", printerCard);
    printerTitle->setObjectName("sectionHeaderAccent");

    QHBoxLayout *typeLayout = new QHBoxLayout();
    typeLayout->setSpacing(16);
    networkRadio = new QRadioButton("Network", printerCard);
    systemRadio = new QRadioButton("System", printerCard);
    typeLayout->addWidget(networkRadio);
    typeLayout->addWidget(systemRadio);
    typeLayout->addStretch();

    // Network fields (IP + port)
    networkFields = new QWidget(printerCard);
    QVBoxLayout *networkLayout = new QVBoxLayout(networkFields);
    networkLayout->setContentsMargins(0, 0, 0, 0);
    networkLayout->setSpacing(10);

    printerAddressInput = new QLineEdit(networkFields);
    printerAddressInput->setFixedHeight(ScreenUtils::px(40));
    printerAddressInput->setPlaceholderText("Printer IP (e.g. 192.168.1.100)");

    printerPortInput = new QLineEdit(networkFields);
    printerPortInput->setFixedHeight(ScreenUtils::px(40));
    printerPortInput->setPlaceholderText("Port (default 9100)");
    printerPortInput->setValidator(new QIntValidator(1, 65535, this));

    networkLayout->addWidget(printerAddressInput);
    networkLayout->addWidget(printerPortInput);

    // System fields (printer names)
    systemFields = new QWidget(printerCard);
    QFormLayout *systemLayout = new QFormLayout(systemFields);
    systemLayout->setContentsMargins(0, 0, 0, 0);
    systemLayout->setSpacing(10);

    receiptPrinterInput = new QLineEdit(systemFields);
    receiptPrinterInput->setFixedHeight(ScreenUtils::px(40));
    kitchenPrinterInput = new QLineEdit(systemFields);
    kitchenPrinterInput->setFixedHeight(ScreenUtils::px(40));
    systemLayout->addRow(new QLabel("Receipt Printer:", systemFields), receiptPrinterInput);
    systemLayout->addRow(new QLabel("Kitchen Printer:", systemFields), kitchenPrinterInput);

    QHBoxLayout *printerButtonsLayout = new QHBoxLayout();
    printerButtonsLayout->setSpacing(12);
    QPushButton *printerSaveButton = new QPushButton("Save Printer", printerCard);
    printerSaveButton->setObjectName("primaryButton");
    printerSaveButton->setFixedHeight(ScreenUtils::px(42));
    QPushButton *testPrintButton = new QPushButton("Test Print", printerCard);
    testPrintButton->setObjectName("ghostButton");
    testPrintButton->setFixedHeight(ScreenUtils::px(42));
    printerButtonsLayout->addWidget(printerSaveButton);
    printerButtonsLayout->addWidget(testPrintButton);

    printerStatusLabel = new QLabel(printerCard);
    printerStatusLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #64748b;"));
    printerStatusLabel->setWordWrap(true);

    printerLayout->addWidget(printerTitle);
    printerLayout->addLayout(typeLayout);
    printerLayout->addWidget(networkFields);
    printerLayout->addWidget(systemFields);
    printerLayout->addLayout(printerButtonsLayout);
    printerLayout->addWidget(printerStatusLabel);
    bodyLayout->addWidget(printerCard);

    // ======================= Logout =======================
    logoutButton = new QPushButton("Logout", body);
    logoutButton->setObjectName("dangerButton");
    logoutButton->setFixedHeight(ScreenUtils::px(46));
    bodyLayout->addWidget(logoutButton);

    bodyLayout->addStretch();
    scrollArea->setWidget(body);
    mainLayout->addWidget(scrollArea, 1);

    connect(backButton, &QPushButton::clicked, this, &SettingsScreen::onBackClicked);
    connect(logoutButton, &QPushButton::clicked, this, &SettingsScreen::onLogoutClicked);
    connect(serverSaveButton, &QPushButton::clicked, this, &SettingsScreen::saveServer);
    connect(printerSaveButton, &QPushButton::clicked, this, &SettingsScreen::savePrinter);
    connect(testPrintButton, &QPushButton::clicked, this, &SettingsScreen::testPrint);
    connect(networkRadio, &QRadioButton::toggled, this, &SettingsScreen::onPrinterTypeChanged);
    connect(systemRadio, &QRadioButton::toggled, this, &SettingsScreen::onPrinterTypeChanged);
}

void SettingsScreen::loadConfig() {
    QString username = settingConfig.getValue("userDetail.username").toString();
    QString store = settingConfig.getValue("userDetail.store.name").toString();
    QString brand = settingConfig.getValue("userDetail.brand.name").toString();

    usernameValue->setText(username);
    storeValue->setText("Store : " + store);
    brandValue->setText("Brand : " + brand);

    QString host = settingConfig.getApiEndpoint("auth", "login");
    QUrl url(host);
    hostInput->setText(url.host());
    portInput->setText(QString::number(url.port(8080)));

    QString type = settingConfig.getValue("printer/type", "system").toString();
    networkRadio->setChecked(type == "network");
    systemRadio->setChecked(type != "network");

    printerAddressInput->setText(settingConfig.getValue("printer/address").toString());
    printerPortInput->setText(QString::number(settingConfig.getValue("printer/port", 9100).toInt()));
    receiptPrinterInput->setText(settingConfig.getApiEndpoint("printer", "receipt"));
    kitchenPrinterInput->setText(settingConfig.getApiEndpoint("printer", "kitchen"));

    onPrinterTypeChanged();
}

void SettingsScreen::onPrinterTypeChanged() {
    bool network = networkRadio->isChecked();
    networkFields->setVisible(network);
    systemFields->setVisible(!network);
}

void SettingsScreen::saveServer() {
    QString host = hostInput->text().trimmed();
    int port = portInput->text().toInt();

    if (host.isEmpty()) {
        serverStatusLabel->setText("Please enter a server host.");
        serverStatusLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #dc2626;"));
        return;
    }

    QString baseUrl = QString("http://%1:%2").arg(host).arg(port);

    settingConfig.setValue("auth/login", baseUrl + "/api/v1/auth/login");
    settingConfig.setValue("auth/user", baseUrl + "/api/v1/user");
    settingConfig.setValue("auth/refresh-token", baseUrl + "/api/v1/auth/refresh-token");
    settingConfig.setValue("menu/category", baseUrl + "/api/v1/category");
    settingConfig.setValue("menu/product", baseUrl + "/api/v1/product");
    settingConfig.setValue("order/confirm", baseUrl + "/api/v1/order");
    settingConfig.setValue("order/daily", baseUrl + "/api/v1/order");
    settingConfig.setValue("order/payment", baseUrl + "/api/v1/order");
    settingConfig.setValue("order/void", baseUrl + "/api/v1/order/%1/void");
    settingConfig.setValue("reports/settlement", baseUrl + "/api/v1/reports/settlement");
    settingConfig.setValue("reports/daily", baseUrl + "/api/v1/reports/daily");
    settingConfig.setValue("reports/sales", baseUrl + "/api/v1/reports/sales");
    settingConfig.setValue("reports/order-volume", baseUrl + "/api/v1/reports/order-volume");
    settingConfig.sync();

    serverStatusLabel->setText("Server saved. Please restart the application.");
    serverStatusLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #16a34a;"));
}

void SettingsScreen::savePrinter() {
    settingConfig.setValue("printer/type", networkRadio->isChecked() ? "network" : "system");
    settingConfig.setValue("printer/address", printerAddressInput->text().trimmed());
    settingConfig.setValue("printer/port", printerPortInput->text().trimmed().isEmpty()
                                              ? 9100
                                              : printerPortInput->text().trimmed().toInt());
    settingConfig.setValue("printer/receipt", receiptPrinterInput->text().trimmed());
    settingConfig.setValue("printer/kitchen", kitchenPrinterInput->text().trimmed());
    settingConfig.sync();

    printerStatusLabel->setText("Printer saved: " + QString(networkRadio->isChecked() ? "network" : "system"));
    printerStatusLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #16a34a;"));
}

void SettingsScreen::testPrint() {
    savePrinter();

    QString type = settingConfig.getValue("printer/type", "system").toString();
    QString printerName = settingConfig.getApiEndpoint("printer", "receipt");
    if (type == "network" && settingConfig.getValue("printer/address").toString().isEmpty()) {
        printerStatusLabel->setText("Printer IP not set.");
        printerStatusLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #dc2626;"));
        return;
    }
    if (type != "network" && printerName.isEmpty()) {
        printerStatusLabel->setText("Receipt printer name not set.");
        printerStatusLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #dc2626;"));
        return;
    }

    QByteArray testData;
    testData.append("\x1B\x40");
    testData.append("\x1B\x61\x01");
    testData.append(" " + settingConfig.getValue("userDetail.brand.name", "Kopi Harmoni").toString().toUtf8() + " \n");
    testData.append("----------------------------\n");
    testData.append("TEST PRINT\n");
    testData.append("Printer: " + type.toUtf8() + "\n");
    testData.append("----------------------------\n");
    testData.append("Terima kasih\n");
    testData.append("\n\n\n");
    testData.append("\x1D\x56\x01");

    printerStatusLabel->setText("Printing...");
    printerStatusLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #64748b;"));
    QApplication::processEvents();

    bool ok = OrderPrint::sendRaw(printerName, testData);
    if (ok) {
        printerStatusLabel->setText("Print sent successfully.");
        printerStatusLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #16a34a;"));
    } else {
        printerStatusLabel->setText("Print failed. Check printer address/name.");
        printerStatusLabel->setStyleSheet(ScreenUtils::qss("font-size: 12px; color: #dc2626;"));
    }
}

void SettingsScreen::onBackClicked() {
    emit backRequested();
    close();
}

void SettingsScreen::onLogoutClicked() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Logout");
    msgBox.setText("Are you sure you want to logout?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes) {
        TokenManager::instance().clearTokens();
        CacheUtils::clearAppCache();
        emit loggedOut();
        close();
    }
}

#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QIntValidator>
#include <QMessageBox>
#include <QApplication>

#include "loginscreen.h"
#include "mainwindow.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), configSetting() {
    setWindowTitle("Settings");
    setModal(true);
    resize(400, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Header
    QLabel *headerLabel = new QLabel("Settings", this);
    headerLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #4CAF50;");
    mainLayout->addWidget(headerLabel);

    // Server configuration
    QLabel *serverLabel = new QLabel("Server Configuration", this);
    serverLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");
    mainLayout->addWidget(serverLabel);

    QFormLayout *serverForm = new QFormLayout();
    hostInput = new QLineEdit(this);
    portInput = new QLineEdit(this);
    portInput->setValidator(new QIntValidator(1, 65535, this));
    serverForm->addRow(new QLabel("Host:", this), hostInput);
    serverForm->addRow(new QLabel("Port:", this), portInput);
    mainLayout->addLayout(serverForm);

    saveServerBtn = new QPushButton("Save Server", this);
    QHBoxLayout *saveServerLayout = new QHBoxLayout();
    saveServerLayout->addWidget(saveServerBtn);
    mainLayout->addLayout(saveServerLayout);
    connect(saveServerBtn, &QPushButton::clicked, this, &SettingsDialog::saveServer);

    statusLabel = new QLabel("", this);
    statusLabel->setStyleSheet("font-size: 12px; color: #666666;");
    mainLayout->addWidget(statusLabel);

    // Printer configuration
    QLabel *printerLabel = new QLabel("Printer Configuration", this);
    printerLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");
    mainLayout->addWidget(printerLabel);

    QFormLayout *printerForm = new QFormLayout();
    receiptPrinterInput = new QLineEdit(this);
    kitchenPrinterInput = new QLineEdit(this);
    printerForm->addRow(new QLabel("Receipt Printer:", this), receiptPrinterInput);
    printerForm->addRow(new QLabel("Kitchen Printer:", this), kitchenPrinterInput);
    mainLayout->addLayout(printerForm);

    QPushButton *savePrinterBtn = new QPushButton("Save Printers", this);
    QHBoxLayout *savePrinterLayout = new QHBoxLayout();
    savePrinterLayout->addWidget(savePrinterBtn);
    mainLayout->addLayout(savePrinterLayout);
    connect(savePrinterBtn, &QPushButton::clicked, this, [this]() {
        configSetting.setValue("printer/receipt", receiptPrinterInput->text());
        configSetting.setValue("printer/kitchen", kitchenPrinterInput->text());
        configSetting.sync();
        statusLabel->setText("Printers saved.");
    });

    // Spacer
    mainLayout->addStretch();

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    logoutBtn = new QPushButton("Logout", this);
    logoutBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #dc3545;"
        "color: white;"
        "padding: 8px 16px;"
        "border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "background-color: #c82333;"
        "}"
    );
    buttonLayout->addWidget(logoutBtn);

    closeBtn = new QPushButton("Close", this);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #6c757d;"
        "color: white;"
        "padding: 8px 16px;"
        "border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "background-color: #5a6268;"
        "}"
    );
    buttonLayout->addWidget(closeBtn);

    mainLayout->addLayout(buttonLayout);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(logoutBtn, &QPushButton::clicked, this, &SettingsDialog::onLogoutClicked);

    loadConfig();
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::loadConfig() {
    QString host = configSetting.getApiEndpoint("auth", "login");
    // Extract host and port from the login URL
    // e.g., http://localhost:8080/api/v1/auth/login -> host=localhost, port=8080
    QUrl url(host);
    hostInput->setText(url.host());
    portInput->setText(QString::number(url.port(8080)));

    receiptPrinterInput->setText(configSetting.getApiEndpoint("printer", "receipt"));
    kitchenPrinterInput->setText(configSetting.getApiEndpoint("printer", "kitchen"));
}

void SettingsDialog::saveServer() {
    QString host = hostInput->text().trimmed();
    int port = portInput->text().toInt();

    // Update all API endpoints by modifying the config
    QString baseUrl = QString("http://%1:%2").arg(host).arg(port);

    // Update config with new base URL
    configSetting.setValue("auth/login", baseUrl + "/api/v1/auth/login");
    configSetting.setValue("auth/user", baseUrl + "/api/v1/user");
    configSetting.setValue("auth/refresh-token", baseUrl + "/api/v1/auth/refresh-token");
    configSetting.setValue("menu/category", baseUrl + "/api/v1/category");
    configSetting.setValue("menu/product", baseUrl + "/api/v1/product");
    configSetting.setValue("order/confirm", baseUrl + "/api/v1/order");
    configSetting.setValue("order/daily", baseUrl + "/api/v1/order");
    configSetting.setValue("order/payment", baseUrl + "/api/v1/order");
    configSetting.setValue("order/void", baseUrl + "/api/v1/order/%1/void");
    configSetting.setValue("reports/settlement", baseUrl + "/api/v1/reports/settlement");
    configSetting.setValue("reports/daily", baseUrl + "/api/v1/reports/daily");
    configSetting.setValue("reports/sales", baseUrl + "/api/v1/reports/sales");
    configSetting.setValue("reports/order-volume", baseUrl + "/api/v1/reports/order-volume");
    configSetting.sync();

    statusLabel->setText("Server configuration saved. Please restart the application.");
}

void SettingsDialog::onLogoutClicked() {
    // Show confirmation dialog
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Logout");
    msgBox.setText("Are you sure you want to logout?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes) {
        accept();
    }
}

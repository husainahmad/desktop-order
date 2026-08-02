#include "loginscreen.h"
#include "ui_loginscreen.h"
#include <QMessageBox>
#include "orderscreen.h"
#include "user.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QLabel>
#include <setting.h>
#include "cacheutils.h"
#include "tokenmanager.h"
#include "screenutils.h"

LoginScreen::LoginScreen(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginScreen), networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    setWindowTitle("Login");
    setFixedSize(ScreenUtils::fittedSize(440, 560, 0.8, 0.85)); // Sized to fit 13" screens

    // Main Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(50, 50, 50, 50);

    // Logo
    QLabel *logoLabel = new QLabel(this);
    QPixmap logo(":/assets/images/pizza.png");
    logo = logo.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    logoLabel->setPixmap(logo);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setAttribute(Qt::WA_TranslucentBackground);

    // Username field
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Username");
    usernameEdit->setStyleSheet("padding: 5px; font-size: 14px;");

    // Password field
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet("padding: 5px; font-size: 14px;");

    // Login button
    loginButton = new QPushButton("Login", this);
    loginButton->setStyleSheet("QPushButton {"
                               "background-color: #007BFF;"
                               "color: white;"
                               "font-size: 16px;"
                               "padding: 8px;"
                               "border-radius: 5px;"
                               "} QPushButton:hover {"
                               "background-color: #0056b3;"
                               "}");

    // Add widgets to layout
    layout->addWidget(logoLabel);
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    layout->addWidget(loginButton);

    // Set layout to the main widget
    setLayout(layout);

    connect(loginButton, &QPushButton::clicked, this, &LoginScreen::handleLogin);
    connect(usernameEdit, &QLineEdit::returnPressed, this, &LoginScreen::handleLogin);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginScreen::handleLogin);
}

LoginScreen::~LoginScreen()
{
    delete ui;
}

void LoginScreen::handleLogin() {
    QString username = usernameEdit->text();
    QString password = passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password");
        return;
    }

    QString loginUrl = settingConfig.getApiEndpoint("auth", "login");

    QNetworkRequest requestLogin(loginUrl);
    requestLogin.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["username"] = username;
    json["password"] = password;
    QJsonDocument doc(json);

    QNetworkReply* replyDetail = networkManager->post(requestLogin, doc.toJson());

    connect(replyDetail, &QNetworkReply::finished, this, [this, replyDetail]() {
        if (replyDetail->error() == QNetworkReply::NoError) {

            QByteArray responseData = replyDetail->readAll();
            if (responseData.isEmpty()) {
                return;
            }

            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonDoc.object();
            QJsonObject tokenObj = jsonObj["data"].isObject() ? jsonObj["data"].toObject() : jsonObj;

            QString accessToken = tokenObj["accessToken"].toString();
            QString refreshToken = tokenObj["refreshToken"].toString();

            TokenManager::instance().setTokens(accessToken, refreshToken);

            handleUserDetail();
        } else {
            qDebug() << "Login failed: " << replyDetail->errorString();
            QMessageBox::warning(this, "Error", "Login failed: " + replyDetail->errorString());
        }
        replyDetail->deleteLater();
    });

}

void LoginScreen::handleUserDetail() {
    QString username = usernameEdit->text();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password");
        return;
    }

    QString loginUrl = settingConfig.getApiEndpoint("auth","user");

    QNetworkRequest requestUser(QUrl(loginUrl + "/" + username));
    requestUser.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    requestUser.setRawHeader("Authorization", "Bearer " + TokenManager::instance().getAccessToken().toUtf8());

    QNetworkReply* replyDetail = networkManager->get(requestUser);

    connect(replyDetail, &QNetworkReply::finished, this, [this, replyDetail]() {
        if (replyDetail->error() == QNetworkReply::NoError) {

            QByteArray responseData = replyDetail->readAll();
            if (responseData.isEmpty()) {
                return;
            }

            CacheUtils::clearAppCache();

            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObj = jsonDoc.object();
            QJsonObject userObj = jsonObj["data"].toObject();

            QJsonObject jsonObjStore = userObj["store"].toObject();
            QJsonObject jsonObjChain = jsonObjStore["chain"].toObject();
            QJsonObject jsonObjBrand = jsonObjChain["brand"].toObject();

            Brand brand(jsonObjBrand["id"].toInt(), jsonObjBrand["name"].toString());
            Chain chain(jsonObjChain["id"].toInt(), jsonObjChain["name"].toString(), brand);
            Store store(jsonObjStore["id"].toInt(), jsonObjStore["name"].toString(), chain);

            User user(userObj["id"].toInt(), userObj["username"].toString(), store);

            settingConfig.setValue("userDetail.username", user.username);
            settingConfig.setValue("userDetail.store.id", user.store.id);
            settingConfig.setValue("userDetail.store.name", user.store.name);
            settingConfig.setValue("userDetail.chain.id", user.store.chain.id);
            settingConfig.setValue("userDetail.chain.name", user.store.chain.name);
            settingConfig.setValue("userDetail.brand.id", user.store.chain.brand.id);
            settingConfig.setValue("userDetail.brand.name", user.store.chain.brand.name);
            settingConfig.sync();

            OrderScreen *orderScreen = new OrderScreen();
            orderScreen->showFullScreen();

            this->close();
        } else {
            qDebug() << "Login failed: " << replyDetail->errorString();
            QMessageBox::warning(this, "Error", "Login failed: " + replyDetail->errorString());
        }
        replyDetail->deleteLater();
    });

}

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

LoginScreen::LoginScreen(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginScreen), networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    setWindowTitle("Login");
    setFixedSize(400, 500); // Fixed window size

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

    QNetworkRequest requestLogin(QUrl(QString("http://localhost:8282/api/v1/auth/login")));
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
            QString dataToken = jsonObj["data"].toString();

            QSettings settings;
            settings.setValue("authToken", dataToken);
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

    QNetworkRequest requestUser(QUrl(QString("http://localhost:8080/api/v1/user/%1").arg(username)));
    requestUser.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* replyDetail = networkManager->get(requestUser);

    connect(replyDetail, &QNetworkReply::finished, this, [this, replyDetail]() {
        if (replyDetail->error() == QNetworkReply::NoError) {

            QByteArray responseData = replyDetail->readAll();
            if (responseData.isEmpty()) {
                return;
            }

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

            QSettings settings;
            settings.setValue("userDetail.username", user.username);
            settings.setValue("userDetail.store.id", user.store.id);
            settings.setValue("userDetail.store.name", user.store.name);
            settings.setValue("userDetail.chain.id", user.store.chain.id);
            settings.setValue("userDetail.chain.name", user.store.chain.name);
            settings.setValue("userDetail.brand.id", user.store.chain.brand.id);
            settings.setValue("userDetail.brand.name", user.store.chain.brand.name);
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

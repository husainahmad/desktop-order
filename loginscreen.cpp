#include "loginscreen.h"
#include "ui_loginscreen.h"
#include <QMessageBox>
#include "orderscreen.h"
#include "user.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <setting.h>
#include "cacheutils.h"
#include "tokenmanager.h"
#include "screenutils.h"
#include "apiclient.h"

LoginScreen::LoginScreen(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginScreen)
{
    ui->setupUi(this);
    setWindowTitle("Login");
    setFixedSize(ScreenUtils::fittedSize(ScreenUtils::px(440), ScreenUtils::px(560), 0.8, 0.85)); // Sized to fit 13" screens

    // Dark gradient background
    setStyleSheet(
        "LoginScreen {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "       stop:0 #0f172a, stop:1 #1e3a5f);"
        "}"
        );

    // Outer layout centers the login card
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setAlignment(Qt::AlignCenter);

    // Centered white card
    QWidget *card = new QWidget(this);
    card->setObjectName("loginCard");
    card->setFixedWidth(ScreenUtils::px(360));

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setSpacing(16);
    layout->setContentsMargins(32, 36, 32, 36);

    // Logo
    QLabel *logoLabel = new QLabel(card);
    QPixmap logo(":/assets/images/pizza.png");
    logo = logo.scaled(ScreenUtils::px(96), ScreenUtils::px(96), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    logoLabel->setPixmap(logo);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setAttribute(Qt::WA_TranslucentBackground);

    // Title
    QLabel *titleLabel = new QLabel("Welcome Back", card);
    titleLabel->setObjectName("loginTitle");
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel *subTitleLabel = new QLabel("Sign in to start taking orders", card);
    subTitleLabel->setObjectName("loginSub");
    subTitleLabel->setAlignment(Qt::AlignCenter);

    // Username field
    usernameEdit = new QLineEdit(card);
    usernameEdit->setPlaceholderText("Username");
    usernameEdit->setFixedHeight(ScreenUtils::px(44));
    usernameEdit->setStyleSheet(ScreenUtils::qss("padding: 5px 14px; font-size: 15px; border-radius: 10px;"));

    // Password field
    passwordEdit = new QLineEdit(card);
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(ScreenUtils::px(44));
    passwordEdit->setStyleSheet(ScreenUtils::qss("padding: 5px 14px; font-size: 15px; border-radius: 10px;"));

    // Login button
    loginButton = new QPushButton("Login", card);
    loginButton->setFixedHeight(ScreenUtils::px(48));
    loginButton->setObjectName("primaryButton");
    loginButton->setStyleSheet(ScreenUtils::qss(
        "QPushButton {"
        "   background-color: #2563eb;"
        "   color: white;"
        "   font-size: 16px;"
        "   font-weight: 700;"
        "   border-radius: 10px;"
        "}"
        "QPushButton:hover { background-color: #1d4ed8; }"
        "QPushButton:pressed { background-color: #1e40af; }"));

    // Add widgets to layout
    layout->addWidget(logoLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(subTitleLabel);
    layout->addSpacing(4);
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    layout->addSpacing(4);
    layout->addWidget(loginButton);

    outerLayout->addWidget(card);

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

    QJsonObject json;
    json["username"] = username;
    json["password"] = password;
    QByteArray body = QJsonDocument(json).toJson();

    const QUrl url(settingConfig.getApiEndpoint("auth", "login"));
    ApiClient::instance().post(url, body, [this](const QJsonObject &response) {
        QJsonObject tokenObj = response["data"].isObject() ? response["data"].toObject() : response;

        QString accessToken = tokenObj["accessToken"].toString();
        QString refreshToken = tokenObj["refreshToken"].toString();

        if (accessToken.isEmpty()) {
            QMessageBox::warning(this, "Error", "Login failed: no access token returned.");
            return;
        }

        TokenManager::instance().setTokens(accessToken, refreshToken);
        handleUserDetail();
    }, [this](const QString &message, int) {
        qDebug() << "Login failed:" << message;
        QMessageBox::warning(this, "Error", "Login failed: " + message);
    });
}

void LoginScreen::handleUserDetail() {
    QString username = usernameEdit->text();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password");
        return;
    }

    QUrl url(settingConfig.getApiEndpoint("auth","user") + "/" + username);
    ApiClient::instance().get(url, [this](const QJsonObject &response) {
        CacheUtils::clearAppCache();

        QJsonObject userObj = response["data"].toObject();

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
    }, [this](const QString &message, int) {
        qDebug() << "Login failed:" << message;
        QMessageBox::warning(this, "Error", "Login failed: " + message);
    });
}

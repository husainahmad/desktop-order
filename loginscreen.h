#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QWidget>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLineEdit>
#include <QPushButton>
#include <setting.h>

namespace Ui {
class LoginScreen;
}

class LoginScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LoginScreen(QWidget *parent = nullptr);
    ~LoginScreen();

private slots:
    void handleLogin();
    void handleUserDetail();

private:
    Ui::LoginScreen *ui;
    QNetworkAccessManager *networkManager;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    Setting settingConfig;
};

#endif // LOGINSCREEN_H

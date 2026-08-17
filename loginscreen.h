#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QWidget>

#include <QLineEdit>
#include <QPushButton>
#include <setting.h>

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
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    Setting settingConfig;
};

#endif // LOGINSCREEN_H

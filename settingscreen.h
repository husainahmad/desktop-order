#ifndef SETTINGSCREEN_H
#define SETTINGSCREEN_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include "setting.h"

class SettingsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsScreen(QWidget *parent = nullptr);
    ~SettingsScreen();

signals:
    void backRequested();
    void loggedOut();

private slots:
    void saveServer();
    void savePrinter();
    void testPrint();
    void onPrinterTypeChanged();
    void onBackClicked();
    void onLogoutClicked();

private:
    void buildUi();
    void loadConfig();

    QLabel *usernameValue;
    QLabel *storeValue;
    QLabel *brandValue;

    QLineEdit *hostInput;
    QLineEdit *portInput;
    QLabel *serverStatusLabel;

    QRadioButton *networkRadio;
    QRadioButton *systemRadio;
    QWidget *networkFields;
    QLineEdit *printerAddressInput;
    QLineEdit *printerPortInput;
    QWidget *systemFields;
    QLineEdit *receiptPrinterInput;
    QLineEdit *kitchenPrinterInput;
    QLabel *printerStatusLabel;

    QPushButton *backButton;
    QPushButton *logoutButton;

    Setting settingConfig;
};

#endif // SETTINGSCREEN_H

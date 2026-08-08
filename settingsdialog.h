#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "setting.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void saveServer();
    void onLogoutClicked();

private:
    void loadConfig();

    QLineEdit *hostInput;
    QLineEdit *portInput;
    QLineEdit *receiptPrinterInput;
    QLineEdit *kitchenPrinterInput;
    QLabel *statusLabel;
    QPushButton *saveServerBtn;
    QPushButton *logoutBtn;
    QPushButton *closeBtn;
    Setting configSetting;
};

#endif // SETTINGSDIALOG_H

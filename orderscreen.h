#ifndef ORDERSCREEN_H
#define ORDERSCREEN_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLocale>
#include "setting.h"

class OrderScreen : public QWidget
{
    Q_OBJECT

public:
    explicit OrderScreen(QWidget *parent = nullptr);
    ~OrderScreen();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onOrderClicked();
    void fetchDataFromAPI();
    void parseJsonResponse(const QByteArray &responseData);
    void onTabChanged(int index);
    void onSettlementClicked();
    void onDailyReportClicked();
    void onSalesReportClicked();
    void onSettingsClicked();
    void onLogoutClicked();

private:
    QTabWidget *tabWidget;
    QWidget *firstTab;
    QPushButton *logoutButton;

    QVBoxLayout *summaryLayout;
    QWidget *summaryWidget;
    QVBoxLayout *ordersLayout;
    QWidget *ordersWidget;

    Setting settingConfig;
    QLocale locale;
    int orderNumberCounter = 0;
    bool exitConfirmed = false;
};

#endif // ORDERSCREEN_H

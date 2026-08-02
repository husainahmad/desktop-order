#ifndef SALESREPORTSCREEN_H
#define SALESREPORTSCREEN_H

#include <QWidget>
#include <QTableWidget>
#include <QDateEdit>
#include <QNetworkAccessManager>
#include <QLocale>
#include "setting.h"

class SalesReportScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SalesReportScreen(QWidget *parent = nullptr);
    ~SalesReportScreen();

private slots:
    void fetchSalesReport();

private:
    void parseSalesReportResponse(const QByteArray &responseData);

    QTableWidget *tableWidget;
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    QNetworkAccessManager *networkManager;
    Setting settingConfig;
    QLocale locale;
};

#endif // SALESREPORTSCREEN_H

#ifndef SALESREPORTSCREEN_H
#define SALESREPORTSCREEN_H

#include <QWidget>
#include <QTableWidget>
#include <QDateEdit>
#include <QLocale>
#include "setting.h"

class SalesReportScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SalesReportScreen(QWidget *parent = nullptr);
    ~SalesReportScreen();

signals:
    void backRequested();

private slots:
    void fetchSalesReport();
    void onBackClicked();

private:
    void parseSalesReportResponse(const QByteArray &responseData);

    QTableWidget *tableWidget;
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    Setting settingConfig;
    QLocale locale;
};

#endif // SALESREPORTSCREEN_H

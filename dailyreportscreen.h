#ifndef DAILYREPORTSCREEN_H
#define DAILYREPORTSCREEN_H

#include <QWidget>
#include <QTableWidget>
#include <QDateEdit>
#include <QNetworkAccessManager>
#include <QLocale>
#include "setting.h"

class DailyReportScreen : public QWidget
{
    Q_OBJECT

public:
    explicit DailyReportScreen(QWidget *parent = nullptr);
    ~DailyReportScreen();

private slots:
    void fetchDailyReport();

private:
    void parseDailyReportResponse(const QByteArray &responseData);

    QTableWidget *tableWidget;
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    QNetworkAccessManager *networkManager;
    Setting settingConfig;
    QLocale locale;
};

#endif // DAILYREPORTSCREEN_H

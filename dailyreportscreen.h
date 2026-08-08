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
    void fetchOrderVolumeReport();

private:
    void parseDailyReportResponse(const QByteArray &responseData);
    void parseOrderVolumeReportResponse(const QByteArray &responseData);

    QTableWidget *tableWidget;
    QTableWidget *orderVolumeTableWidget;
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    QNetworkAccessManager *networkManager;
    Setting settingConfig;
    QLocale locale;
};

#endif // DAILYREPORTSCREEN_H

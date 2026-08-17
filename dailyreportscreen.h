#ifndef DAILYREPORTSCREEN_H
#define DAILYREPORTSCREEN_H

#include <QWidget>
#include <QTableWidget>
#include <QDateEdit>
#include <QLocale>
#include "setting.h"

class DailyReportScreen : public QWidget
{
    Q_OBJECT

public:
    explicit DailyReportScreen(QWidget *parent = nullptr);
    ~DailyReportScreen();

signals:
    void backRequested();

private slots:
    void fetchDailyReport();
    void fetchOrderVolumeReport();
    void onBackClicked();

private:
    void parseDailyReportResponse(const QByteArray &responseData);
    void parseOrderVolumeReportResponse(const QByteArray &responseData);

    QTableWidget *tableWidget;
    QTableWidget *orderVolumeTableWidget;
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    Setting settingConfig;
    QLocale locale;
};

#endif // DAILYREPORTSCREEN_H

#ifndef ORDERTABLEWIDGET_H
#define ORDERTABLEWIDGET_H

#include <QWidget>
#include <QTabWidget>

namespace Ui {
class OrderTableWidget;
}

class OrderTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderTableWidget(const QJsonArray &dataArray, QTabWidget *tabWidget, QWidget *parent = nullptr);
    ~OrderTableWidget();

private:
    Ui::OrderTableWidget *ui;
    QTabWidget *tabWidget;
};

#endif // ORDERTABLEWIDGET_H

#ifndef ORDERTABLEWIDGET_H
#define ORDERTABLEWIDGET_H

#include <QWidget>

namespace Ui {
class OrderTableWidget;
}

class OrderTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderTableWidget(const QJsonArray &dataArray, QWidget *parent = nullptr);
    ~OrderTableWidget();

private:
    Ui::OrderTableWidget *ui;
};

#endif // ORDERTABLEWIDGET_H

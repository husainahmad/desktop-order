#ifndef PRODUCTWIDGET_H
#define PRODUCTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QLocale>
#include <QMouseEvent>
#include "product.h"

namespace Ui {
class ProductWidget;
}

class ProductWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProductWidget(Product product, QWidget *parent = nullptr);
    ~ProductWidget();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void setImageFromBase64(QLabel *label, const QString &base64String);
private:
    Ui::ProductWidget *ui;
    QLocale locale;
};

#endif // PRODUCTWIDGET_H

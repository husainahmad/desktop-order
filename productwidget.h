#ifndef PRODUCTWIDGET_H
#define PRODUCTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QLocale>
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

private slots:
    void setImageFromBase64(QLabel *label, const QString &base64String);
private:
    Ui::ProductWidget *ui;
    QLocale locale;
};

#endif // PRODUCTWIDGET_H

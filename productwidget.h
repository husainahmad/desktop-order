#ifndef PRODUCTWIDGET_H
#define PRODUCTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QLocale>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QEvent>
#include "product.h"

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
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void setImageFromBase64(QLabel *label, const QString &base64String);
private:
    QLocale locale;
    QLabel *hoverBadge = nullptr;
    QLabel *m_imageLabel = nullptr;
};

#endif // PRODUCTWIDGET_H

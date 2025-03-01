#ifndef ORDERPOPUPWINDOW_H
#define ORDERPOPUPWINDOW_H

#include <QDialog>
#include <QTextBrowser>
#include <QJsonObject>
#include <QLocale>

class QTextBrowser;
class QPushButton;

class OrderPopupWindow : public QDialog {
    Q_OBJECT

public:
    explicit OrderPopupWindow(const QJsonObject &order, QWidget *parent = nullptr);
    ~OrderPopupWindow();

private slots:
    void printOrder();
    void closeWindow();
private:
    QTextBrowser *htmlViewer;
    QPushButton *printButton;
    QPushButton *closeButton;
    QLocale locale;
};

#endif // ORDERPOPUPWINDOW_H

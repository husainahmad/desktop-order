#ifndef SPLASSCREEN_H
#define SPLASSCREEN_H

#include <QWidget>

namespace Ui {
class SplasScreen;
}

class SplasScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplasScreen(QWidget *parent = nullptr);
    ~SplasScreen();

private:
    Ui::SplasScreen *ui;
};

#endif // SPLASSCREEN_H

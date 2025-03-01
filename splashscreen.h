#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>

namespace Ui {
class SplashScreen;
}

class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(QWidget *parent = nullptr);
    ~SplashScreen();

private:
    Ui::SplashScreen *ui;
};

#endif // SPLASHSCREEN_H

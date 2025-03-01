#include "loginscreen.h"
#include "mainwindow.h"

#include <QApplication>
#include <QSplashScreen>
#include <QThread>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSplashScreen splash;

    splash.show();
    QThread::sleep(2);

    LoginScreen loginScreen;

    loginScreen.resize(800, 600);
    loginScreen.show();

    //MainWindow w;
    //w.show();

    splash.finish(&loginScreen);
    return a.exec();
}

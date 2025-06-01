#include "loginscreen.h"
#include "mainwindow.h"

#include <QApplication>
#include <QSplashScreen>
#include <QThread>


#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include "LoginScreen.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize splash screen with an image
    QPixmap pixmap(":/assets/images/splash.jpg");  // Ensure you have a valid image path
    QSplashScreen splash(pixmap);
    splash.show();

    // Process UI events while splash is visible
    QCoreApplication::processEvents();

    // Simulate loading time (non-blocking)
    QTimer::singleShot(2000, [&]() {
        LoginScreen *loginScreen = new LoginScreen();
        loginScreen->resize(800, 600);
        loginScreen->show();
        splash.finish(loginScreen);
    });

    return a.exec();
}


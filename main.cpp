#include "loginscreen.h"
#include "mainwindow.h"

#include <QApplication>
#include <QSplashScreen>
#include <QThread>


#include <QApplication>
#include <QSplashScreen>
#include <QTimer>


static QTextStream *gLogStream = nullptr;

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(type);
    Q_UNUSED(context);

    if (gLogStream) {
        QString output = QString("[%1] %2\n")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(msg);
        *gLogStream << output;
        gLogStream->flush();
    }
}


void installLogger() {
    QFile *logFile = new QFile("log.txt");
    logFile->open(QIODevice::Append | QIODevice::Text);
    gLogStream = new QTextStream(logFile);  // assign to global

    qInstallMessageHandler(myMessageHandler);
}


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    installLogger(); // Enable file logging

    qDebug() << "✅ Application started in release mode with debug log enabled";


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

    return app.exec();
}


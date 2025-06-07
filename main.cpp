#include "loginscreen.h"
#include "mainwindow.h"

#include <QApplication>
#include <QSplashScreen>
#include <QThread>


#include <QApplication>
#include <QSplashScreen>
#include <QTimer>

QTextStream* gLogStream = nullptr;

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg) {
    if (!gLogStream) return;

    QString level;
    switch (type) {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtInfoMsg:     level = "INFO"; break;
    case QtWarningMsg:  level = "WARN"; break;
    case QtCriticalMsg: level = "CRIT"; break;
    case QtFatalMsg:    level = "FATAL"; break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    *gLogStream << "[" << timestamp << "][" << level << "] " << msg << "\n";
    gLogStream->flush();
}

void installLogger() {
    static QFile *logFile = new QFile("log.txt");  // Creates the file in app working directory

    // Open in append + text mode. This will CREATE the file if it doesn't exist.
    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "❌ Failed to open or create log.txt";
        return;
    }

    static QTextStream *stream = new QTextStream(logFile);
    gLogStream = stream;

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


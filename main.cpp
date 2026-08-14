#include "loginscreen.h"
#include "screenutils.h"
#include "apiclient.h"
#include "busyindicator.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QTextStream>
#include <QDateTime>

QTextStream* gLogStream = nullptr;

namespace {
const qint64 kMaxLogSize = 2 * 1024 * 1024; // 2 MB
int gLogWriteCounter = 0;

void rotateLogIfNeeded(QFile *file, QTextStream *stream) {
    if (file->size() < kMaxLogSize) {
        return;
    }

    stream->flush();
    file->close();

    const QString logPath = file->fileName();
    QFile::remove(logPath + ".old");
    QFile::rename(logPath, logPath + ".old");

    if (file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        gLogStream->setDevice(file);
    }
}
}

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

    if (++gLogWriteCounter % 200 == 0) {
        rotateLogIfNeeded(qobject_cast<QFile *>(gLogStream->device()),
                          gLogStream);
    }
}

void installLogger() {
    static QFile *logFile = new QFile("log.txt");

    if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open or create log.txt";
        return;
    }

    static QTextStream *stream = new QTextStream(logFile);
    gLogStream = stream;

    qInstallMessageHandler(myMessageHandler);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    installLogger();

    qDebug() << "Application started";
    qDebug() << "Screen scale factor:" << ScreenUtils::scaleFactor();

    QFont baseFont = app.font();
    qreal pointSize = baseFont.pointSizeF();
    if (pointSize > 0) {
        baseFont.setPointSizeF(pointSize * ScreenUtils::scaleFactor());
    } else {
        baseFont.setPixelSize(qRound(13 * ScreenUtils::scaleFactor()));
    }
    app.setFont(baseFont);

    QFile themeFile(":/theme.qss");
    if (themeFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(ScreenUtils::scalePx(QString::fromUtf8(themeFile.readAll())));
        themeFile.close();
    } else {
        qWarning() << "Failed to load theme.qss";
    }

    LoginScreen loginScreen;
    loginScreen.show();

    QObject::connect(&ApiClient::instance(), &ApiClient::busyChanged,
                     [](bool busy) {
                         if (busy) {
                             BusyIndicator::show();
                         } else {
                             BusyIndicator::hide();
                         }
                     });

    return app.exec();
}
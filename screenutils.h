#ifndef SCREENUTILS_H
#define SCREENUTILS_H

#include <QGuiApplication>
#include <QScreen>
#include <QRect>
#include <QSize>
#include <QSizeF>
#include <QByteArray>
#include <QtGlobal>
#include <QtMath>
#include <QRegularExpression>
#include <QString>

namespace ScreenUtils {

inline QRect availableScreenGeometry() {
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return QRect(0, 0, 1280, 800);
    }
    return screen->availableGeometry();
}

inline int availableHeight() {
    return availableScreenGeometry().height();
}

inline QSize fittedSize(int preferredWidth, int preferredHeight,
                        double widthRatio = 0.95, double heightRatio = 0.92) {
    QRect available = availableScreenGeometry();
    int width = qMin(preferredWidth, qMax(320, (int)(available.width() * widthRatio)));
    int height = qMin(preferredHeight, qMax(240, (int)(available.height() * heightRatio)));
    return QSize(width, height);
}

// Reference design assumed to be a ~13.3" display.
inline double referenceInches() { return 13.3; }

// Reference logical resolution (1280x720) used to avoid overflow on small screens.
inline double referenceWidth() { return 1280.0; }
inline double referenceHeight() { return 720.0; }

// Global scale factor so the UI adapts to the actual device screen size.
// It combines the physical diagonal (keeps visual proportions consistent across
// devices) and the logical resolution (prevents widgets overflowing on smaller
// or lower-resolution displays). The result is cached after first call.
//
// Can be overridden for testing / simulation with the ORDER_SCALE_FACTOR
// environment variable, e.g. ORDER_SCALE_FACTOR=0.83 to simulate an 11" device.
inline double scaleFactor() {
    static double cached = -1.0;
    if (cached < 0.0) {
        QByteArray override = qgetenv("ORDER_SCALE_FACTOR");
        if (!override.isEmpty()) {
            bool ok = false;
            double forced = override.toDouble(&ok);
            if (ok && forced > 0.0) {
                cached = forced;
                return cached;
            }
        }

        QScreen *screen = QGuiApplication::primaryScreen();
        QSizeF mm = screen ? screen->physicalSize() : QSizeF(0, 0);
        double diagInches = 0.0;
        if (mm.width() > 0.0 && mm.height() > 0.0) {
            double w = mm.width() / 25.4;
            double h = mm.height() / 25.4;
            diagInches = qSqrt(w * w + h * h);
        }
        if (diagInches <= 0.0) diagInches = referenceInches();

        QRect avail = availableScreenGeometry();
        double resScale = qMin(avail.width() / referenceWidth(),
                               avail.height() / referenceHeight());
        double physScale = diagInches / referenceInches();
        cached = qBound(0.7, qMin(physScale, resScale), 1.5);
    }
    return cached;
}

// Scale a base pixel value by the current screen scale factor.
inline int px(int base) {
    return qMax(1, qRound(base * scaleFactor()));
}

// Scale a double pixel value by the current screen scale factor.
inline double pxD(double base) {
    return base * scaleFactor();
}

// Scale every NNpx value inside a stylesheet string to match the screen size.
inline QString qss(const QString &stylesheet) {
    static const QRegularExpression re("(\\d+(?:\\.\\d+)?)px");
    QString result;
    qsizetype lastEnd = 0;
    auto it = re.globalMatch(stylesheet);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        result += stylesheet.mid(lastEnd, match.capturedStart() - lastEnd);
        double value = match.captured(1).toDouble();
        result += QString::number(value * scaleFactor()) + "px";
        lastEnd = match.capturedEnd();
    }
    result += stylesheet.mid(lastEnd);
    return result;
}

// Scale every NNpx value inside arbitrary text (used for the global theme file).
inline QString scalePx(const QString &text) {
    return qss(text);
}

}

#endif // SCREENUTILS_H

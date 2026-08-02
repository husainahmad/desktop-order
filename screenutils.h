#ifndef SCREENUTILS_H
#define SCREENUTILS_H

#include <QGuiApplication>
#include <QScreen>
#include <QRect>
#include <QSize>
#include <QtGlobal>

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

}

#endif // SCREENUTILS_H

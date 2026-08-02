#ifndef TOUCHUTILS_H
#define TOUCHUTILS_H

#include <QAbstractScrollArea>
#include <QAbstractItemView>
#include <QScroller>
#include <QScrollerProperties>

namespace TouchUtils {

inline void enableTouchScrolling(QAbstractScrollArea *area) {
    if (!area) return;
    area->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(area->viewport(), QScroller::TouchGesture);
}

inline void enableItemViewScrolling(QAbstractItemView *view) {
    if (!view) return;
    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
    QScroller::grabGesture(view->viewport(), QScroller::TouchGesture);
}

}

#endif // TOUCHUTILS_H

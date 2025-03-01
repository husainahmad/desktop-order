#include "toast.h"

Toast::Toast(QWidget *parent, const QString &message, int duration)
    : QLabel(parent) {
    setText(message);
    setStyleSheet("background-color: rgba(0, 0, 0, 180); color: white; padding: 10px; border-radius: 5px;");
    setAlignment(Qt::AlignCenter);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    adjustSize();

    // Center the toast on parent
    if (parent) {
        move(parent->width() / 2 - width() / 2, parent->height() - height() - 50);
    }

    show();

    // Fade-out animation
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(500);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    animation->setEasingCurve(QEasingCurve::OutQuad);

    // Hide toast after duration
    QTimer::singleShot(duration, [=]() {
        animation->start();
        connect(animation, &QPropertyAnimation::finished, this, &QLabel::deleteLater);
    });
}

// Helper function to show toast easily
void showToast(QWidget *parent, const QString &message, int duration) {
    new Toast(parent, message, duration);
}

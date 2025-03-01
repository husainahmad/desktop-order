#ifndef TOAST_H
#define TOAST_H

#include <QLabel>
#include <QTimer>
#include <QPropertyAnimation>

class Toast : public QLabel {
    Q_OBJECT
public:
    explicit Toast(QWidget *parent, const QString &message, int duration = 3000);
};

// Helper function to show a toast message easily
void showToast(QWidget *parent, const QString &message, int duration = 3000);
#endif // TOAST_H

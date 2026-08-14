#ifndef BUSYINDICATOR_H
#define BUSYINDICATOR_H

#include <QWidget>
#include <QTimer>
#include <QPointer>

class QLabel;

class BusyIndicator : public QWidget
{
    Q_OBJECT

public:
    static void show();
    static void hide();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    explicit BusyIndicator(QWidget *parent);

    void ensureParent();
    void updateOverlayGeometry();
    void rotate();
    void paintSpinner();

    static QPointer<BusyIndicator> s_instance;
    static int s_refCount;

    QTimer m_timer;
    int m_angle = 0;
    QWidget *m_card;
    QLabel *m_spinner;
    QLabel *m_text;
};

#endif // BUSYINDICATOR_H

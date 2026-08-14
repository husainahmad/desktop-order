#include "busyindicator.h"

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QEvent>
#include <QScreen>

QPointer<BusyIndicator> BusyIndicator::s_instance;
int BusyIndicator::s_refCount = 0;

BusyIndicator::BusyIndicator(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setObjectName("busyOverlay");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    m_card = new QWidget(this);
    m_card->setObjectName("busyCard");
    m_card->setFixedSize(180, 140);

    QVBoxLayout *cardLayout = new QVBoxLayout(m_card);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setSpacing(12);

    m_spinner = new QLabel(m_card);
    m_spinner->setFixedSize(48, 48);
    m_spinner->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(m_spinner);

    m_text = new QLabel("Processing...", m_card);
    m_text->setObjectName("busyText");
    m_text->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(m_text);

    m_card->setLayout(cardLayout);
    layout->addWidget(m_card);
    setLayout(layout);

    connect(&m_timer, &QTimer::timeout, this, &BusyIndicator::rotate);
}

void BusyIndicator::show()
{
    if (!s_instance) {
        s_instance = new BusyIndicator(nullptr);
    }

    s_instance->ensureParent();
    ++s_refCount;

    s_instance->updateOverlayGeometry();
    s_instance->m_timer.start(40);
    s_instance->paintSpinner();
    s_instance->QWidget::show();
    s_instance->raise();
}

void BusyIndicator::hide()
{
    if (!s_instance) {
        return;
    }

    --s_refCount;
    if (s_refCount <= 0) {
        s_refCount = 0;
        s_instance->m_timer.stop();
        s_instance->QWidget::hide();
    }
}

void BusyIndicator::ensureParent()
{
    QWidget *window = QApplication::activeWindow();
    if (!window) {
        for (QWidget *widget : QApplication::topLevelWidgets()) {
            if (widget->isVisible()) {
                window = widget;
                break;
            }
        }
    }
    if (!window) {
        return;
    }

    if (parentWidget() != window) {
        setParent(window);
        window->installEventFilter(this);
    }
    updateOverlayGeometry();
}

void BusyIndicator::updateOverlayGeometry()
{
    if (parentWidget()) {
        setGeometry(parentWidget()->rect());
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            setGeometry(screen->availableGeometry());
        }
    }
}

bool BusyIndicator::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
        updateOverlayGeometry();
    }
    return QWidget::eventFilter(watched, event);
}

void BusyIndicator::rotate()
{
    m_angle = (m_angle + 30) % 360;
    paintSpinner();
}

void BusyIndicator::paintSpinner()
{
    QPixmap pixmap(48, 48);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(QColor(226, 232, 240), 5, Qt::SolidLine, Qt::RoundCap));
    painter.drawEllipse(6, 6, 36, 36);

    painter.setPen(QPen(QColor(37, 99, 235), 5, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(6, 6, 36, 36, m_angle * 16, 300 * 16);

    m_spinner->setPixmap(pixmap);
}

void BusyIndicator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), QColor(15, 23, 42, 110));
}

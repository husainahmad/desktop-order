#ifndef ORDERTABBUTTON_H
#define ORDERTABBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QTabBar>
#include "screenutils.h"

class OrderTabButton : public QWidget
{
    Q_OBJECT
public:
    explicit OrderTabButton(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QVBoxLayout *v = new QVBoxLayout(this);
        v->setContentsMargins(ScreenUtils::px(10), ScreenUtils::px(3), ScreenUtils::px(6), ScreenUtils::px(3));
        v->setSpacing(ScreenUtils::px(1));

        QHBoxLayout *h = new QHBoxLayout();
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(ScreenUtils::px(8));

        titleLabel = new QLabel(this);
        titleLabel->setObjectName("tabTitleLabel");
        h->addWidget(titleLabel);
        h->addStretch();

        closeButton = new QPushButton(QStringLiteral("\u2715"), this);
        closeButton->setObjectName("tabCloseButton");
        closeButton->setFixedSize(ScreenUtils::px(22), ScreenUtils::px(22));
        h->addWidget(closeButton, 0, Qt::AlignTop);

        v->addLayout(h);

        subtitleLabel = new QLabel(this);
        subtitleLabel->setObjectName("tabSubtitleLabel");
        v->addWidget(subtitleLabel);

        setMouseTracking(true);
    }

    void setTitle(const QString &text) { titleLabel->setText(text); }
    void setSubtitle(const QString &text) { subtitleLabel->setText(text); }
    void setSelectable(bool selectable) { m_selectable = selectable; }

    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QPushButton *closeButton;

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            emit clicked();
            if (m_selectable) {
                QTabBar *bar = qobject_cast<QTabBar *>(parentWidget());
                if (bar) {
                    for (int i = 0; i < bar->count(); ++i) {
                        if (bar->tabButton(i, QTabBar::LeftSide) == this) {
                            bar->setCurrentIndex(i);
                            break;
                        }
                    }
                }
            }
            event->accept();
            return;
        }
        QWidget::mousePressEvent(event);
    }

private:
    bool m_selectable = true;
};

#endif // ORDERTABBUTTON_H

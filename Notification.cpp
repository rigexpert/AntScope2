#include "Notification.h"

#include <QTimer>
#include <QLabel>
#include <QFont>
#include <QPainter>
#include <QPaintEvent>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDebug>

namespace
{
    const QColor FILL_COLOR(66, 68, 70);
    const QColor TEXT_COLOR(Qt::red);

    const QFont FONT("MS Sans Serif", 14);
    const int DISPLAY_TIME = 1500;
}


Notification::Notification(const QString& text, QWidget* parent)
    : Notification(text, FONT, DISPLAY_TIME, parent)
{}

Notification::Notification(const QString& text, const QFont& font, int milliseconds, QWidget* parent)
    : QWidget(parent)
    , m_label(text)
    , m_opacity(.5)
    , m_milliseconds(milliseconds)
{
    setFont(font);
    m_textColor = TEXT_COLOR;
    m_label.prepare(QTransform(), font);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    if (parent)
    {
        QPoint offset(m_label.size().width() / 2 + 20, m_label.size().height() / 2 + 20);
        setGeometry(QRect(parent->rect().center() - offset, parent->rect().center() + offset));
    }
    else
        resize(270, 80);
}

Notification::~Notification()
{}

void Notification::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!m_url.isEmpty())
        {
            QDesktopServices::openUrl(QUrl::fromUserInput(m_url));
            event->accept();
        }
    } else {
        QWidget::mouseReleaseEvent(event);
    }
    fadeOut();
}

void Notification::showImmediatly()
{
    show();
    QCoreApplication::processEvents();
}

void Notification::run()
{
    show();
    update();
    QTimer::singleShot(m_milliseconds, this, &Notification::fadeOut);
}

void Notification::fadeOut()
{
    m_animation = new QPropertyAnimation(this, "opacity", this);
    connect(m_animation, &QPropertyAnimation::finished, this, &Notification::deleteLater);

    m_animation->setDuration(500);
    m_animation->setStartValue(opacity());
    m_animation->setEndValue(0.);
    m_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Notification::setOpacity(qreal opacity)
{
    m_opacity = opacity;
    update();
}

qreal Notification::opacity() const
{
    return m_opacity;
}

void Notification::showMessage(const QString& message, QColor textColor, QRect rect, int milliseconds, QWidget* parent)
{
    Notification* notification = new Notification(message, FONT, milliseconds, parent);
    notification->setTextColor(textColor);
    notification->setGeometry(rect);
    notification->run();
}

void Notification::showMessage(const QString& message, const QFont& font, int milliseconds, QWidget* parent)
{
    (new Notification(message, font, milliseconds, parent))->run();
}

void Notification::showMessage(const QString& message, QWidget* parent)
{
    showMessage(message, FONT, parent);
}

void Notification::showMessage(const QString& message, const QFont& font, QWidget* parent)
{
    showMessage(message, font, DISPLAY_TIME, parent);
}

void Notification::showMessage(const QString& message, QString& url, QRect rect, int milliseconds, QWidget* parent)
{
    Notification* notification = new Notification(message, FONT, milliseconds, parent);
    notification->m_url = url;
    notification->setGeometry(rect);
    notification->run();
}

void Notification::paintEvent(QPaintEvent* event)
{
    QPainter p(this);

    p.setOpacity(m_opacity);
    p.fillRect(event->rect(), FILL_COLOR);
    p.setPen(m_textColor);
    p.drawRect(event->rect().adjusted(0, 0, -1, -1));
    p.setFont(font());

    QSize halfSize = m_label.size().toSize() / 2;
    p.drawStaticText(rect().center() - QPoint(halfSize.width(), halfSize.height()), m_label);
}


#include "popupindicator.h"

PopUpIndicator* PopUpIndicator::m_popUpIndicator = nullptr;

PopUpIndicator::PopUpIndicator(QWidget *parent)
    : PopUp(parent)
{
    m_popUpIndicator = this;
    setBackgroundColor(Qt::red);
    setPenColor(Qt::red);
}

void PopUpIndicator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

//    QRect roundedRect;
//    roundedRect.setX(rect().x() + 5);
//    roundedRect.setY(rect().y() + 5);
//    roundedRect.setWidth(rect().width() - 10);
//    roundedRect.setHeight(rect().height() - 10);
    painter.setBrush(QBrush(Qt::red));
    painter.setPen(Qt::red);

    int R = rect().width()/2-2;
    painter.drawEllipse(rect().center(), R, R);
}

void PopUpIndicator::showIndicator(QWidget* parent)
{
    if (m_popUpIndicator == nullptr)
        m_popUpIndicator = new PopUpIndicator(parent);
    QPoint pt = m_popUpIndicator->parentWidget()->mapToGlobal(QPoint(10, 10));
    m_popUpIndicator->move(pt.x(), pt.y());
    m_popUpIndicator->show();
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void PopUpIndicator::hideIndicator(QWidget* parent)
{
    if (m_popUpIndicator == nullptr)
        m_popUpIndicator = new PopUpIndicator(parent);
    QApplication::restoreOverrideCursor();
    m_popUpIndicator->hideAnimation();
}

void PopUpIndicator::setIndicatorVisible(bool visible)
{
    if (visible)
        showIndicator();
    else
        hideIndicator();
}

void PopUpIndicator::show()
{
    setWindowOpacity(0.0);

    animation.setDuration(150);
    animation.setStartValue(0.0);
    animation.setEndValue(1.0);

    QWidget::show();

    animation.start();
}

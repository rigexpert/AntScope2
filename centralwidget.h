#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <QPainter>

class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        qreal ht = height();
        QRadialGradient gradient(width()/2, ht-15, width()/2);

        gradient.setColorAt(0.7, QColor("#020202"));
        gradient.setColorAt(0.3, QColor("#138fc3"));

        painter.fillRect(rect(), gradient);
    }

signals:
};

#endif // CENTRALWIDGET_H

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QDateTime>
#include "qcustomplot.h"
#include "CustomPlot.h"

class GLWidget : public QWidget
{
    Q_OBJECT

public:
    GLWidget(QCustomPlot* _plotter, QWidget *parent = 0);
    GLWidget(QWidget *parent = 0);
    virtual ~GLWidget();
    QCustomPlot* plotter() { return m_plotter; }
    void setPlotter(QCustomPlot* _plotter);

public slots:
    void replot();
    void onMouseWheelEvent(QWheelEvent*);
    void onMouseMoveEvent(QMouseEvent*);

private:
    QCustomPlot* m_plotter=nullptr;
    QDateTime m_prevDateTime;
    qint64 m_repaintTimeMsec=0;

protected:
    void paintEvent(QPaintEvent *event);

};

#endif // GLWIDGET_H

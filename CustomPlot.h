#ifndef CUSTOMPLOT_H
#define CUSTOMPLOT_H

#include "qcustomplot.h"
#include "customgraph.h"

class CustomPlot : public QCustomPlot
{
    Q_OBJECT

    bool m_incrementalDraw = false;
    int m_graphCount;
    QRect m_clipRect;
    QMutex m_drawMutex;

public:
    explicit CustomPlot(int _numGraphs, QWidget *parent = 0);
    QPixmap& paintBuffer() { return mPaintBuffer; }
    QCPGraph *addGraph(QCPAxis *keyAxis=0, QCPAxis *valueAxis=0);
    CustomGraph *graph(int index) const;
    CustomGraph *graph() const;
    void setIncremental(bool _mode);
    void setGraphCount(int _count) { m_graphCount = _count; }
    QRect getClipRect() { return m_clipRect; }
    virtual void drawIncrementally();

protected:
    virtual void paintEvent(QPaintEvent *event);
};

#endif // CUSTOMPLOT_H

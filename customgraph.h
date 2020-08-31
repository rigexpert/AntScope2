#ifndef CUSTOMGRAPH_H
#define CUSTOMGRAPH_H

#include <QObject>
#include "qcustomplot.h"

class CustomGraph : public QCPGraph
{
    Q_OBJECT

    QVector<QPointF> m_prevLineCoords;
    bool m_checked = true;

public:
    explicit CustomGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);

    bool checked() const { return m_checked; }
    void setChecked(bool _state) { m_checked = _state; }

    QRectF getClipRect();
    QRectF getClipRect(QVector<QPointF>& _lineCoords);

    void drawLine(QCPPainter *painter);
    void restoreLine(QCPPainter *painter);
    void getLineData(QVector<QPointF> *linePixelData) const;

protected:
    virtual void draw(QCPPainter *painter);
};

#endif // CUSTOMGRAPH_H

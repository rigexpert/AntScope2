#ifndef CUSTOMGRAPH_H
#define CUSTOMGRAPH_H

#include <QObject>
#include "qcustomplot.h"

class CustomGraph : public QCPGraph
{
    Q_OBJECT

    QVector<QPointF> m_prevLineCoords;

public:
    explicit CustomGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);

    QRectF getClipRect();
    QRectF getClipRect(QVector<QPointF>& _lineCoords);

    void drawLine(QCPPainter *painter);
    void restoreLine(QCPPainter *painter);
    void getLineData(QVector<QPointF> *linePixelData) const;
};

#endif // CUSTOMGRAPH_H

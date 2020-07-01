#include "customgraph.h"

CustomGraph::CustomGraph(QCPAxis *keyAxis, QCPAxis *valueAxis)
    : QCPGraph(keyAxis, valueAxis)
{
    m_prevLineCoords << QPointF(0, 0) << QPointF(0, 0);
}

void CustomGraph::drawLine(QCPPainter *painter)
{
    int len = data()->size();
    if (len < 2)
        return;

    LineStyle style = mLineStyle;
    mLineStyle = QCPGraph::lsLine;

    // fill vectors with data appropriate to plot style:
    QVector<QPointF> *lineCoords = new QVector<QPointF>;
    getPlotData(lineCoords, nullptr);

    if (len != lineCoords->size()) {
        delete lineCoords;
        return; // HUCK for wheel & mouse events
    }
    QVector<QPointF> _drawCoords;
    _drawCoords << lineCoords->at(len-2) << lineCoords->at(len-1);

    painter->save();
    painter->setClipRect(getClipRect(_drawCoords));

    drawLinePlot(painter, &_drawCoords);
    if (name() == "Graph 1")
    {
        m_prevLineCoords[0] = _drawCoords[0];
        m_prevLineCoords[1] = _drawCoords[1];
    }
    painter->restore();

    delete lineCoords;
    mLineStyle = style;
}

void CustomGraph::restoreLine(QCPPainter *painter)
{
    if (name() != "Graph 1")
        return;
    painter->save();
    painter->setClipRect(getClipRect(m_prevLineCoords));
    drawLinePlot(painter, &m_prevLineCoords);
    painter->restore();
}

QRectF CustomGraph::getClipRect()
{
    QRectF _clip;
    if (data()->size() > 1) {
        QCPDataMap::iterator iter = data()->end();
        QCPData _last = *(--iter);
        QCPData _prev = *(--iter);
        QPointF _ptPrev = coordsToPixels(_prev.key, _prev.value);
        QPointF _ptLast = coordsToPixels(_last.key, _last.value);
        QRectF _clipRect(_ptPrev, _ptLast);
        return _clipRect.normalized().adjusted(-2, -2, 2, 2);
    }
    return _clip;
}

QRectF CustomGraph::getClipRect(QVector<QPointF>& _lineCoords)
{
    QRectF _clip;
    if (_lineCoords.size() > 1) {
        QRectF _clipRect(_lineCoords[0], _lineCoords[1]);
        return _clipRect.normalized().adjusted(-2, -2, 2, 2);
    }
    return _clip;
}




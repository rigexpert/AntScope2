#include "CustomPlot.h"


CustomPlot::CustomPlot(int _numGraphs, QWidget *parent)
    : m_graphCount(_numGraphs), QCustomPlot(parent)
{
}

QCPGraph *CustomPlot::addGraph(QCPAxis *keyAxis, QCPAxis *valueAxis)
{
  if (!keyAxis) keyAxis = xAxis;
  if (!valueAxis) valueAxis = yAxis;
  if (!keyAxis || !valueAxis)
  {
    qDebug() << Q_FUNC_INFO << "can't use default QCustomPlot xAxis or yAxis, because at least one is invalid (has been deleted)";
    return 0;
  }
  if (keyAxis->parentPlot() != this || valueAxis->parentPlot() != this)
  {
    qDebug() << Q_FUNC_INFO << "passed keyAxis or valueAxis doesn't have this QCustomPlot as parent";
    return 0;
  }

  QCPGraph *newGraph = new CustomGraph(keyAxis, valueAxis);
  if (addPlottable(newGraph))
  {
    newGraph->setName(QLatin1String("Graph ")+QString::number(mGraphs.size()));
    return newGraph;
  } else
  {
    delete newGraph;
    return 0;
  }
}

CustomGraph *CustomPlot::graph(int index) const
{
    return (CustomGraph*)QCustomPlot::graph(index);
}

void CustomPlot::setIncremental(bool _mode)
{
    m_incrementalDraw = _mode;
}


CustomGraph *CustomPlot::graph() const
{
    return (CustomGraph*)QCustomPlot::graph();
}

void CustomPlot::drawIncrementally()
{
    QCPPainter painter;
    painter.begin(&paintBuffer());
    painter.save();

    CustomGraph* _graph0 = graph(0);
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    _graph0->restoreLine(&painter);
    painter.restore();

    QRectF _clip;
    for (int i=m_graphCount; i>0; i--)
    {
        CustomGraph* _graph = graph(graphCount()-i);
        _graph->drawLine(&painter);
        QRectF r = _graph->getClipRect();
        if (i == m_graphCount)
            _clip = r;
        else
            _clip = _clip.united(r);
    }

    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    _graph0->drawLine(&painter);
    _clip = _clip.united(_graph0->getClipRect());
    m_clipRect = QRect(_clip.x(), _clip.y(), _clip.width(), _clip.height());

    painter.end();
    update(m_clipRect);
}


void CustomPlot::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter(this);
  painter.drawPixmap(event->rect().topLeft(), mPaintBuffer, event->rect());
}


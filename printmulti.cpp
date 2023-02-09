#include "printmulti.h"
#include "ui_print.h"

Printmulti::Printmulti(const QList<QString>& tabs, QWidget *parent)
    : Print(parent)
{
    ui->widgetsLayout->removeWidget(ui->widgetGraph);
    ui->widgetGraph->hide();

    foreach (const QString& tab, tabs) {
        m_tabNames << tab;
        QCustomPlot* plot = new QCustomPlot();
        m_mapTabPlots.insert(tab, plot);
        QFont font = ui->widgetGraph->xAxis->tickLabelFont();
        font.setPointSize(12);
        plot->xAxis->setTickLabelFont(font);
        plot->yAxis->setTickLabelFont(font);

        font.setPointSize(14);
        plot->xAxis->setLabelFont(font);
        plot->yAxis->setLabelFont(font);
        plot->legend->setVisible(true);

        ui->widgetsLayout->addWidget(plot);
    }
}

void Printmulti::setData(const QString& tab, QCPDataMap *m, QPen pen, QString name)
{
    QCustomPlot* plot = m_mapTabPlots[tab];
    plot->addGraph();

    plot->graph()->setData(m,true);
    plot->graph()->setPen(pen);
    plot->graph()->setName(name);

    QPen gridPen = plot->xAxis->grid()->pen();
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(QColor(0, 0, 0, 255));
    plot->xAxis->grid()->setPen(gridPen);
    plot->yAxis->grid()->setPen(gridPen);

    m_isSmithGraph = false;
    rescale();
}

void Printmulti::addMarker(double fq, int number)
{
    m_mFqList.append(fq);

    auto keys = m_mapTabPlots.keys();
    foreach (auto key, keys) {
        QCustomPlot* plot = m_mapTabPlots[key];
        QCPItemStraightLine *line = new QCPItemStraightLine(plot);
        QCPItemText *text = new QCPItemText(plot);
        line->setAntialiased(false);
        plot->addItem(line);
        plot->addItem(text);
        line->setPen(QPen(QColor(255,0,0,150)));
        text->setColor(QColor(255, 0, 0, 150));

        line->point1->setCoords(fq, -2000);
        line->point2->setCoords(fq, 2000);

        text->setText(QString::number(number));
        m_mStraightLineList.append(line);
        m_mTextList.append(text);
    }

    rescale();
}

void Printmulti::setRange(const QString& tab, QCustomPlot* _plot)
{
    QCPRange x = _plot->xAxis->range();
    QCPRange y = _plot->yAxis->range();
    QCustomPlot* plot = m_mapTabPlots[tab];
    plot->xAxis->setRangeMin(x.lower);
    plot->xAxis->setRangeMax(x.upper);

    plot->yAxis->setRangeMin(y.lower);
    plot->yAxis->setRangeMax(y.upper);

    plot->xAxis->setRange(x);
    plot->yAxis->setRange(y);
}

void Printmulti::addBand (double x1, double x2, double y1, double y2)
{
    auto keys = m_mapTabPlots.keys();
    foreach (auto key, keys) {
        QCustomPlot* plot = m_mapTabPlots[key];
        Print::addBand(x1, x2, y1, y2, plot);
    }
}

void Printmulti::rescale()
{
    auto keys = m_mapTabPlots.keys();
    foreach (auto key, keys) {
        QCustomPlot* plot = m_mapTabPlots[key];
        for(int i = 0; i < m_mTextList.length(); ++i)
        {
            double offsetX = (plot->xAxis->range().upper - plot->xAxis->range().lower)/40;
            double offsetY = (plot->yAxis->range().upper - plot->yAxis->range().lower)/10;

            m_mTextList.at(i)->position->setCoords(m_mFqList.at(i) + offsetX, plot->yAxis->range().center()-offsetY);
        }
        plot->replot();
    }
}

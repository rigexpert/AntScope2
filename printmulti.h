#ifndef PRINTMULTI_H
#define PRINTMULTI_H

#include "print.h"

class Printmulti : public Print
{
    Q_OBJECT
public:
    Printmulti(const QList<QString>& tabs, QWidget *parent = 0);
    void setData(const QString& tab, QCPDataMap *m, QPen pen, QString name);
    virtual void addMarker(double fq, int number);
    //virtual void setRange(QCPRange x, QCPRange y);
    void setRange(const QString& tab, QCustomPlot* plot);
    virtual void addBand (double x1, double x2, double y1, double y2);
    virtual void rescale();

protected:
    QList<QString> m_tabNames;
    QMap<QString, QCustomPlot*> m_mapTabPlots;
};

#endif // PRINTMULTI_H

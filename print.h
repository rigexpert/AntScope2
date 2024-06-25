#ifndef PRINT_H
#define PRINT_H

#include <QDialog>
#include <analyzer/analyzerparameters.h>
#include <markers.h>
#include <QSettings>
#include <settings.h>
#include "printmarkers.h"

namespace Ui {
class Print;
}

class Print : public QDialog
{
    Q_OBJECT

public:
    explicit Print(QWidget *parent = 0);
    ~Print();
    virtual void addMarker(double fq, int number);

    //virtual void setRange(QCPRange x, QCPRange y);
    virtual void setRange(QCustomPlot* plot);
    void setRange_yAxis2(QCPRange range);
    void setLabel(QString xLabel, QString yLabel);
    void setData(QCPDataMap *m, QPen pen, QString name);
    void setSmithData(QCPCurveDataMap *map, QPen pen, QString name);
    void setName(QString name) { m_graphName = name; }
    void drawBands(QStringList* _bands, double y1, double y2);
    virtual void addBand (double x1, double x2, double y1, double y2);
    void addBand (double x1, double x2, double y1, double y2, QCustomPlot* plot);
    void setHead(QString string);

    void updateTable();
    void drawSmithImage (void);

    virtual void rescale();
    void updateMarkers(int markers, int measurements, QList<QList<QVariant>> info);

protected:
    void resizeEvent(QResizeEvent *e);

protected slots:
    void on_lineSlider_valueChanged(int value);
    void on_printBtn_clicked();
    void on_pdfPrintBtn_clicked();
    void on_pngPrintBtn_clicked();

protected:
    Ui::Print *ui;
    QSettings *m_settings;
    QString m_lastPath;

    QVector <double> m_mFqList;
    QVector <QCPCurve*> m_curveList;
    QVector <QCPItemStraightLine*> m_mStraightLineList;
    QVector <QCPItemText*> m_mTextList;
    QVector <QCPCurveDataMap*> m_curveDataList;
    QVector <QCPItemText*> m_textList;

    bool m_isSmithGraph;
    QString m_graphName;
};

#endif // PRINT_H

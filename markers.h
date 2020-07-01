#ifndef MARKERS_H
#define MARKERS_H

#include <QObject>
#include <qcustomplot.h>
#include <popup.h>
#include <markerspopup.h>
#include <QSettings>
#include <analyzer/analyzerparameters.h>
#include <settings.h>
#include <measurements.h>

struct marker
{
    double frequency;
    QCPItemStraightLine *swrLine = NULL;//QCPItemTracer *swrTracer;
    QCPItemStraightLine *phaseLine = NULL;
    QCPItemStraightLine *rsLine = NULL;
    QCPItemStraightLine *rpLine = NULL;
    QCPItemStraightLine *rlLine = NULL;
//    QCPItemStraightLine *smithTracer = NULL;
    QCPItemText *swrLineText = NULL;
    QCPItemText *phaseLineText = NULL;
    QCPItemText *rsLineText = NULL;
    QCPItemText *rpLineText = NULL;
    QCPItemText *rlLineText = NULL;
//    QCPItemText *smithTracerText = NULL;

    void clear()
    {
        if(swrLine)
            delete swrLine;
        if(phaseLine)
            delete phaseLine;
        if(rsLine)
            delete rsLine;
        if(rpLine)
            delete rpLine;
        if(rlLine)
            delete rlLine;
        if(swrLineText)
            delete swrLineText;
        if(phaseLineText)
            delete phaseLineText;
        if(rsLineText)
            delete rsLineText;
        if(rpLineText)
            delete rpLineText;
        if(rlLineText)
            delete rlLineText;
    }
};

class Markers : public QObject
{
    Q_OBJECT
public:
    explicit Markers(QObject *parent = 0);
    ~Markers();

    void setWidgets(QCustomPlot * swr, QCustomPlot * phase, QCustomPlot * rs, QCustomPlot * rp,
                    QCustomPlot * rl, QCustomPlot * tdr, QCustomPlot * smith);
    void setMeasurements(Measurements *m);
    void create(double fq);
    void setFq(double fq);
    void add();
    bool getMarkersHintEnabled(void);
    void saveBmp(QString path);
    QString getMarkersHintText();
    QList <QStringList> getMarkersHintList();
    qint32 getMarkersCount();
    marker getMarker( quint32 number);
    void repaint();
    void on_translate();
    void changeColorTheme(bool _dark);

private:
    QCustomPlot *m_swrWidget;
    QCustomPlot *m_phaseWidget;
    QCustomPlot *m_rsWidget;
    QCustomPlot *m_rpWidget;
    QCustomPlot *m_rlWidget;
    QCustomPlot *m_tdrWidget;
    QCustomPlot *m_smithWidget;

    QVector <marker*> m_markersList;

    MarkersPopUp * m_markersHint;

    QString m_currentTab;

    bool m_markersHintEnabled;

    QSettings * m_settings;

    Measurements *m_measurements;

    bool m_focus;

    double interpolate(double fq1, double fq2, double fq3, double param1, double param2);
signals:

public slots:
    void on_focus(bool focus);
    void on_mainWindowPos(int x, int y);
    void on_currentTab(QString name);
    void on_newMeasurement(QString);
    void on_measurementComplete();
    void setMarkersHintEnabled(bool enabled);
    void redraw(void);
    void rescale();
    void on_removeMarker(int number);
};

#endif // MARKERS_H

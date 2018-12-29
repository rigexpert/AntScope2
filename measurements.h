#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include <QObject>
#include <QVector>
#include <math.h>
#include <qdebug.h>
#include <qcustomplot.h>
#include <analyzer/analyzerparameters.h>
#include <popup.h>
#include <QSettings>
#include <calibration.h>
#include <ctime>
#include <complex>
#include <settings.h>

#define MAX_MEASUREMENTS 5
#define TDR_MAXARRAY 20000

#define SPEEDOFLIGHT 299792458.0
#define FEETINMETER 3.2808399
#ifndef DBL_MAX
#define DBL_MAX 1.797693134862315e+308
#endif
typedef std::complex <double> Complex;

class Measurements : public QObject
{
    Q_OBJECT
public:
    explicit Measurements(QObject *parent = 0);
    ~Measurements();

    void setWidgets(QCustomPlot * swr, QCustomPlot * phase, QCustomPlot * rs, QCustomPlot * rp,
                    QCustomPlot * rl, QCustomPlot * tdr, QCustomPlot * smith, QTableWidget *table);
    void setCalibration(Calibration * _calibration);
    bool getCalibrationEnabled(void);
    void deleteRow(int row);
    qint32 getFarEndMeasurement (void) {return m_farEndMeasurement;}
    measurement* getMeasurement(int number) {return &m_measurements[m_measurements.length()-1 - number];}
    measurement* getMeasurementSub(int number) {return &m_farEndMeasurementsSub[m_farEndMeasurementsSub.length()-1 - number];}
    measurement* getMeasurementAdd(int number) {return &m_farEndMeasurementsAdd[m_farEndMeasurementsAdd.length()-1 - number];}
    qint32 getMeasurementLength(void) {return m_measurements.length();}
    bool isEmpty() { return getMeasurementLength() == 0; }
    bool getGraphHintEnabled(void);
    void saveData(quint32 number, QString path);
    void loadData(QString path);

    void exportData(QString _name, int _type, int _number);
    void importData(QString _name);

    double getZ0(void) const{ return m_Z0;}
    void setZ0(double _Z0) { m_Z0 = _Z0;}

    int CalcTdr(QVector<rawData> *data);
    void FFT(float real[], float imag[], int length, int Inverse = 0);

    int CalcTdr2(QVector <rawData> *data);
    qint16 DTF_FindRadix2Length(qint16 length, int *log2N);
    void FFT2(double *Rdat, double *Idat, int N, int LogN, int Ft_Flag);

    void setCableVelFactor(double value);
    void setCableResistance(double value);
    void setCableLossConductive(double value);
    void setCableLossDielectric(double value);
    void setCableLossFqMHz(double value);
    void setCableLossUnits(int value);
    void setCableLossAtAnyFq(bool value);
    void setCableLength(double value);
    void setCableFarEndMeasurement(int value);
    void on_translate();

private:
//    QVector <rawData> m_rawDataVector;
//    QVector < QVector<rawData> > m_rawDataLists;
    QVector <QString> m_tableNames;

    QString m_currentTab;

//    measurement m_measurements[MAX_MEASUREMENTS];
    QList <measurement> m_measurements;
    QList <measurement> m_viewMeasurements;
    QList <measurement> m_farEndMeasurementsAdd;
    QList <measurement> m_farEndMeasurementsSub;

    QCustomPlot *m_swrWidget;
    QCustomPlot *m_phaseWidget;
    QCustomPlot *m_rsWidget;
    QCustomPlot *m_rpWidget;
    QCustomPlot *m_rlWidget;
    QCustomPlot *m_tdrWidget;
    QCustomPlot *m_smithWidget;
    QTableWidget *m_tableWidget;

    qint32 m_currentIndex;

    PopUp *m_graphHint;
    PopUp *m_graphBriefHint;

    QCPItemStraightLine *m_swrLine;
    QCPItemStraightLine *m_swrLine2;
    QCPItemStraightLine *m_phaseLine;
    QCPItemStraightLine *m_phaseLine2;
    QCPItemStraightLine *m_rsLine;
    QCPItemStraightLine *m_rpLine;
    QCPItemStraightLine *m_rlLine;
    QCPItemStraightLine *m_rlLine2;
    QCPItemStraightLine *m_tdrLine;

    QSettings * m_settings;
    Calibration * m_calibration;

    bool m_graphHintEnabled;
    bool m_graphBriefHintEnabled;

    volatile bool m_calibrationMode;

    double m_Z0;

    double *m_pdTdrImp;
    double *m_pdTdrStep;

    double m_tdrResolution;
    double m_tdrRange;

    qint32 m_dotsNumber;

//    bool m_calibrationEnabled;
    bool m_measureSystemMetric;

    double m_cableVelFactor;
    double m_cableResistance;
    double m_cableLossConductive;
    double m_cableLossDielectric;
    double m_cableLossFqMHz;
    qint32 m_cableLossUnits;
    qint32 m_cableLossAtAnyFq;
    double m_cableLength;
    qint32 m_farEndMeasurement;
    QCPItemEllipse * m_smithTracer;

    bool m_focus;

    quint32 computeSWR(double freq, double Z0, double R, double X, double *VSWR, double *RL);
    double computeZ (double R, double X);

    void NormRXtoSmithPoint(double Rnorm, double Xnorm, double &x, double &y);    
    void drawSmithImage(void);
    void calcFarEnd(void);
signals:
    void calibrationChanged();
    void import_finished(double _fqMin_khz, double _fqMax_khz);

public slots:
    void on_newData(rawData _rawData);
    void on_newMeasurement(QString name);
    void on_newMeasurement(QString name, qint64 fq, qint64 sw, qint64 dots);
    void on_currentTab(QString);
    void on_focus(bool focus);
    void hideGraphBriefHint();
    void showHideHints();
    void on_newCursorFq(double x, int number, int mouseX, int mouseY);
    void on_newCursorSmithPos (double x, double y, int number);
    void updatePopUp(double xPos, int number, int mouseX, int mouseY);
    void on_mainWindowPos(int x, int y);
    void setGraphHintEnabled(bool enabled);
    void setGraphBriefHintEnabled(bool enabled);
    void setCalibrationMode(bool enabled);
    void on_calibrationEnabled(bool enabled);
    void on_dotsNumberChanged(int number);
    void on_redrawGraphs();
    void on_changeMeasureSystemMetric (bool state);
    void replot();
};

#endif // MEASUREMENTS_H

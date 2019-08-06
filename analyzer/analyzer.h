#ifndef ANALYZER_H
#define ANALYZER_H

#include <QObject>
#include <QDateTime>
#include <analyzer/comanalyzer.h>
#include <analyzer/hidanalyzer.h>
#include <math.h>
#include "analyzerparameters.h"

#include <analyzer/analyzerparameters.h>
#include <analyzer/updater/downloader.h>
#include <devinfo/redeviceinfo.h>
#include <updatedialog.h>
#include <crc32.h>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

class Analyzer : public QObject
{
    QMap<QString, QString> m_mapFullInfo;

    Q_OBJECT
public:
    explicit Analyzer(QObject *parent = nullptr);
    ~Analyzer();

    QString getModelString(void);
    quint32 getModel(void);
    quint32 getHidModel( void );
    void updateFirmware (QIODevice *fw);
    double getVersion() const;
    bool checkFile(QString path);
    bool openComPort(const QString& portName, quint32 portSpeed);
    void closeComPort();

    void setIsMeasuring (bool _isMeasuring);

    void setContinuos(bool isContinuos)
    {
        m_isContinuos = isContinuos;
        if(m_comAnalyzer != nullptr)
        {
            m_comAnalyzer->setContinuos(isContinuos);
        }
        if(m_hidAnalyzer != nullptr)
        {
            m_hidAnalyzer->setContinuos(isContinuos);
        }
    }
    bool isMeasuring (void) const { return m_isMeasuring;}

    hidAnalyzer * getHidAnalyzer() { return m_hidAnalyzer; }
    comAnalyzer * getComAnalyzer() { return m_comAnalyzer; }

    void setAnalyzerModel (int model);
    quint32 getAnalyzerModel (void) const { return m_analyzerModel;}

    void setComAnalyzerFound (bool found) {m_comAnalyzerFound = found;}
    void setHidAnalyzerFound (bool found) {m_hidAnalyzerFound = found;}

    QString getSerialNumber(void) const;
    int getDots() { return m_dotsNumber; }
    bool sendCommand(QString cmd);

private:
//    void send (char* byte);
//    char read (void);

    hidAnalyzer * m_hidAnalyzer;
    comAnalyzer * m_comAnalyzer;

    quint32 m_analyzerModel;
    bool m_comAnalyzerFound;
    bool m_hidAnalyzerFound;
    quint32 m_chartCounter;
    bool m_isMeasuring;
    bool m_isContinuos;
    quint32 m_dotsNumber;
    bool m_autoCheckUpdate;

    Downloader *m_downloader;
    UpdateDialog *m_updateDialog;

    QByteArray  *m_pfw;
    qint32 m_INFOSIZE;
    const quint32 m_MAGICAA230Z;
    const quint32 m_MAGICHID;
    const quint32 m_INTERNALMAGICAA230Z;
    const quint32 m_MAGICAA30ZERO;

    volatile bool m_calibrationMode;

    //---------------------------------------

signals:
    void analyzerFound(QString);
    void analyzerDisconnected();
    void newData (rawData);
    void newUserData (rawData, UserData);
    void newUserDataHeader (QStringList);
    void newMeasurement(QString);
    void newMeasurement(QString, qint64 fqFrom, qint64 fqTo, qint32 dotsNumber);
    void continueMeasurement(qint64 fqFrom, qint64 fqTo, qint32 dotsNumber);
    void measurementComplete();
    void analyzerDataStringArrived(QString);
    void analyzerScreenshotDataArrived(QByteArray);
    void screenshotComplete(void);
    void updatePercentChanged(qint32);
    void aa30bootFound();
    void aa30updateComplete();
    void signalMeasurementError();

public slots:
    void searchAnalyzer();
    void on_hidAnalyzerFound (quint32 analyzerNumber);
    void on_comAnalyzerFound (quint32 analyzerNumber);
    void on_comAnalyzerDisconnected ();
    void on_hidAnalyzerDisconnected ();
    void on_measure (qint64 fqFrom, qint64 fqTo, qint32 dotsNumber);
    void on_measureUser (qint64 fqFrom, qint64 fqTo, qint32 dotsNumber);
    void on_measureContinuous(qint64 fqFrom, qint64 fqTo, qint32 dotsNumber);
    void on_measureOneFq(QWidget* parent, qint64 fqFrom, qint32 dotsNumber);
    void on_newData(rawData _rawData);
    void on_newUserData(rawData,UserData);
    void on_newUserDataHeader(QStringList);
    void on_analyzerDataStringArrived(QString str);
    void on_stopMeasuring();
    void on_itemDoubleClick(QString number, QString dotsNumber, QString name);
    void on_dialogClosed();
    void getAnalyzerData();
    void on_analyzerScreenshotDataArrived(QByteArray arr);
    void on_screenshotComplete(void);
    void makeScreenshot();
    void on_updatePercentChanged(int number);
    void on_downloadInfoComplete();
    void on_downloadFileComplete();
    void on_checkUpdatesBtn_clicked();
    void setAutoCheckUpdate( bool state);
    void readFile(QString pathToFw);
    void on_internetUpdate();
    void on_progress(qint64 downloaded,qint64 total);
    void on_measureCalib(int dotsNumber);
    void setCalibrationMode(bool enabled);
    void on_stopMeasure();
    void on_changedAutoDetectMode(bool state);
    void on_changedSerialPort(QString portName);
    void slotFullInfo(QString str);
};

#endif // ANALYZER_H

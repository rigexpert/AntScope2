#ifndef COMANALYZER_H
#define COMANALYZER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QTimer>
#include <qdebug.h>
#include <math.h>

#ifndef _NO_WINDOWS_
#include <windows.h>
#endif

#include <analyzer/analyzerparameters.h>
#include <devinfo/redeviceinfo.h>


class comAnalyzer : public QObject
{
    Q_OBJECT
    enum ReturnCode{Complete=0, EraseError, WriteFwError,
                AppSizeError, CrcMCUError, CrcFlashError,
                TimeoutMCUError, FirmwareTypeError, TimeoutError, OpenFwError,
                OpenPortError, WritePortError, ReadPortError,};
public:
    explicit comAnalyzer(QObject *parent = 0);
    ~comAnalyzer();
    QString getVersion() const;
    QString getRevision() const;
    QString getSerial() const;
    bool update(QIODevice *fw);
    bool openComPort(const QString& portName, quint32 portSpeed);
    void closeComPort();

    void setIsMeasuring (bool isMeasuring) {m_isMeasuring = isMeasuring;}
    bool getIsMeasuring (void) const { return m_isMeasuring;}
    void setContinuos(bool continuos){m_isContinuos = continuos;}
    bool getContinuos(void){ return m_isContinuos;}
    void setAnalyzerModel (int model) {m_analyzerModel = model;}
    bool getAnalyzerModel (void) const { return m_analyzerModel;}
    void setIsFRXMode(bool _mode=true) { m_isFRX = _mode;}
    bool getIsFRXMode() { return m_isFRX; }
    qint64 sendData(QString data);
    void setParseState(int _state) { m_parseState=_state; } // analyzerparameters.h: enum parse{}
    int getParseState() { return m_parseState; }
    void applyLicense(QString _license);

private:
    QSerialPort * m_comPort;
    QStringList m_comAvailables;
    QByteArray m_incomingBuffer;
    //QString m_chartData;
    quint32 m_parseState;
    quint32 m_analyzerModel;
    QTimer * m_chartTimer;
    QTimer * m_sendTimer;
    QList <QString> m_stringList;
    QString m_version;
    QString m_revision;
    QString m_serialNumber;

    volatile bool m_isMeasuring;
    volatile bool m_isContinuos;
    volatile bool m_isFRX = true;

    volatile bool m_ok;
    volatile bool m_updateOK;
    volatile qint32 m_updateErr;
    volatile bool m_analyzerPresent;
//    volatile bool m_searchAA30ZERO;

    bool m_autoDetectMode;
    QString m_serialPortName;
    QString m_license;

    qint32 parse (QByteArray arr);
    quint32 compareStrings(QString arr, QString arr1);
    bool waitAnswer();
    QString textError(ReturnCode code);

    unsigned char crc8(QByteArray *buf);//, unsigned int length);

signals:
    void analyzerFound (quint32);
    void analyzerDisconnected();
    void newData(rawData);
    void newUserDataHeader(QStringList);
    void newUserData(rawData, UserData);
    void analyzerDataStringArrived(QString);
    void analyzerScreenshotDataArrived(QByteArray);
    void updatePercentChanged(int);
    void aa30bootFound();
    void aa30updateComplete();
    void signalFullInfo(QString str);
    void signalMeasurementError();

public slots:
    void dataArrived();
    void searchAnalyzer();
    void timeoutChart();
    void timeoutChartUser();
    void startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber, bool frx=true);
    void startMeasureOneFq(qint64 fqFrom, int dotsNumber, bool frx=true);
    void stopMeasure();
    void continueMeasurement();
    void checkAnalyzer();
    void getAnalyzerData();
    void getAnalyzerData(QString number);
    void makeScreenshot();
    void on_screenshotComplete();
    void on_measurementComplete();
    void on_changedAutoDetectMode(bool state);
    void on_changedSerialPort(QString portName);
    void versionRequest();
};

#endif // COMANALYZER_H

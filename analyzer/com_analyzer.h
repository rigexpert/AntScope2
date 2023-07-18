#ifndef COM_ANALYZER_H
#define COM_ANALYZER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QTimer>
#include <qdebug.h>
#include <math.h>

#include "baseanalyzer.h"

#ifndef _NO_WINDOWS_
#include <windows.h>
#endif

#include <analyzer/analyzerparameters.h>
#include <devinfo/redeviceinfo.h>

#define PING_TIMEOUT_MS 3000


class ComAnalyzer : public BaseAnalyzer
{
    Q_OBJECT
    enum ReturnCode{Complete=0, EraseError, WriteFwError,
                AppSizeError, CrcMCUError, CrcFlashError,
                TimeoutMCUError, FirmwareTypeError, TimeoutError, OpenFwError,
                OpenPortError, WritePortError, ReadPortError,};
public:
    explicit ComAnalyzer(QObject *parent = 0);
    ~ComAnalyzer();

    bool update(QIODevice *fw);
    bool openComPort(const QString& portName, quint32 portSpeed);
    void closeComPort();

    virtual qint64 sendData(const QByteArray& data);
    virtual qint64 sendCommand(const QString& data);

    void setTakeData(bool _state) { m_isTakeData = _state; }
    virtual bool refreshConnection();
    virtual bool connectAnalyzer();
    virtual void disconnectAnalyzer();

signals:
    //void analyzerFound (quint32);
    //void analyzerDisconnected();
    //void newData(rawData);
    //void newUserDataHeader(QStringList);
    //void newUserData(rawData, UserData);
    //void analyzerDataStringArrived(QString);
    //void analyzerScreenshotDataArrived(QByteArray);
    //void updatePercentChanged(int);
    void signalFullInfo(QString str);
    void signalMeasurementError();

    //{ TODO obsolete
    void aa30bootFound();
    void aa30updateComplete();
    //}

public slots:
    void searchAnalyzer();
    //void startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber, bool frx=true);
    //void startMeasureOneFq(qint64 fqFrom, int dotsNumber, bool frx=true);
    //void stopMeasure();
    //void getAnalyzerData();
    //void getAnalyzerData(QString number);
    void makeScreenshot();
    //void on_screenshotComplete();
    //void on_measurementComplete();
    //void continueMeasurement();

    //{ TODO obsolete
    void on_changedAutoDetectMode(bool state);
    void on_changedSerialPort(QString portName);
    //}

private slots:
// COM specific
    void dataArrived();
    void versionRequest();
    void handlePing();
    void sendPing();
    void timeoutChart();
    void timeoutChartUser();

private:
    QSerialPort * m_comPort;
    QStringList m_comAvailables;
    //QByteArray m_incomingBuffer;
    //QString m_chartData;
    QTimer * m_chartTimer;
    //QTimer * m_sendTimer;
    QTimer * m_pingTimer;

    //QList <QString> m_stringList;
    long m_lastReadTimeMS;
    bool m_bWaitingPing = false;

    bool m_bAA55mode = false;
    bool m_bAA55modeNewProtocol = false;


//    volatile bool m_ok;
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


};

#endif // COM_ANALYZER_H

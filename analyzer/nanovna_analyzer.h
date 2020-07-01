#ifndef NANOVNA_ANALYZER_H
#define NANOVNA_ANALYZER_H

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

#include "analyzerparameters.h"
#include "devinfo/redeviceinfo.h"

#define NANOVNA_VID 0x0483
#define NANOVNA_PID 0x5740

class NanovnaAnalyzer : public QObject
{
    Q_OBJECT

    enum {
        WAIT_NANO_NO=100,
        WAIT_NANO_VER,
        WAIT_NANO_VER_COMPLETE,
        WAIT_NANO_SWEEP,
        WAIT_NANO_FQ,
        WAIT_NANO_DATA
    };

public:
    explicit NanovnaAnalyzer(QObject *parent = 0);
    ~NanovnaAnalyzer();
    QString getVersion() const;
    QString getRevision() const;
    QString getSerial() const;
    bool openComPort(const QString& portName, quint32 portSpeed=115200);
    void closeComPort();

    void setIsMeasuring (bool isMeasuring) {m_isMeasuring = isMeasuring;}
    bool getIsMeasuring (void) const { return m_isMeasuring;}
    qint64 sendData(QString data);
    void setParseState(int _state) { m_parseState=_state; } // analyzerparameters.h: enum parse{}
    int getParseState() { return m_parseState; }
    QSerialPort* comport() { return m_comPort; }
    rawData toRawData(QString& s1p);
    void setContinuos(bool continuos){m_isContinuos = continuos;}
    bool getContinuos(void){ return m_isContinuos;}

    static void detectPorts();
    static QList<QSerialPortInfo> availablePorts() { return m_listNanovnaPorts; }
    static int portsCount() { return m_listNanovnaPorts.size(); }
    static bool isConnected() { return m_isConnected; }

private:
    QSerialPort * m_comPort;
    QStringList m_comAvailables;
    QByteArray m_incomingBuffer;
    //QString m_chartData;
    quint32 m_parseState;
    QList <QString> m_stringList;
    QString m_version;
    QString m_revision;
    QString m_serialNumber;

    volatile bool m_isMeasuring;
    volatile bool m_isContinuos;
    volatile bool m_ok;
    volatile bool m_analyzerPresent;
    qint64 m_fqFrom;
    qint64 m_fqTo;
    int m_dotsNumber;
    QString m_serialPortName;

    qint32 parse (QByteArray arr);
    bool waitAnswer();
    QList<QString> m_listFQ;

    static QList<QSerialPortInfo> m_listNanovnaPorts;
    static bool m_isConnected;

signals:
    void analyzerFound (QString);
    void analyzerDisconnected();
    void analyzerDataStringArrived(QString);
    void analyzerScreenshotDataArrived(QByteArray);
    void signalFullInfo(QString str);
    void signalMeasurementError();
    void newData(rawData);
    void dataReceived(QString msg);
    void completeMeasurement();

public slots:
    void dataArrived();
    void searchAnalyzer();
    void startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber, bool frx=true);
    void stopMeasure();
    void checkAnalyzer();
    void makeScreenshot();
    void on_screenshotComplete();
    void on_measurementComplete();
    void on_changedSerialPort(QString portName);
    void versionRequest();
    void portClosed();
};

#endif // NANOVNA_ANALYZER_H

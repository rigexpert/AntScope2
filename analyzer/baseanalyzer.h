#ifndef BASEANALYZER_H
#define BASEANALYZER_H

#include <QObject>
#include <analyzer/analyzerparameters.h>

class BaseAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit BaseAnalyzer(QObject *parent = 0);

    virtual QString getVersion() const { return m_version; }
    virtual QString getRevision() const { return m_revision; }
    virtual void setRevision(QString rev){ m_revision = rev; }
    virtual QString getSerial() const { return m_serialNumber; }
    virtual bool update(QIODevice *) { return false; }
    virtual bool openComPort(const QString& portName, quint32 portSpeed) { Q_UNUSED(portSpeed); Q_UNUSED(portName); return false; }
    virtual void closeComPort() {}

    virtual void setIsMeasuring (bool isMeasuring) {m_isMeasuring = isMeasuring;}
    virtual bool isMeasuring (void) const { return m_isMeasuring;}
    virtual void setContinuos(bool continuos){m_isContinuos = continuos;}
    virtual bool getContinuos(void){ return m_isContinuos;}
    virtual void setAnalyzerModel (int model) {m_analyzerModel = model;}
    virtual int getAnalyzerModel (void) const { return m_analyzerModel;}
    virtual void setIsFRXMode(bool _mode=true) { m_isFRX = _mode;}
    virtual bool getIsFRXMode() { return m_isFRX; }
    virtual qint64 sendData(QString data) { qDebug() << "BaseAnalyzer::sendData"; return 0; }
    virtual void setParseState(int _state) { m_parseState=_state; } // analyzerparameters.h: enum parse{}
    virtual int getParseState() { return m_parseState; }
    virtual void applyLicense(QString _license) {}
    virtual void setTakeData(bool _state) { m_isTakeData = _state; }
    virtual bool connectAnalyzer() = 0;
    virtual void disconnectAnalyzer() = 0;
    ReDeviceInfo::InterfaceType type() { return m_type; }

signals:
#ifdef NEW_CONNECTION
    void analyzerFound (quint32);
#else
    void analyzerFound (QString);
#endif
    void analyzerDisconnected();
    void newData(rawData);
    void newUserDataHeader(QStringList);
    void newUserData(rawData, UserData);
    void analyzerDataStringArrived(QString);
    void analyzerScreenshotDataArrived(QByteArray);
    void updatePercentChanged(int);
    void signalFullInfo(QString str);
    void signalMeasurementError();
    void signalOk();
    void completeMeasurement(); // emited by NanoVNA

public slots:
    void searchAnalyzer() {}
    virtual void startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber, bool frx=true);
    void startMeasureOneFq(qint64 fqFrom, int dotsNumber, bool frx=true);
    void stopMeasure();
    void getAnalyzerData();
    void getAnalyzerData(QString number);
    void makeScreenshot() {}
    void on_screenshotComplete();
    void on_measurementComplete();
    void continueMeasurement();

protected:
    QString m_version;
    QString m_revision;
    QString m_serialNumber;
    quint32 m_parseState;
    quint32 m_analyzerModel;
    volatile bool m_isMeasuring;
    volatile bool m_isContinuos;
    volatile bool m_isFRX = true;
    bool m_ok = false;
    QTimer * m_sendTimer;
    bool m_isTakeData = false;
    QByteArray m_incomingBuffer;
    QList <QString> m_stringList;
    ReDeviceInfo::InterfaceType m_type = ReDeviceInfo::WRONG;

};

#endif // BASEANALYZER_H

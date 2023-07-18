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
    virtual bool isMeasuring (void) const { return m_isMeasuring; }
    virtual void setContinuos(bool continuos){m_isContinuos = continuos;}
    virtual bool getContinuos(void){ return m_isContinuos;}
    virtual void setAnalyzerModel (int model) {m_analyzerModel = model;}
    virtual int getAnalyzerModel (void) const { return m_analyzerModel;}
    virtual void setIsFRXMode(bool _mode=true) { m_isFRX = _mode;}
    virtual bool getIsFRXMode() { return m_isFRX; }
    virtual qint64 sendData(const QByteArray& ) { return 0; }
    virtual qint64 sendCommand(const QString& ) { return 0; }
    virtual void setParseState(int _state) { m_parseState=_state; } // analyzerparameters.h: enum parse{}
    virtual int getParseState() { return m_parseState; }
    virtual void applyLicense(QString _license) {}
    virtual void setTakeData(bool _state) { m_isTakeData = _state; }
    virtual bool refreshConnection() { return false; }
    virtual bool connectAnalyzer() = 0;
    virtual void disconnectAnalyzer() = 0;
    ReDeviceInfo::InterfaceType connectionType() { return m_type; }

signals:
    void analyzerFound (int analyzerIndex);
    void analyzerDisconnected();
    void newData(RawData);
    void newUserDataHeader(QStringList);
    void newUserData(RawData, UserData);
    void analyzerDataStringArrived(QString);
    void analyzerScreenshotDataArrived(QByteArray);
    void updatePercentChanged(int);
    void signalFullInfo(const QString& _info);
    void signalMeasurementError();
    void signalAnalyzerError(const QString& msg);
    void signalOk();
    void completeMeasurement(); // emited by NanoVNA

public slots:
    virtual void searchAnalyzer() {}
    virtual void startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber, bool frx=true);
    virtual void startMeasureOneFq(qint64 fqFrom, int dotsNumber, bool frx=true);
    virtual void stopMeasure();
    virtual void getAnalyzerData();
    virtual void getAnalyzerData(QString number);
    virtual void makeScreenshot() {}
    virtual void on_screenshotComplete();
    virtual void on_measurementComplete();
    virtual void continueMeasurement();

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

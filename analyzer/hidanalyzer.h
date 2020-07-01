#ifndef HIDANALYZER_H
#define HIDANALYZER_H

#include <QCoreApplication>
#include <QObject>
#include <QTimer>
#include <QMessageBox>
#include <analyzer/usbhid/hidapi/hidapi.h>
#include <qdebug.h>
#include <analyzer/analyzerparameters.h>
#include <math.h>

//enum hidParse{
//    VER = 1,
//    WAIT_DATA
//};

#ifndef RE_VID

#define RE_VID          0x0483
#define RE_PID			0xA1DE

#define RE_BOOT_VID     0x0483
#define RE_BOOT_PID     0xA1DA

#define REPORT_SIZE 64
#define ANTSCOPE_REPORT			0x07
#define INPUT_BUFFER_SIZE		32768

#define PREFIX_SERIAL_NUMBER_AA35	1350
#define PREFIX_SERIAL_NUMBER_AA35_ZOOM	1351
#define PREFIX_SERIAL_NUMBER_AA55	1550
#define PREFIX_SERIAL_NUMBER_AA55_ZOOM	1551
#define PREFIX_SERIAL_NUMBER_AA230_ZOOM	1232
#define PREFIX_SERIAL_NUMBER_AA650_ZOOM	1650
#define PREFIX_SERIAL_NUMBER_AA230_STICK 4230
#define PREFIX_SERIAL_NUMBER_AA2000	4002
#define PREFIX_SERIAL_NUMBER_STICK_PRO 4670
#define PREFIX_SERIAL_NUMBER_AA1500_ZOOM	1015

#endif

class hidAnalyzer : public QObject
{
    Q_OBJECT
    enum BLCMD {BL_CMD_GET_ID = 1, BL_CMD_ERASE, BL_CMD_WRITE,
                            BL_CMD_DATA, BL_CMD_CHECK, BL_CMD_START,
                            BL_CMD_OK, BL_CMD_ERROR};
public:
    explicit hidAnalyzer(QObject *parent = nullptr);
    ~hidAnalyzer();

    QString getVersion(void) const;
    QString getRevision(void) const;
    void setRevision(QString rev){ m_revision = rev;}
    QString getSerial(void) const;
    int getModel(void) const;
    void nonblocking (int nonblock);
    bool update(QIODevice *fw);
    void setIsMeasuring (bool isMeasuring);
    bool getIsMeasuring (void) const { return m_isMeasuring;}
    void setContinuos(bool continuos){m_isContinuos = continuos;}
    bool getContinuos(void){ return m_isContinuos;}
    void preUpdate();
    void setIsFRXMode(bool _mode=true) { m_isFRX = _mode;}
    bool getIsFRXMode() { return m_isFRX; }
    void sendData(QString data);
    void setParseState(int _state) { m_parseState=_state; } // analyzerparameters.h: enum parse{}
    int getParseState() { return m_parseState; }
    void applyLicense(QString _license);

private:
    hid_device *m_hidDevice;
    QString m_serialNumber;
    quint32 m_parseState;
    QTimer *m_checkTimer;
    quint32 m_analyzerModel;
    QTimer * m_chartTimer;
    QTimer * m_sendTimer;
    QTimer * m_hidReadTimer;
    QByteArray m_incomingBuffer;
    QList <QString> m_stringList;

    QString m_version;
    QString m_revision;

    volatile bool m_ok;
    volatile bool m_isMeasuring;
    volatile bool m_isContinuos;
    volatile bool m_isFRX = true;

    bool m_analyzerPresent;

    volatile bool m_bootMode;

    QMutex m_mutexSearch;
    QMutex m_mutexRead;
    struct hid_device_info* m_devices;
    QThread* m_refreshThread;

//    unsigned char m_inputBuffer[INPUT_BUFFER_SIZE];
//    int m_inputBufferHead;
//    int m_inputBufferTail;

    bool connect(quint32 vid, quint32 pid);
    bool disconnect(void);
    qint32 parse (QByteArray arr);
    bool waitAnswer();
    QFuture<struct hid_device_info*> *m_futureRefresh;
    QFutureWatcher<struct hid_device_info*> *m_watcherRefresh;
    QString m_license;

signals:
    void analyzerFound (quint32);
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

public slots:
    bool searchAnalyzer(bool arrival);
    void startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber);
    void startMeasureOneFq(qint64 fqFrom, int dotsNumber);
    void getAnalyzerData();
    void getAnalyzerData(QString number);
    void makeScreenshot();
    void stopMeasure();
    void refreshReady();
    void startResresh();

private slots:
    void checkTimerTick ();
    void on_screenshotComplete();
    void on_measurementComplete();
    void timeoutChart();
    void timeoutChartUser();
    void continueMeasurement();
    void hidRead (void);
    struct hid_device_info* refreshThreadStarted();

};

#endif // HIDANALYZER_H

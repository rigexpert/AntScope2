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

class hidAnalyzer : public QObject
{
    Q_OBJECT
    enum BLCMD {BL_CMD_GET_ID = 1, BL_CMD_ERASE, BL_CMD_WRITE,
                            BL_CMD_DATA, BL_CMD_CHECK, BL_CMD_START,
                            BL_CMD_OK, BL_CMD_ERROR};
public:
    explicit hidAnalyzer(QObject *parent = 0);
    ~hidAnalyzer();

    QString getVersion(void) const;
    QString getRevision(void) const;
    void setRevision(quint32 rev){m_revision = QString::number(rev);}
    QString getSerial(void) const;
    int getModel(void) const;
    void nonblocking (int nonblock);
    bool update(QIODevice *fw);
    void setIsMeasuring (bool isMeasuring) {m_isMeasuring = isMeasuring;}
    bool getIsMeasuring (void) const { return m_isMeasuring;}


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
    bool m_isMeasuring;
    bool m_analyzerPresent;

    volatile bool m_bootMode;

//    unsigned char m_inputBuffer[INPUT_BUFFER_SIZE];
//    int m_inputBufferHead;
//    int m_inputBufferTail;

    bool connect(quint32 vid, quint32 pid);
    bool disconnect(void);
    void sendData(QString data);
    qint32 parse (QByteArray arr);
    bool waitAnswer();

signals:
    void analyzerFound (quint32);
    void analyzerDisconnected();
    void newData(rawData);
    void analyzerDataStringArrived(QString);
    void analyzerScreenshotDataArrived(QByteArray);
    void updatePercentChanged(int);
    void signalFullInfo(QString str);

public slots:
    bool searchAnalyzer(bool arrival);
    void startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber);
    void getAnalyzerData();
    void getAnalyzerData(QString number);
    void makeScreenshot();
    void stopMeasure();

private slots:
    void checkTimerTick ();
    void on_screenshotComplete();
    void on_measurementComplete();
    void timeoutChart();
    void continueMeasurement();
    void hidRead (void);
};

#endif // HIDANALYZER_H

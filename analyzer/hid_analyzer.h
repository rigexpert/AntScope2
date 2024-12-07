#ifndef HID_ANALYZER_H
#define HID_ANALYZER_H

#include <QCoreApplication>
#include <QObject>
#include <QTimer>
#include <QMessageBox>
#include <analyzer/usbhid/hidapi/hidapi.h>
#include <qdebug.h>
#include <analyzer/analyzerparameters.h>
#include <math.h>

#include "baseanalyzer.h"

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

#ifndef NEW_ANALYZER
#define PREFIX_SERIAL_NUMBER_AA35	1350
#define PREFIX_SERIAL_NUMBER_AA35_ZOOM	1351
#define PREFIX_SERIAL_NUMBER_AA55	1550
#define PREFIX_SERIAL_NUMBER_AA55_ZOOM	1551
#define PREFIX_SERIAL_NUMBER_AA230_ZOOM	1232
#define PREFIX_SERIAL_NUMBER_AA650_ZOOM	1650
#define PREFIX_SERIAL_NUMBER_AA230_STICK 4230
#define PREFIX_SERIAL_NUMBER_AA2000	4002
#define PREFIX_SERIAL_NUMBER_STICK_PRO 4600
#define PREFIX_SERIAL_NUMBER_AA1500_ZOOM	1015
#define PREFIX_SERIAL_NUMBER_ZEROII	4001
#define PREFIX_SERIAL_NUMBER_TOUCH 4003
#define PREFIX_SERIAL_NUMBER_TOUCH_EINK 4004
#endif

#endif

class HidAnalyzer : public BaseAnalyzer
{
    Q_OBJECT
    enum BLCMD {BL_CMD_GET_ID = 1, BL_CMD_ERASE, BL_CMD_WRITE,
                            BL_CMD_DATA, BL_CMD_CHECK, BL_CMD_START,
                            BL_CMD_OK, BL_CMD_ERROR};
public:
    explicit HidAnalyzer(QObject *parent = nullptr);
    ~HidAnalyzer();

    bool update(QIODevice *fw);

    void nonblocking (int nonblock);
    void preUpdate();
    QString hidError(hid_device* _device);
    virtual bool connectAnalyzer();
    virtual void disconnectAnalyzer();
    bool closeHid();
    qint64 sendCommand(const QString& data);
    virtual qint64 sendData(const QByteArray& );
    virtual bool refreshConnection();

signals:

public slots:
    bool searchAnalyzer(bool arrival);
    void makeScreenshot();
//    void startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber, bool frx=true);
//    void startMeasureOneFq(qint64 fqFrom, int dotsNumber, bool frx=true);

private slots:
    void refreshReady();
    void startResresh();
    void checkTimerTick ();
    void timeoutChart();
    void timeoutChartUser();
    void timeoutChartS21();
    void hidRead (void);
    struct hid_device_info* refreshThreadStarted();

private:
    hid_device *m_hidDevice;
    QTimer *m_checkTimer;
    QTimer * m_chartTimer;
    QTimer * m_hidReadTimer;
    bool m_analyzerPresent;
    volatile bool m_bootMode;
    QMutex m_mutexSearch;
    QMutex m_mutexRead;
    struct hid_device_info* m_devices;
    QThread* m_refreshThread;
    bool connectHid(quint32 vid, quint32 pid);
    bool disconnectHid(void);
    qint32 parse (QByteArray arr);
    bool waitAnswer();
    QFuture<struct hid_device_info*> *m_futureRefresh;
    QFutureWatcher<struct hid_device_info*> *m_watcherRefresh;
};

#endif // HID_ANALYZER_H

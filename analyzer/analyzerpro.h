#ifndef ANALYZERPRO_H
#define ANALYZERPRO_H

#include <QObject>
#include <QDateTime>
#include "baseanalyzer.h"
#include <updatedialog.h>
#include <crc32.h>
#include "analyzerparameters.h"
#include <analyzer/updater/downloader.h>


class AnalyzerPro : public QObject
{
    Q_OBJECT

    BaseAnalyzer* m_baseAnalyzer=nullptr;
    quint32 m_analyzerModel;
    quint32 m_chartCounter;
    bool m_isMeasuring=false;
    bool m_isContinuos=false;
    quint32 m_dotsNumber;
    bool m_getAnalyzerData=false;

    Downloader *m_downloader;
    UpdateDialog *m_updateDialog;
    bool m_bManualUpdate = false;

    QByteArray  *m_pfw;
    qint32 m_INFOSIZE;
    volatile bool m_calibrationMode=false;
    QString m_license;

public:
    explicit AnalyzerPro(QObject *parent = nullptr);
    virtual ~AnalyzerPro();

    QString getModelString(void);
    quint32 getModel(void);
    double getVersion() const;
    QString getVersionString() const;
    QString getRevision() const;
    QString getSerialNumber(void) const;
    QString getLicense() const { return m_license; }

    void updateFirmware (QIODevice *fw);
    void setContinuos(bool isContinuos);
    bool isMeasuring() { return m_isMeasuring; }
    void setIsMeasuring (bool _isMeasuring);
    bool sendData(const QByteArray& _data);
    bool sendCommand(const QString& _cmd);
    void setParseState(int _state);
    int  getParseState();
    ReDeviceInfo::InterfaceType connectionType();

protected:
    bool createDevice(const SelectionParameters& param, BaseAnalyzer* analyzer=nullptr);
    void connectSignals();

signals:
    void showNotification(QString msg, QString url);
    void newMeasurement(QString, qint64 fqFrom, qint64 fqTo, qint32 dotsNumber);
    void continueMeasurement(qint64 fqFrom, qint64 fqTo, qint32 dotsNumber);
    void measurementComplete();
    void measurementCompleteNano();
    void newData (RawData);
    void newS21Data (S21Data);
    void newUserData (RawData, UserData);
    void newUserDataHeader (QStringList);
    void newAnalyzerData (RawData);
    void newMeasurement(QString);
    void analyzerDataStringArrived(QString);
    void analyzerScreenshotDataArrived(QByteArray);
    void screenshotComplete(void);
    void updatePercentChanged(int number);
    void signalMeasurementError();
    void deviceDisconnected();
    void updateAutocalibrate5(int _dots, QString _msg);
    void stopAutocalibrate5();
    void analyzerFound (int analyzerIndex);
    void signalAnalyzerError(const QString& msg);
    void signalMatch_12Received(QByteArray data);
    void signalMatch_Profile_B16Received(QByteArray data);

public slots:
    bool refreshConnection(); // use SelectionParameters::selected
    void searchAnalyzer();
    void on_analyzerFound (int analyzerIndex) { emit analyzerFound(analyzerIndex); }
    void on_connectDevice(BaseAnalyzer* analyzer=nullptr);
    void on_disconnectDevice();
    void on_measure (qint64 fqFrom_hz, qint64 fqTo_hz, qint32 dotsNumber);
    void on_measureS21 (qint64 fqFrom_hz, qint64 fqTo_hz, qint32 dotsNumber);
    void on_measureUser (qint64 fqFrom_hz, qint64 fqTo_hz, qint32 dotsNumber);
    void on_measureContinuous(qint64 fqFrom_hz, qint64 fqTo_hz, qint32 dotsNumber);
    void on_measureOneFq(QWidget* parent, qint64 fqFrom_hz, qint32 dotsNumber);
    void on_stopMeasure();
    void on_newData(RawData _rawData);
    void on_newS21Data(S21Data _s21Data);
    void on_newUserData(RawData,UserData);
    void on_newUserDataHeader(QStringList);
    void on_analyzerDataStringArrived(QString str);
    void getAnalyzerData();
    void closeAnalyzerData();

    void on_itemDoubleClick(QString number, QString dotsNumber, QString name);

    void on_analyzerScreenshotDataArrived(QByteArray arr);
    void on_screenshotComplete(void);
    void makeScreenshot();

    bool needCheckForUpdate();
    void on_updatePercentChanged(int number);
    void on_statisticsComplete();
    void on_downloadInfoComplete();
    void on_downloadFileComplete();
    void on_checkUpdatesBtn_clicked();
    void checkFirmwareUpdate();
    void readFile(QString pathToFw);
    void on_internetUpdate();
    void on_progress(qint64 downloaded,qint64 total);

    void on_measureCalib(int dotsNumber);
    void setCalibrationMode(bool enabled);

    void slotFullInfo(const QString& _info);

    void applyAnalyzer();

};

#endif // ANALYZERPRO_H

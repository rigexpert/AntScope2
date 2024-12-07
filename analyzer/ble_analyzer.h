#ifndef BLEANALYZER_H
#define BLEANALYZER_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QBluetoothAddress>
#include <QBluetoothUuid>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QDataStream>
#include <QTimer>
#include "baseanalyzer.h"


enum {
    BLE_VER_CMD = (quint8)0xE6,
    BLE_FRX_CMD = (quint8)0x7F,
    BLE_REC_LIST_CMD = (quint8)0x8F,
    BLE_REC_DATA_CMD = (quint8)0x9F,
    BLE_REC_ERASE_CMD = (quint8)0xAF,
    BLE_SCREENSHOT_CMD = (quint8)0xAD,
    BLE_PING_CMD = (quint8)0x5A,
    BLE_CANCEL_CMD = (quint8)0x69,
    BLE_FULLINFO_CMD = (quint8)0x9B,
    BLE_SCREEN_PAL_CMD = (quint8)0xDA
};

enum {
    BLE_FULLINFO_NAME = (quint8)0x00,
    BLE_FULLINFO_FLAGS = (quint8)0x01,
    BLE_FULLINFO_MEASURER = (quint8)0x02,
    BLE_FULLINFO_SERIAL_VER = (quint8)0x03,
    BLE_FULLINFO_MCU_TYPE_STR = (quint8)0x04,
    BLE_FULLINFO_DISPLAY = (quint8)0x05,
    BLE_FULLINFO_FLASH = (quint8)0x06,
    BLE_FULLINFO_HW_STR = (quint8)0x07
};

enum {
    BLE_SUPPORT_NONE = 0,
    BLE_SUPPORT_PARTIAL,
    BLE_SUPPORT_FULL
};

#define BLE_PACKET_SIZE 20
#define PING_TIMEOUT_MS 3000

class BleDeviceInfo: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ getName NOTIFY deviceChanged)
    Q_PROPERTY(QString deviceAddress READ getAddress NOTIFY deviceChanged)

public:
    BleDeviceInfo(const QBluetoothDeviceInfo &device) : m_device(device) {}
    BleDeviceInfo() {}

    void setDevice(const QBluetoothDeviceInfo &device) { m_device = device; emit deviceChanged(); }

    QString getName() const { return m_device.name(); }
    QString getAddress() const { return m_device.address().toString(); }
    QBluetoothDeviceInfo getDevice() const { return m_device; };

signals:
    void deviceChanged();

private:
    QBluetoothDeviceInfo m_device;
};


class BleRequestRecord {
public:
    BleRequestRecord() { reset(); }
    void reset() { m_recordName.clear(); m_recordCell=-1; m_requestPoints=0; m_requestCenter=0; m_requestRange=0; }
    QString record() {
        return QString("%1,%2,%3,%4:%5")
                .arg(m_recordCell)
                .arg(m_requestCenter)
                .arg(m_requestRange)
                .arg(m_requestPoints)
                .arg(m_recordName);
    }
    qint64 m_requestCenter; // kHz
    qint64 m_requestRange;  // kHz
    qint16 m_requestPoints=0;
    int m_recordCell=-1;
    QString m_recordName;
};


class BleAnalyzer : public BaseAnalyzer
{
    Q_OBJECT
    Q_PROPERTY(QString error READ error WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(QString info READ info WRITE setInfo NOTIFY infoChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)
    QString uuidService    = "D973F2E0-B19E-11E2-9E96-0800200C9A66";
    QString uuidRead       = "706E4F15-3EE6-41C6-BA10-CA8ABDCF3043";
    QString uuidWrite      = "6F8963A8-21E9-4055-86B8-2F911D736CFF";
    QString uuidReturn     = "07395738-5d8a-11ec-bf63-0242ac130002";
    QString uuidDescriptor = "00002902-0000-1000-8000-00805f9b34fb";

public:
    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    static int supported();

    explicit BleAnalyzer(QObject *parent = nullptr);
    virtual ~BleAnalyzer();

    QString error() const;
    void setError(const QString& error);
    QString info() const;
    void setInfo(const QString& info);
    void setResponse(QString& text);
    void setRequest(QString& text);
    void clearMessages();
    bool scanning() const;
    QList<BleDeviceInfo*>& devices();
    void setDevice(BleDeviceInfo *device);
    void setAddressType(AddressType type);
    AddressType addressType() const;
    QString trace(QString _title, QByteArray& _data) ;
    QString bytesToString(QDataStream& stream);
    double shortToDouble(qint16 src);
    void write(QByteArray& arr);
    virtual bool refreshConnection();
    void sendFullInfo();

    // analyzer
    virtual bool connectAnalyzer();
    virtual void disconnectAnalyzer();

protected:
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void dataReceived(const QLowEnergyCharacteristic &c,
                              const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d,
                              const QByteArray &value);
    void parseFullInfo(QDataStream& stream);
    void parseFRX(QDataStream& stream);
    void parseRecList(QDataStream& stream);
    void parseRecData(QDataStream& stream);
    void parseScreenShot(const QByteArray &arr);
    bool checkCRC(const QByteArray& data);
    void returnCRC(const QByteArray& data);
    void setInnerScan(bool _state) { m_innerScan = _state; }

public slots:
    void searchAnalyzer();
    void connectToService(const QString &address);
    void startMeasure(qint64 from_hz, qint64 to_hz, int dotsNumber, bool frx=true);
    void startMeasureOneFq(qint64 fqFrom_hz, int dotsNumber, bool frx=true);
    void getAnalyzerData();
    void getAnalyzerData(QString number);
    void makeScreenshot();
    void stopMeasure();

private slots:
    void addDevice(const QBluetoothDeviceInfo&);
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();
    void parseResponse(const QByteArray& arr);
    void startPing();
    void handlePing();
    void sendPing();
    void stopPing();

signals:
    void errorChanged();
    void infoChanged();
    void response(QString& text);
    void request(QString& text);
    void scanningChanged(int state=0); // 1-started, 0-finished
    void devicesChanged(BleDeviceInfo* info=nullptr);
    void measuringChanged();
    void aliveChanged();
    void statsChanged();

private:
    QString m_error;
    QString m_info;
    bool m_serviceFound=false;
    QTimer * m_pingTimer;
    long m_lastReadTimeMS;
    bool m_bWaitingPing = false;
    bool m_reuChip = false;
    bool m_insideWrite = false;
    bool m_innerScan = false; // scan for restoreConnection purpoces
    BleRequestRecord m_requestRecord;
    QMap<QString, BleRequestRecord> m_analyzerRecords;
    QString m_name;

    // DeviceFinder
    QBluetoothDeviceDiscoveryAgent *m_deviceDiscoveryAgent;
    QList<BleDeviceInfo*> m_devices;
    BleDeviceInfo *m_currentDevice = nullptr;
    // DeviceHandler
    QLowEnergyController *m_control = nullptr;
    QLowEnergyService *m_service = nullptr;
    QLowEnergyCharacteristic m_charWrite;
    QLowEnergyCharacteristic m_charRead;
    QLowEnergyCharacteristic m_charReturn;
    QLowEnergyDescriptor m_notificationDesc;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;
};

#endif // BLEANALYZER_H

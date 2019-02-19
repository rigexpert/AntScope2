#ifndef AA30ZEROFIRMWAREUPDATER_H
#define AA30ZEROFIRMWAREUPDATER_H

#include "firmwareupdater.h"
#include <QObject>
#include <QSerialPort>
#include <QCoreApplication>
#include <qendian.h>
#include <QThread>

class AA30ZEROFirmwareUpdater : public FirmwareUpdater
{
    Q_OBJECT
public:
    AA30ZEROFirmwareUpdater();
    ~AA30ZEROFirmwareUpdater();

    bool update(const ReDeviceInfo &dev, QIODevice *fw);
    FirmwareInfo firmwareInfo(const ReDeviceInfo &dev);
    bool setInBootMode(ReDeviceInfo &dev);
    static unsigned char crc8(QByteArray *buf);


signals:

public slots:
    virtual void stop();

private:
    QSerialPort m_port;
    volatile bool m_stop;

    bool openDevice(const ReDeviceInfo &dev);
    void closeDevice();
    bool waitAnswer();
//    QString textError(ReturnCode code);
};

#endif // AA30ZEROFIRMWAREUPDATER_H

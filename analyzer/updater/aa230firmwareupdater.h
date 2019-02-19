#ifndef AA230FIRMWAREUPDATER_H
#define AA230FIRMWAREUPDATER_H

#include "firmwareupdater.h"
#include <QSerialPort>
#include <QCoreApplication>

class AA230FirmwareUpdater : public FirmwareUpdater
{
    Q_OBJECT

    enum ReturnCode{Complete=0, EraseError, WriteFwError,
                    AppSizeError, CrcMCUError, CrcFlashError,
                    TimeoutMCUError, FirmwareTypeError, TimeoutError, OpenFwError,
                    OpenPortError, WritePortError, ReadPortError,};

public:
    explicit AA230FirmwareUpdater(QObject *parent = 0);
    ~AA230FirmwareUpdater();

    bool update(const ReDeviceInfo &dev, QIODevice *fw);
    FirmwareInfo firmwareInfo(const ReDeviceInfo &dev);
    bool setInBootMode(ReDeviceInfo &dev);


signals:

public slots:
    virtual void stop();

private:
    QSerialPort m_port;
    volatile bool m_stop;

    bool openDevice(const ReDeviceInfo &dev);
    void closeDevice();
    bool waitAnswer();
    QString textError(ReturnCode code);
};

#endif // AA230FIRMWAREUPDATER_H

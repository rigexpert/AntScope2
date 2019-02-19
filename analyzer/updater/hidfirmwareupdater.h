#ifndef HIDFIRMWAREUPDATER_H
#define HIDFIRMWAREUPDATER_H

#include "firmwareupdater.h"
#include <QLoggingCategory>
#include <QThread>

class HidFirmwareUpdater : public FirmwareUpdater
{
    Q_OBJECT

    enum BLCMD {BL_CMD_GET_ID = 1, BL_CMD_ERASE, BL_CMD_WRITE,
                BL_CMD_DATA, BL_CMD_CHECK, BL_CMD_START,
                BL_CMD_OK, BL_CMD_ERROR};

public:
    explicit HidFirmwareUpdater(QObject *parent = 0);
    ~HidFirmwareUpdater();

    bool update(const ReDeviceInfo &dev, QIODevice *fw);

    FirmwareInfo firmwareInfo(const ReDeviceInfo &dev);
    bool setInBootMode(ReDeviceInfo &dev);

signals:

public slots:
    void stop();

private:

    hid_device*  m_handleDev;
    bool   m_needStop;
    bool waitAnswer();
    bool openDevice(const ReDeviceInfo &dev);
    void closeDevice();

    QString hidError();

};
Q_DECLARE_LOGGING_CATEGORY(HIDFIRMWAREUPDATER)

#endif // HIDFIRMWAREUPDATER_H

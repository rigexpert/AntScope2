#ifndef FIRMWAREUPDATER_H
#define FIRMWAREUPDATER_H

#include <QObject>
#include <QIODevice>
#include "devinfo/redeviceinfo.h"

class FirmwareUpdater:public QObject
{
    Q_OBJECT

public:

    struct FirmwareInfo
    {
        quint32 magic;
        quint32 devtype;
        quint32 major;
        quint32 minor;
        quint32 build;
        quint32 sn;
        quint32 rev;
        char  date[20];
        char  time[20];
    };

    explicit FirmwareUpdater(QObject *parent=0);
    virtual ~FirmwareUpdater();

    virtual bool update(const ReDeviceInfo &dev, QIODevice *fw)=0;

    virtual FirmwareInfo firmwareInfo(const ReDeviceInfo &dev)=0;
    virtual bool setInBootMode(ReDeviceInfo &dev)=0;
    QString lastError() const;

public slots:
    virtual void stop()=0;

protected:
    QString m_error;

signals:
    void progress(int percent);

private:

};

#endif // FIRMWAREUPDATER_H

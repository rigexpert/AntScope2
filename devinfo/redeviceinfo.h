#ifndef REDEVICEINFO_H
#define REDEVICEINFO_H

#include <QCoreApplication>
#include <QObject>
#include <QList>
#include <QMap>
#include <analyzer/usbhid/hidapi/hidapi.h>
//#include "hidapi.h"
#include "ftdi/ftdiinfo.h"

class ReDeviceInfo
{
public:
    enum InterfaceType {HID=0, Serial};
    ReDeviceInfo();
    ~ReDeviceInfo();

    QString systemName() const;
    QString serial() const;
    InterfaceType type() const;
    quint16 vid() const;
    quint16 pid() const;
    QString portName() const;


    static QList<ReDeviceInfo> availableDevices(InterfaceType type);
    static QString deviceName(const ReDeviceInfo &dev);
    static QString externalSerial(const ReDeviceInfo &dev);


protected:
    ReDeviceInfo(InterfaceType type, const QString &name,
                 const QString &serial, quint16 vid, quint16 pid);

    ReDeviceInfo(InterfaceType type, const QString &name, const QString &portName);

private:
    static const quint16 m_bootVid;
    static const quint16 m_bootPid;
    static const quint16 m_devVid;
    static const quint16 m_devPid;

    QString         m_name;
    QString         m_serial;
    QString         m_portName;
    InterfaceType   m_type;
    quint16         m_vid;
    quint16         m_pid;

    static QList<ReDeviceInfo> availableHID(quint16 vid, quint16 pid);
    static QList<ReDeviceInfo> availableSerial();
    static bool checkSerial(const ReDeviceInfo &dev);
};
Q_DECLARE_METATYPE(ReDeviceInfo)

#endif // REDEVICEINFO_H

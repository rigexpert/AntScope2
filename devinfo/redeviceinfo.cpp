#include "redeviceinfo.h"

const quint16 ReDeviceInfo::m_bootVid = 0x0483;
const quint16 ReDeviceInfo::m_bootPid = 0xA1DA;
const quint16 ReDeviceInfo::m_devVid = 0x0483;
const quint16 ReDeviceInfo::m_devPid = 0xA1DE;




ReDeviceInfo::ReDeviceInfo():
    m_vid(0),
    m_pid(0)
{

}

ReDeviceInfo::ReDeviceInfo(InterfaceType type, const QString &name,
                           const QString &serial, quint16 vid, quint16 pid):
    m_name(name),
    m_serial(serial),
    m_type(type),
    m_vid(vid),
    m_pid(pid)
{

}

ReDeviceInfo::ReDeviceInfo(InterfaceType type, const QString &name,  const QString &portName):
    m_name(name),
    m_portName(portName),
    m_type(type),
    m_vid(0),
    m_pid(0)
{

}

ReDeviceInfo::ReDeviceInfo(const ReDeviceInfo& that):
    m_name(that.m_name),
    m_portName(that.m_portName),
    m_serial(that.m_serial),
    m_type(that.m_type),
    m_vid(that.m_vid),
    m_pid(that.m_pid)
{

}

ReDeviceInfo::~ReDeviceInfo()
{

}

QList<ReDeviceInfo> ReDeviceInfo::availableDevices(InterfaceType type)
{
    QList<ReDeviceInfo> list;
    Q_UNUSED(type);

    if (type == HID) {
        list = availableHID(m_bootVid, m_bootPid);
        list += availableHID(m_devVid, m_devPid);
    } else if (type == Serial) {
        return availableSerial();
    }

    return list;
}

ReDeviceInfo ReDeviceInfo::byPort(const QString& port)
{
    QList<ReDeviceInfo> list = ReDeviceInfo::availableSerial();
    foreach (const ReDeviceInfo& info, list) {
        if (info.portName() == port)
            return ReDeviceInfo(info);
    }
    return ReDeviceInfo();
}


QList<ReDeviceInfo> ReDeviceInfo::availableHID(quint16 vid, quint16 pid)
{
    QList<ReDeviceInfo> list;
    QString name;
    QString serial;

    struct hid_device_info *devs, *cur_dev;
    hid_init();

    /* start found of hid-devices*/
    devs = hid_enumerate(vid, pid);

    /*all found devices places in dynamic list*/
    cur_dev = devs;
    while (cur_dev)
    {
        name = QString::fromWCharArray(cur_dev->product_string);
        serial = QString::fromWCharArray(cur_dev->serial_number);
        list.append(ReDeviceInfo(HID, name, serial, vid, pid));
        cur_dev = cur_dev->next;
    }

    /*delete list*/
    hid_free_enumeration(devs);

    hid_exit();
    return list;
}

void showFtdiInfo(FtdiInfo::Info item)
{
    QString portName = item.Info::portName;
    QString portInfo = item.Info::portInfo;
}

QList<ReDeviceInfo> ReDeviceInfo::availableSerial()
{
    QList<ReDeviceInfo> list;
    QList<FtdiInfo::Info> infolist = FtdiInfo::info();

    foreach (FtdiInfo::Info item, infolist)
    {
        item.portInfo.remove("RigExpert ");
        list.append(ReDeviceInfo(Serial, item.portInfo, item.portName));
    }

    if (list.isEmpty()) {
        // ???? obsolete
        //list.append(ReDeviceInfo(Serial, "AA-30 ZERO", "AA-30 ZERO"));
    }
    return list;
}


QString ReDeviceInfo::deviceName(const ReDeviceInfo &dev)
{
    int id;
    QString type;
    QString name;
    QString modelCode;
    QMap <QString, QString> analyzers;
    QMap <QString, QString> interfases;

    if (dev.type() == Serial) {
        return dev.systemName();
    }

    analyzers.insert("550", "55 ZOOM");
    analyzers.insert("551", "55 ZOOM");
    interfases.insert("080", "8");

    if (dev.serial().length() < 9) {
        return "Unknown "+ dev.serial();
    }

    id = dev.serial().mid(0, 1).toInt();
    modelCode = dev.serial().mid(1, 3);

    switch (id)
    {
    case 1:
        name = "AA-";
        type = analyzers.value(modelCode);
        break;

    case 2:
        name = "TI-";
        type = interfases.value(modelCode);
        break;

    default:
        return "Unknown";
    }

    return name+type;
}


QString ReDeviceInfo::externalSerial(const ReDeviceInfo &dev)
{
    if (dev.serial().length() < 9) {
        return "Unknown "+ dev.serial();
    }

    return dev.serial().mid(4, 5);
}

QString ReDeviceInfo::systemName() const
{
    return m_name;
}

QString ReDeviceInfo::serial() const
{
    return m_serial;
}


ReDeviceInfo::InterfaceType ReDeviceInfo::type() const
{
    return m_type;
}

quint16 ReDeviceInfo::vid() const
{
    return m_vid;
}

quint16 ReDeviceInfo::pid() const
{
    return m_pid;
}

QString ReDeviceInfo::portName() const
{
    return m_portName;
}

QList<QSerialPortInfo> bluetoothPorts()
{
    QList<QSerialPortInfo> list;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (info.description().contains("Bluetooth", Qt::CaseInsensitive)) {
            list << info;
        }
    }
    return list;
}


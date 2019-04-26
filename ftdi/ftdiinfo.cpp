#include "ftdiinfo.h"


#ifdef Q_OS_WIN
 #include <Windows.h>
 #include <ftdi/ftd2xx.h>

 QList<FtdiInfo::Info> FtdiInfo::m_lastInfo;
 QList<QSerialPortInfo> FtdiInfo::m_lastPorts;
#endif


FtdiInfo::FtdiInfo(QObject *parent) : QObject(parent)
{

}



FtdiInfo::~FtdiInfo()
{

}


#ifdef Q_OS_WIN
QList <FtdiInfo::Info> FtdiInfo::info()
{
    FT_STATUS status;
    DWORD num = 0;
    FT_DEVICE_LIST_INFO_NODE *nodes = nullptr;
    QList<QSerialPortInfo> info;
    QString desc;
    QString number;
    FT_HANDLE handle;
    LONG serialNum;
    QList<FtdiInfo::Info> ftdiInfo;
    FtdiInfo::Info item;
//    bool state;

    info = QSerialPortInfo::availablePorts();
//    state = equalLists(info, m_lastPorts);

//    if (state && !m_lastInfo.isEmpty()) {
//        return m_lastInfo;
//    }

    status = FT_CreateDeviceInfoList(&num);

//    if (status != FT_OK || num == 0)
//    {
//        return QList<FtdiInfo::Info>();
//    }

    nodes = new FT_DEVICE_LIST_INFO_NODE [num];

    status = FT_GetDeviceInfoList(nodes, &num);

//    if (status != FT_OK)
//    {
//        delete [] nodes;
//        return QList<FtdiInfo::Info>();
//    }

    for(unsigned int i = 0; i < num; ++i)
    {
        desc.clear();
        desc.append(nodes[i].Description);

        if (desc.indexOf("RigExpert") >= 0 && desc.indexOf("AA") >= 0)
        {
            number.clear();

            if (FT_Open(i, &handle) == FT_OK)
            {
                status = FT_GetComPortNumber(handle, &serialNum);

                if (status == FT_OK)
                {
                    number.setNum(serialNum);
                    number.insert(0, "COM");
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Err FT_GetComPortNumber";
                }

                FT_Close(handle);
            }

            if (number.isEmpty())
            {
                continue;
            }
            //port was found
            for (qint32 j = 0; j < info.count(); ++ j)
            {
                if (info.at(j).portName() == number)
                {
                    item.portInfo = desc;
                    item.portName = info.at(j).portName();
                    ftdiInfo.append(item);
                    break;
                }
            }
        }
    }

    m_lastPorts = info;
    m_lastInfo = ftdiInfo;

    delete [] nodes;
    return ftdiInfo;
}


bool FtdiInfo::equalLists(const QList<QSerialPortInfo> &a, const QList<QSerialPortInfo> &b)
{
    if (a.isEmpty() || b.isEmpty()) {
        return false;
    }

    if (a.count() != b.count()) {
        return false;
    }

    for (int i=0; i < a.count(); ++ i)
    {
        if (a.at(i).portName() != b.at(i).portName() ||
            a.at(i).serialNumber() != b.at(i).serialNumber()) {
            return false;
        }
    }

    return true;
}

#elif defined(Q_OS_MAC) || defined(Q_OS_LINUX)
QList <FtdiInfo::Info> FtdiInfo::info()
{
    QList<QSerialPortInfo> infoList = QSerialPortInfo::availablePorts();
    QList<FtdiInfo::Info> ftdiList;
    QSerialPortInfo info;
    FtdiInfo::Info item;
    QString desc;


    foreach (info, infoList)
    {
        desc = info.description();
        if (desc.indexOf("RigExpert") >= 0 && desc.indexOf("AA") >= 0)
        {
            item.portInfo = desc;
            item.portName = info.portName();
            ftdiList.append(item);
        }
    }

    return ftdiList;
}
#else
 #error "Add code for your OS"
#endif

#ifndef FTDIINFO_H
#define FTDIINFO_H

#include <QObject>
#include <QList>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>

#include <QLibrary>

#ifdef Q_OS_WIN
 #include <windows.h>
 #include <ftdi/ftd2xx.h>

typedef FT_STATUS (*fnFT_Open) (int deviceNumber, FT_HANDLE *pHandle);
typedef FT_STATUS (*fnFT_Close) (FT_HANDLE ftHandle);
typedef FT_STATUS (*fnFT_CreateDeviceInfoList) (DWORD*);
typedef FT_STATUS (*fnFT_GetDeviceInfoList) (FT_DEVICE_LIST_INFO_NODE *pDest, DWORD*);
typedef FT_STATUS (*fnFT_GetComPortNumber) (FT_HANDLE ftHandle, LPLONG	lpdwComPortNumber);

#endif



class FtdiInfo : public QObject
{
    Q_OBJECT
public:
    struct Info
    {
        QString portName;
        QString portInfo;
    };

    explicit FtdiInfo(QObject *parent = 0);
    virtual ~FtdiInfo();

    static QList<Info> info();

signals:

public slots:

private:   
#ifdef Q_OS_WIN
    static QList<Info> m_lastInfo;
    static QList<QSerialPortInfo> m_lastPorts;
    static bool equalLists(const QList<QSerialPortInfo> &a, const QList<QSerialPortInfo> &b);
#endif

};

#endif // FTDIINFO_H

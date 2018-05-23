#ifndef FTDIINFO_H
#define FTDIINFO_H

#include <QObject>
#include <QList>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>



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

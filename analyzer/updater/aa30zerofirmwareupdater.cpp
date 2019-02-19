#include "aa30zerofirmwareupdater.h"

static const unsigned char crc8_table[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F,
    0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E, 0x79,
    0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53,
    0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, 0x90, 0x97,
    0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1,
    0xB4, 0xB3, 0xBA, 0xBD, 0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC,
    0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88,
    0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, 0x27, 0x20, 0x29, 0x2E,
    0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04,
    0x0D, 0x0A, 0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E,
    0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8,
    0xAD, 0xAA, 0xA3, 0xA4, 0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2,
    0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56,
    0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
    0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A,
    0x33, 0x34, 0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39,
    0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F,
    0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5,
    0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1,
    0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3};

AA30ZEROFirmwareUpdater::AA30ZEROFirmwareUpdater():m_stop(false)
{

}

AA30ZEROFirmwareUpdater::~AA30ZEROFirmwareUpdater()
{

}

bool AA30ZEROFirmwareUpdater::update(const ReDeviceInfo &dev, QIODevice *fw)
{
    QByteArray arr;

    if(m_port.isOpen())
    {
        m_port.close();
    }

    m_port.setPortName(dev.portName());
    m_port.setBaudRate(115200);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setFlowControl(QSerialPort::NoFlowControl);
    m_port.open(QIODevice::ReadWrite);
    m_port.waitForReadyRead(1000);

#define BLOCK_SIZE 64
    qint64 totalsize = fw->bytesAvailable();

    int i;
    for (i = 0; i < totalsize; i += (arr.length()-4))
    {
        if(i != 0)
        {
            if(!waitAnswer())
            {
                emit progress(100);
                return false;
            }
        }
        emit progress(i*100/totalsize);

        arr = fw->read(BLOCK_SIZE);
        if(i == 0)
        {
            arr.insert(0,0x03);
        }else
        {
            arr.insert(0,0x04);
        }
        arr.append(crc8(&arr));
        unsigned char len = arr.length();
        arr.insert(0,len);
        arr.insert(0,0xAF);

        m_port.write(arr);

        QCoreApplication::processEvents();
    }

    if(!waitAnswer())
    {
        emit progress(100);
        return false;
    }

    arr.clear();
    arr.append(0xAF);
    arr.append(0x02);
    arr.append(0x06);
    arr.append(0x12);
    m_port.write(arr);
    m_port.waitForReadyRead(1000);
    arr = m_port.readAll();
    if(arr.length() >= 5)
    {
//        qDebug() << QString::number((unsigned char)arr.at(0));
//        qDebug() << QString::number((unsigned char)arr.at(1));
//        qDebug() << QString::number((unsigned char)arr.at(2));
//        qDebug() << QString::number((unsigned char)arr.at(3));
//        qDebug() << QString::number((unsigned char)arr.at(4));
        closeDevice();
        return false;
    }
//    m_port.waitForBytesWritten(10000);// waitForReadyRead(100);
    emit progress(100);
    closeDevice();
    return true;
}

void AA30ZEROFirmwareUpdater::stop()
{
    m_stop = true;
}

AA30ZEROFirmwareUpdater::FirmwareInfo AA30ZEROFirmwareUpdater::firmwareInfo(const ReDeviceInfo &dev)
{
    Q_UNUSED(dev);
    FirmwareInfo info;
    if(m_port.isOpen())
    {
        m_port.close();
    }

    m_port.setPortName(dev.portName());
    m_port.setBaudRate(115200);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setFlowControl(QSerialPort::NoFlowControl);
    m_port.open(QIODevice::ReadWrite);
    m_port.waitForReadyRead(1000);
    /******************************/

    //get info
    QByteArray arr;
    arr = m_port.readAll();
    arr.clear();
    arr.append(175);
    arr.append(2);
    arr.append(1);
    arr.append(7);

    if (m_port.write(arr) < 0)
    {
//        return false;
    }
    for(int g = 0; g < 20; ++g)
    {
        m_port.waitForReadyRead(50);
        if(m_port.bytesAvailable() >= 64)
        {
            break;
        }
        QCoreApplication::processEvents();
    }
    arr = m_port.readAll();
    if(arr.length() >= 64)
    {
        if((unsigned char)arr.at(0) == (unsigned char)175)
        {
            QByteArray test = arr;
            test.remove(0,2);
            unsigned char crc = arr.at(63);
            test.remove(61,1);
            unsigned char calcCrc = crc8(&test);
            if( ((unsigned char)arr.at(2) == 4) &&
                (crc == calcCrc) )
            {
                const char *pd;
                pd = arr.constData();
                info.magic = qFromLittleEndian<quint32>(*((quint32*)&pd[4+3]));
                info.major = qFromLittleEndian<quint32>(*((quint32*)&pd[12+3]));
                info.minor = qFromLittleEndian<quint32>(*((quint32*)&pd[16+3]));
                info.sn = qFromLittleEndian<quint32>(*((quint32*)&pd[24+3]));
                info.rev = qFromLittleEndian<quint32>(*((quint32*)&pd[28+3]));
            }
        }
    }
    return info;
}

bool AA30ZEROFirmwareUpdater::setInBootMode(ReDeviceInfo &dev)
{
    QByteArray arr;
    if (!openDevice(dev))
    {
        qDebug() << "Cant open device!";
        return false;
    }

    arr.append("\n\rBOOTLOADER\n");
    m_port.write(arr);
    m_port.waitForReadyRead(1000);
    m_port.close();
    m_port.setBaudRate(115200);
    m_port.open(QIODevice::ReadWrite);
    m_port.waitForReadyRead(500);
    /******************************/

    //get info
    arr = m_port.readAll();
    arr.clear();
    arr.append(175);
    arr.append(2);
    arr.append(1);
    arr.append(7);

    if (m_port.write(arr) < 0)
    {
        return false;
    }
    for(int g = 0; g < 20; ++g)
    {
        m_port.waitForReadyRead(50);
        if(m_port.bytesAvailable() >= 64)
        {
            break;
        }
        QCoreApplication::processEvents();
    }
    arr = m_port.readAll();
    for(int i = 0; i < 5; ++i)
    {
        if(arr.length() >= 64)
        {
            if((unsigned char)arr.at(0) == (unsigned char)175)
            {
                QByteArray test = arr;
                test.remove(0,2);
                unsigned char crc = arr.at(63);
                test.remove(61,1);
                unsigned char calcCrc = crc8(&test);
                if( ((unsigned char)arr.at(2) == 4) &&
                    (crc == calcCrc) )
                {
                    return true;
                }else
                {
                    qDebug() << "Error CRC!";
                }
            }
            else
            {
                qDebug() << ".at(0) != 175";
//                qDebug() << (unsigned char)arr.at(0);
//                qDebug() << (unsigned char)arr.at(1);
//                qDebug() << (unsigned char)arr.at(2);
//                qDebug() << (unsigned char)arr.at(3);
//                qDebug() << "-----";
                arr.remove(0,1);
                m_port.waitForReadyRead(100);
                arr += m_port.readAll();
            }
        }else
        {
            qDebug() << "Get corupted data!";
        }
    }
    qDebug() << "xz!";
    return false;
}

bool AA30ZEROFirmwareUpdater::waitAnswer()
{
    int times = 1;
    while (times < 100)
    {
        if (m_port.waitForReadyRead(100))
        {
            if(m_port.bytesAvailable() >= 4)
            {
                QByteArray arr = m_port.readAll();
                if((unsigned char)arr.at(0) == (unsigned char)175)
                {
                    QByteArray test = arr;
                    test.remove(0,2);
                    test.remove(arr.at(1)-1,1);
                    if( ((unsigned char)arr.at(2) == 7) &&
                        ((unsigned char)arr.at(arr.at(1) + 1) == crc8(&test)) )
                    {
    //                    qDebug() << QString::number((unsigned char)arr.at(0));
    //                    qDebug() << QString::number(arr.at(1));
    //                    qDebug() << QString::number(arr.at(2));
    //                    qDebug() << QString::number(arr.at(3));
//                        qDebug() << "OK";
                        return true;
                    }
                }
            }
        }

        if ((times%10) == 0) {
            QCoreApplication::processEvents();
        }

        if (m_stop) {
            return true;
        }

        ++ times;
    }
    return false;
}

bool AA30ZEROFirmwareUpdater::openDevice(const ReDeviceInfo &dev)
{
    bool state;
    if(m_port.isOpen())
    {
        m_port.close();
    }
    m_port.setPortName(dev.portName());
    if(dev.systemName().indexOf("AA-30 ZERO BOOT") >= 0)
    {
        m_port.setBaudRate(115200);
    }else if(dev.systemName().indexOf("AA-30 ZERO") >= 0)
    {
        m_port.setBaudRate(38400);
    }
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setFlowControl(QSerialPort::NoFlowControl);
    state = m_port.open(QIODevice::ReadWrite);
    if(!state)
    {
        qDebug() << m_port.errorString();
    }
    return state;
}


void AA30ZEROFirmwareUpdater::closeDevice()
{
    m_port.close();
}

unsigned char AA30ZEROFirmwareUpdater::crc8(QByteArray *buf)
{
    unsigned char crc8_calculation_register = 0;

    for(int i = 0; i < buf->length(); ++i)
    {
        crc8_calculation_register = crc8_table[crc8_calculation_register ^ (unsigned char)(buf->at(i))];
        crc8_calculation_register &= 0xFF;
    }
    return crc8_calculation_register;
}

#include "aa230firmwareupdater.h"

AA230FirmwareUpdater::AA230FirmwareUpdater(QObject *parent) :
    FirmwareUpdater(parent)
{

}

AA230FirmwareUpdater::~AA230FirmwareUpdater()
{

}

bool AA230FirmwareUpdater::update(const ReDeviceInfo &dev, QIODevice *fw)
{
    char retcode;
    qint64 retlen;
    QByteArray arr;
    int totalsize = fw->bytesAvailable();

    m_stop = false;

    m_error.clear();

    if (!openDevice(dev)) {
        m_error = textError(OpenPortError);
        return false;
    }

    if (!m_port.write((const char*)"3", 1)) {
        m_error = textError(WritePortError);
        closeDevice();
        return false;
    }

//    qDebug() << "Start uploading...";
    arr.clear();

    for (int i = 0; i <= totalsize; i += arr.length())
    {
        if (!waitAnswer()) {
            m_error = textError(TimeoutError);
            closeDevice();
            return false;
        }

        if (m_stop) {
            closeDevice();
            return true;
        }

        retlen = m_port.read(&retcode, 1);

        if (retlen <= 0) {
            m_error = textError(ReadPortError);
            closeDevice();
            return false;
        }
//        qDebug() << "retCode: " << QString::number(retcode);
        if (retcode != Complete) {
            m_error = textError((ReturnCode)retcode);
            closeDevice();
            return false;
        }

        emit progress(i*100/totalsize);
        QCoreApplication::processEvents();

        arr = fw->read(512);

        if (!arr.isEmpty()) {
            if (!m_port.write(arr)) {
                qDebug() << "Err in this place" << i << totalsize << arr.length() << m_port.errorString();
                m_error = textError(WritePortError);
                closeDevice();
                return false;
            }
        } else {
            //complete
            break;
        }
    }

    emit progress(100);
    closeDevice();
    return true;
}

void AA230FirmwareUpdater::stop()
{
    m_stop = true;
}

AA230FirmwareUpdater::FirmwareInfo AA230FirmwareUpdater::firmwareInfo(const ReDeviceInfo &dev)
{
    int probe = 0;
    FirmwareInfo info;
    memset(&info, 0, sizeof(info));

    QByteArray arr;
    if (!openDevice(dev)) {
        m_error = textError(OpenPortError);
        return info;
    }

    arr.append(0x31);

    if (m_port.write(arr) < 0) {
        m_error = textError(WritePortError);
        return info;
    }
    arr.clear();
    if (m_port.waitForReadyRead(500))
    {
        while(m_port.bytesAvailable() < sizeof(FirmwareInfo))
        {
            m_port.waitForReadyRead(50);
            ++ probe;
            if (probe > 10)
            {
                break;
            }
        }
        arr = m_port.readAll();
    }

    if (arr.isEmpty()) {
        return info;
    }

    memcpy(&info, arr.constData(), sizeof(FirmwareInfo));

    return info;
}

bool AA230FirmwareUpdater::setInBootMode(ReDeviceInfo &dev)
{
    int probe = 0;

    QByteArray arr;
    if (!openDevice(dev)) {
        m_error = textError(OpenPortError);;
        return false;
    }

    arr.append("BOOTLOADER\n");

    if (m_port.write(arr) < 0) {
        m_error = textError(WritePortError);
        closeDevice();
        return false;
    }

    //delay for device
    m_port.waitForBytesWritten(-1);
    m_port.waitForReadyRead(100);
    arr = m_port.readAll();
    /******************************/

    //get info
    arr.clear();
    arr.append(0x31);

    if (m_port.write(arr) < 0) {
        m_error = textError(WritePortError);
        return false;
    }
    arr.clear();
    if (m_port.waitForReadyRead(500))
    {
        while(m_port.bytesAvailable() < sizeof(FirmwareInfo))
        {
            m_port.waitForReadyRead(50);
            ++ probe;
            if (probe > 10) {
                break;
            }
        }

        arr = m_port.readAll();
    }

    if (arr.isEmpty()) {
        return false;
    }

    return true;
}

bool AA230FirmwareUpdater::waitAnswer()
{
    int times = 1;
    while (times < 1000)
    {
        if (m_port.waitForReadyRead(50)) {
            return true;
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

bool AA230FirmwareUpdater::openDevice(const ReDeviceInfo &dev)
{
    bool state;
    m_port.setPortName(dev.portName());
    state = m_port.open(QIODevice::ReadWrite);

    m_port.setBaudRate(/*1500000*/115200);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setFlowControl(QSerialPort::NoFlowControl);
    return state;
}


void AA230FirmwareUpdater::closeDevice()
{
    m_port.close();
}

QString AA230FirmwareUpdater::textError(ReturnCode code)
{
    QString str;

    switch(code)
    {
    case EraseError:
        str = tr("Can not erase flash memory.\n"
                "You should reconnect your analyzer,\n"
                "restart %1 and update firmware.").arg(QCoreApplication::applicationName());
        break;

    case WriteFwError:

        str = tr("Can not write data to flash memory.\n"
                "You should reconnect your analyzer,\n"
                "restart %1 and update firmware.").arg(QCoreApplication::applicationName());
        break;

    case AppSizeError:
        str =  tr("Firmware file has wrong length.\n"
                  "Try to choose another file.");
        break;

    case CrcMCUError:
        str =  tr("Firmware has incorrect CRC.\n"
                  "Try to choose another file.");
        break;

    case CrcFlashError:
        str = tr("Serial flash has incorrect CRC.\n"
                 "Try to choose another file.");
        break;

    case TimeoutMCUError:
        str =  tr("Timeout in the device.\n"
               "You should reconnect your analyzer,\n"
               "restart %1 and update firmware.").arg(QCoreApplication::applicationName());
        break;

    case FirmwareTypeError:
        str = tr("Firmware file has wrong format.\n"
              "Try to choose another file.");
        break;

    case TimeoutError:
        str = tr("Timeout.\n"
              "You should restart %1 and update firmware.").arg(QCoreApplication::applicationName());
        break;

    case OpenPortError:
        str = tr("Can not open serial port.\n"
              "You should close application which uses this port.");
        break;

    case WritePortError:
        str = tr("Can not write data to serial port.\n"
              "You should restart %1 and update firmware.").arg(QCoreApplication::applicationName());
        break;

    case ReadPortError:
        str = tr("Can not read data from serial port.\n"
                  "You should restart %1 and update firmware.").arg(QCoreApplication::applicationName());
        break;

    default:
        qDebug() << "Unknown";
    }

    return str;
}

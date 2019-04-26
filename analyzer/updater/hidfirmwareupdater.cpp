#include "hidfirmwareupdater.h"

Q_LOGGING_CATEGORY(HIDFIRMWAREUPDATER, "hid.updater")

HidFirmwareUpdater::HidFirmwareUpdater(QObject *parent) :
    FirmwareUpdater(parent),
    m_needStop(false)
{

}

HidFirmwareUpdater::~HidFirmwareUpdater()
{
}

bool HidFirmwareUpdater::update(const ReDeviceInfo &dev, QIODevice *fw)
{
    unsigned char buff[65];
    bool firstWrite = true;
    QByteArray arr;
    int hret;
    bool res = true;
    int totsize = fw->bytesAvailable();
    m_error.clear();

    if (totsize == 0) {
        m_error = tr("Firmware size = 0.");
        return false;
    }

    if (!openDevice(dev)) {
        m_error = tr("Can't open HID device.");
        return false;
    }

    m_needStop = false;

    for (int i = 0; i < totsize; i += 48)
    {
        memset(buff, 0, sizeof(buff));
        arr = fw->read(48);

        if (arr.length() <= 0) {
            break;
        }
        if (firstWrite) {
            buff[1] = BL_CMD_WRITE;
        } else {
            buff[1] = BL_CMD_DATA;
        }
        buff[2] = (unsigned char) arr.length();
        memcpy(&buff[3], arr.constData(), arr.length());
        hret = hid_write(m_handleDev, buff, sizeof(buff));

        if (hret <= 0) {
            m_error = hidError();
           // qCDebug(HIDFIRMWAREUPDATER) << Q_FUNC_INFO << "hret=" << hret;
            res = false;
            break;
        }

        emit progress(i*100/totsize);
        QCoreApplication::processEvents();
        if (m_needStop) {
            closeDevice();
            m_error = tr("Update was stopped manually.");
            return false;
        }

        if (firstWrite) {
            res = waitAnswer();
            firstWrite = false;
            if (!res) {
                break;
            }
        }
    }

    emit progress(100);

    if (res) {
        //check firmware
        memset(buff, 0, sizeof(buff));
        buff[1] = BL_CMD_CHECK;

        hret = hid_write(m_handleDev, buff, sizeof(buff));
        if (hret > 0) {
            res = waitAnswer();
        } else {
            m_error = hidError();
            qCWarning(HIDFIRMWAREUPDATER) << "Wrong res";
            res = false;
        }

        if (res) {
            //start application
            memset(buff, 0, sizeof(buff));
            buff[1] = BL_CMD_START;
            hid_write(m_handleDev, buff, sizeof(buff));
        }
    }

    closeDevice();

    return res;
}

bool HidFirmwareUpdater::waitAnswer()
{
    int times = 0;
    unsigned char buff[65];
    int ret;

    while (times < 1000)
    {
        QCoreApplication::processEvents();
        ret = hid_read_timeout(m_handleDev, buff, sizeof(buff), 60);

        if (ret == 0) {
            ++ times;
            if (m_needStop) {
                m_error = tr("Update was stopped manually.");
                return false;
            }

        } else if (ret > 0) {
            if (buff[0] == BL_CMD_OK) {
                return true;
            } else {
                m_error = tr("Bootloader has returned BL_CMD_ERROR.");
                return false;
            }
        } else {
            qCWarning(HIDFIRMWAREUPDATER) << "Some read error";
            m_error = hidError();
            return false;
        }
    }
    return false;
}




HidFirmwareUpdater::FirmwareInfo HidFirmwareUpdater::firmwareInfo(const ReDeviceInfo &dev)
{
    FirmwareInfo info;
    unsigned char buff[65] = {0};
    int ret;

    memset(&info, 0, sizeof(FirmwareInfo));

    if (!openDevice(dev)) {
        return info;
    }

    buff[0] = 0;
    buff[1] = BL_CMD_GET_ID;

    ret = hid_write(m_handleDev, buff, sizeof(buff));

    if (ret <= 0) {
        closeDevice();
        return info;
    }

    ret = hid_read_timeout(m_handleDev, buff, sizeof(buff), 100);

    if (ret > 0) {
        memcpy((char*)&info, &buff[6], 60);
    }

    closeDevice();
    return info;
}


bool HidFirmwareUpdater::openDevice(const ReDeviceInfo &dev)
{
    struct hid_device_info *devs, *cur_dev;
    QString serial;
    bool ret = false;

    /* start found of hid-devices*/
    devs = hid_enumerate(dev.vid(), dev.pid());

    /*all found devices places in dynamic list*/
    cur_dev = devs;
    while (cur_dev)
    {
        serial = QString::fromWCharArray(cur_dev->serial_number);

        if (serial == dev.serial()) {
            break;
        }

        cur_dev = cur_dev->next;
    }

    if (cur_dev && (m_handleDev = hid_open_path(cur_dev->path))){
        ret = true;
    }

    /*delete list*/
    hid_free_enumeration(devs);

    return ret;
}

void HidFirmwareUpdater::closeDevice()
{
    if (m_handleDev) {
        hid_close(m_handleDev);
        m_handleDev = NULL;
    }
}

void HidFirmwareUpdater::stop()
{
    m_needStop = true;
}

QString HidFirmwareUpdater::hidError()
{
    const wchar_t* perr;
    perr = hid_error(m_handleDev);
    return QString::fromWCharArray(perr);
}

bool HidFirmwareUpdater::setInBootMode(ReDeviceInfo &dev)
{
    unsigned char buff[64] = {0};
    bool state = true;
    int ret;

    if (dev.vid() == 0x0483 && dev.pid() == 0xA1DA) {
        return true;
    }

    if (!openDevice(dev)) {
        m_error = tr("Can't open HID device.");
        return false;
    }

    memset(buff, 0, sizeof(buff));
    buff[0] = 1;
    memcpy(&buff[1], "RESET", strlen("RESET"));
    ret = hid_write(m_handleDev, buff, sizeof(buff));

    if (ret <= 0) {
        m_error = hidError();
        state = false;
    }

    closeDevice();


    QList<ReDeviceInfo> list;
    int attempts = 0;
    state = false;

    while (attempts < 60)
    {
        QThread::msleep(200);

        list = ReDeviceInfo::availableDevices(ReDeviceInfo::HID);

        foreach (ReDeviceInfo item, list)
        {
            if (item.pid() == 0xA1DA && item.serial().right(5) == dev.serial().right(5)) {
                dev = item;
                state = true;
                attempts = 1000;
                break;
            }
        }

        ++ attempts;
    }

    return state;
}

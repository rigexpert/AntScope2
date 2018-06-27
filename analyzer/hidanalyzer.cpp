#include "hidanalyzer.h"

hidAnalyzer::hidAnalyzer(QObject *parent) : QObject(parent),
      m_parseState(1),
      m_analyzerModel(0),
      m_chartTimer(NULL),
      m_ok(false),
      m_isMeasuring(false),
      m_analyzerPresent(false),
      m_bootMode(false)
{
    m_hidDevice = NULL;
    m_checkTimer = new QTimer(this);
    QObject::connect(m_checkTimer, SIGNAL(timeout()), this, SLOT(checkTimerTick()));
    m_checkTimer->start(1000);

    m_chartTimer = new QTimer(this);
    QObject::connect(m_chartTimer, SIGNAL(timeout()), this, SLOT(timeoutChart()));
    m_chartTimer->start(30);

    m_sendTimer = new QTimer(this);
    QObject::connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(continueMeasurement()));

    m_hidReadTimer = new QTimer(this);
    QObject::connect(m_hidReadTimer, SIGNAL(timeout()), this, SLOT(hidRead()));
    m_hidReadTimer->start(1);
}

hidAnalyzer::~hidAnalyzer()
{
    if(m_checkTimer != NULL)
    {
        m_checkTimer->stop();
        delete m_checkTimer;
        m_checkTimer = NULL;
    }
    if(m_chartTimer != NULL)
    {
        m_chartTimer->stop();
        delete m_chartTimer;
        m_chartTimer = NULL;
    }
    if(m_sendTimer != NULL)
    {
        m_sendTimer->stop();
        delete m_sendTimer;
        m_sendTimer = NULL;
    }
    if(m_hidReadTimer != NULL)
    {
        m_hidReadTimer->stop();
        delete m_hidReadTimer;
        m_hidReadTimer = NULL;
    }
    disconnect();
}

QString hidAnalyzer::getVersion(void) const
{
    return m_version;
}

QString hidAnalyzer::getRevision() const
{
    return m_revision;
}

QString hidAnalyzer::getSerial(void) const
{
    return m_serialNumber;
}

int hidAnalyzer::getModel(void) const
{
    return m_analyzerModel;
}

void hidAnalyzer::nonblocking (int nonblock)
{
    hid_set_nonblocking(m_hidDevice, nonblock);
}

bool hidAnalyzer::searchAnalyzer(bool arrival)
{
    bool result = false;
    if(arrival)//add device
    {
        struct hid_device_info *devs, *cur_dev;
        devs = hid_enumerate(0x0, 0x0);
        cur_dev = devs;

        for(int i = 0; i < 100; i++)
        {
            if(!cur_dev)
            {
              break;
            }
            if( cur_dev->vendor_id == RE_VID && cur_dev->product_id == RE_PID)
            {
                QString number = QString::fromWCharArray(cur_dev->serial_number);
                number.remove(4,5);
                int serialNumber = number.toInt();
                if((serialNumber == PREFIX_SERIAL_NUMBER_AA35) ||
                   (serialNumber == PREFIX_SERIAL_NUMBER_AA35_ZOOM))
                {
                    m_serialNumber = QString::fromWCharArray(cur_dev->serial_number);
                    connect(RE_VID, RE_PID);
                    result = true;
                    if(!result)
                    {
                        return false;
                    }
                    for(quint32 i = 0; i < QUANTITY; ++i)
                    {
                        if(names[i] == "AA-35 ZOOM")
                        {
                            m_analyzerModel = i;
                            emit analyzerFound(i);
                            break;
                        }
                    }
                    break;
                }else if((serialNumber == PREFIX_SERIAL_NUMBER_AA55) ||
                         (serialNumber == PREFIX_SERIAL_NUMBER_AA55_ZOOM))
                {
                    m_serialNumber = QString::fromWCharArray(cur_dev->serial_number);
                    connect(RE_VID, RE_PID);
                    result = true;
                    if(!result)
                    {
                        return false;
                    }
                    for(quint32 i = 0; i < QUANTITY; ++i)
                    {
                        if(names[i] == "AA-55 ZOOM")
                        {
                            m_analyzerModel = i;
                            emit analyzerFound(i);
                            break;
                        }
                    }
                    break;
                }else if(serialNumber == PREFIX_SERIAL_NUMBER_AA230_ZOOM)
                {
                    m_serialNumber = QString::fromWCharArray(cur_dev->serial_number);
                    connect(RE_VID, RE_PID);
                    result = true;
                    if(!result)
                    {
                        return false;
                    }
                    for(quint32 i = 0; i < QUANTITY; ++i)
                    {
                        if(names[i] == "AA-230 ZOOM")
                        {
                            m_analyzerModel = i;
                            emit analyzerFound(i);
                            break;
                        }
                    }
                    break;
                }
            }else if((cur_dev->vendor_id == RE_BOOT_VID) && (cur_dev->product_id == RE_BOOT_PID))
            {
                QString number = QString::fromWCharArray(cur_dev->serial_number);
                number.remove(4,5);
                int serialNumber = number.toInt();
                if((serialNumber == PREFIX_SERIAL_NUMBER_AA35) ||
                   (serialNumber == PREFIX_SERIAL_NUMBER_AA35_ZOOM))
                {
                    m_serialNumber = QString::fromWCharArray(cur_dev->serial_number);
                    connect(RE_BOOT_VID, RE_BOOT_PID);
                    for(quint32 i = 0; i < QUANTITY; ++i)
                    {
                        if(names[i] == "AA-35 ZOOM")
                        {
                            m_bootMode = true;
                            m_analyzerModel = i;
                            break;
                        }
                    }
                    break;
                }else if((serialNumber == PREFIX_SERIAL_NUMBER_AA55) ||
                         (serialNumber == PREFIX_SERIAL_NUMBER_AA55_ZOOM))
                {
                    m_serialNumber = QString::fromWCharArray(cur_dev->serial_number);
                    connect(RE_BOOT_VID, RE_BOOT_PID);
                    for(quint32 i = 0; i < QUANTITY; ++i)
                    {
                        if(names[i] == "AA-55 ZOOM")
                        {
                            m_bootMode = true;
                            m_analyzerModel = i;
                            break;
                        }
                    }
                    break;
                }
            }
            cur_dev = cur_dev->next;
        }
        hid_free_enumeration(devs);
        return result;
    }else//delete device
    {
        struct hid_device_info *devs, *cur_dev;

        devs = hid_enumerate(0x0, 0x0);
        cur_dev = devs;

        bool result = true;
        int i;
        for(i = 0; i < 100; i++)
        {
            if(!cur_dev)
            {
              break;
            }
            if( cur_dev->vendor_id == RE_VID && cur_dev->product_id == RE_PID &&  QString::fromWCharArray(cur_dev->serial_number) == m_serialNumber )
            {
                result = false;
                break;
            }else if(cur_dev->vendor_id == RE_BOOT_VID && cur_dev->product_id == RE_BOOT_PID &&  QString::fromWCharArray(cur_dev->serial_number) == m_serialNumber)
            {
                result = false;
                break;
            }
            cur_dev = cur_dev->next;
        }
        if(result)
        {
            m_bootMode = false;
            m_analyzerModel = 0;
            emit analyzerDisconnected();
            disconnect();
        }
        hid_free_enumeration(devs);
        return false;
    }
}

bool hidAnalyzer::connect(quint32 vid, quint32 pid)
{
    if(m_hidDevice != NULL)
    {
        return false;
    }

    wchar_t* serial = new wchar_t[m_serialNumber.length()+1];
    m_serialNumber.toWCharArray(serial);
    serial[m_serialNumber.length()] = 0;
    m_hidDevice = hid_open(vid, pid, serial);//m_hidDevice = hid_open(RE_VID, RE_PID, serial);

    if(m_hidDevice != NULL)
    {
        hid_set_nonblocking(m_hidDevice, 1);
        m_parseState = VER;
        sendData("VER\r\n");
        return true;
    }
    else
    {
        return false;
    }
}

bool hidAnalyzer::disconnect(void)
{
    if(m_hidDevice != NULL)
    {
        hid_close(m_hidDevice);
        m_hidDevice = NULL;
        return true;
    }
    else
    {
        return false;
    }
}

void hidAnalyzer::checkTimerTick ()
{
    if(!m_isMeasuring)
    {
        searchAnalyzer((bool)!m_analyzerModel);
    }
}

void hidAnalyzer::on_screenshotComplete()
{
    m_isMeasuring = false;
}

void hidAnalyzer::on_measurementComplete()
{
    m_isMeasuring = false;
}

void hidAnalyzer::sendData(QString data)
{
    if(m_hidDevice == NULL)
    {
        return;
    }
    unsigned char buf[REPORT_SIZE] = {0};
    int size = data.length();
    if(size)
    {
        buf[0] = ANTSCOPE_REPORT;
        buf[1] = (unsigned char)size;
        for (int i = 0; i < size; i++)
        {
            buf[i+2] = data[i].toLatin1();
        }
        hid_write(m_hidDevice, buf, REPORT_SIZE);
    }
}

void hidAnalyzer::startMeasure(int fqFrom, int fqTo, int dotsNumber)
{
    static qint32 state = 1;
    static QString FQ;
    static QString SW;
    static QString FRX;

    quint32 center;
    quint32 band;


    if(dotsNumber > 0)
    {
        state = 1;
    }

    switch(state)
    {
    case 1:
        m_isMeasuring = true;
        m_parseState = WAIT_DATA;
        if(dotsNumber != 0)
        {
            band = fqTo - fqFrom;
            center = band/2 + fqFrom;
        }else
        {
            band = 0;
            center = fqFrom;
        }
        FQ  = "FQ"  + QString::number(center) + 0x0D;
        SW  = "SW"  + QString::number(band) + 0x0D;
        FRX = "FRX" + QString::number(dotsNumber) + 0x0D;
        m_ok = false;
        sendData(FQ);
        m_sendTimer->start(10);
        state++;
        break;
    case 2:
        if(m_ok)
        {
            m_ok = false;
            sendData(SW);
            state++;
        }
        break;
    case 3:
        if(m_ok)
        {
            m_ok = false;
            sendData(FRX);
            state = 1;
            m_sendTimer->stop();
        }
        break;
    default:
        break;
    }
}

void hidAnalyzer::continueMeasurement()
{
    startMeasure(0,0,0);
}

void hidAnalyzer::timeoutChart()
{
    quint32 len;
    QStringList stringList;
    QString str;

    len = m_stringList.length();

    if(len >=1)
    {
        str = m_stringList.takeFirst();
        QString tempString;
        for(qint32 i = 0; i < str.length(); ++i)
        {
            if(str.at(i) == ',')
            {
                stringList.append(tempString);
                tempString.clear();
            }else
            {
                tempString.append(str.at(i));
            }
            if(i == str.length()-1)
            {
                stringList.append(tempString);
                tempString.clear();
            }
        }
        while(stringList.length() >= 3)
        {
            rawData data;
            str = stringList.takeFirst();
            data.fq = str.toDouble(0);

            str = stringList.takeFirst();
            data.r = str.toDouble(0);

            str = stringList.takeFirst();
            data.x = str.toDouble(0);

            emit newData(data);
        }
    }
}

void hidAnalyzer::hidRead (void)
{
    if(m_hidDevice == NULL)
    {
        return;
    }
    unsigned char readBuff[64];
    int read = hid_read(m_hidDevice, readBuff, 64);

    if(read > 0)
    {
        if(readBuff[0] == ANTSCOPE_REPORT)
        {
            for(int i = 0; i < readBuff[1]; i++)
            {
                m_incomingBuffer.append(readBuff[i+2]);
            }
            int ret = parse(m_incomingBuffer);
            m_incomingBuffer.remove(0,ret);
        }
    }
}

qint32 hidAnalyzer::parse (QByteArray arr)
{
    quint32 retVal = 0;
    if(m_parseState == WAIT_SCREENSHOT_DATA)
    {
        int pos = arr.indexOf("screencomp: ");
        if(pos == 0)
        {
            return 12;
        }else
        {
            emit analyzerScreenshotDataArrived(arr);
            retVal += arr.length();
            return retVal;
        }
    }else if(m_parseState == WAIT_ANALYZER_UPDATE)
    {
        //
    }else if(arr.length() >= 1)
    {
        QStringList stringList;
        QString tempString;
        for(qint32 i = 0; i < arr.length(); ++i)
        {
            if(arr.at(i) == '\n')
            {
                retVal++;
                stringList.append(tempString);
                tempString.clear();
            }else
            {
                tempString.append(arr.at(i));
            }
        }
        if(stringList.isEmpty())
        {
            return 0;
        }
        for(int i = 0; i < stringList.length(); ++i)
        {
            QString str = stringList.at(i);
            int r = str.indexOf('\r');
            if(r)
            {
                str.remove(r,1);
                retVal++;
            }
            if(str.isEmpty())
            {
                continue;
            }
            if(str == "OK")
            {
                m_ok = true;
                continue;
            }
            if(str == "ERROR")
            {
                continue;
            }
            if(m_parseState == VER)
            {
                for(quint32 i = QUANTITY-1; i > 0; i--)
                {
                    if(str.indexOf(names[i]) >= 0 )
                    {
                        int pos = names[i].length() + 1;
                        if(str.length() >= pos+3)
                        {
                            m_version.clear();
                            m_version.append(str.at(pos));
                            m_version.append(str.at(pos+1));
                            m_version.append(str.at(pos+2));
                        }
                        pos = str.indexOf("REV ");
                        if(pos >= 0)
                        {
                            pos += 4;
                            int revLen = str.length() - pos;
                            m_revision.clear();
                            for(int t = 0; t < revLen; ++t)
                            {
                                m_revision.append(str.at(pos+t));
                            }
                        }else
                        {
                            m_revision.clear();
                            m_revision.append('1');
                        }
                        retVal += str.length();
                        break;
                    }
                }
            }else if(m_parseState == WAIT_DATA)
            {
                m_stringList.append(str);
                retVal += str.length();
                str.clear();
            }else if(m_parseState == WAIT_ANALYZER_DATA)
            {
                emit analyzerDataStringArrived(str);
                retVal += str.length();
                str.clear();
            }
        }
        return retVal;
    }
    return 0;
}

void hidAnalyzer::getAnalyzerData()
{
    m_parseState = WAIT_ANALYZER_DATA;
    m_incomingBuffer.clear();
    sendData("FLASHH\r");
}

void hidAnalyzer::getAnalyzerData(QString number)
{
    m_parseState = WAIT_DATA;
    m_incomingBuffer.clear();
    QString str = "FLASHFRX" + number + "\r";
    sendData(str);
}

void hidAnalyzer::makeScreenshot()
{
    m_parseState = WAIT_SCREENSHOT_DATA;
    nonblocking(true);
    m_incomingBuffer.clear();
    QString str = "screenshot\r";
    sendData(str);
}

bool hidAnalyzer::waitAnswer()
{
    int times = 0;
    unsigned char buff[65];
    int ret;

    while (times < 1000)
    {
        QCoreApplication::processEvents();
        if(!m_hidDevice)
        {
            return false;
        }
        ret = hid_read_timeout(m_hidDevice, buff, 65, 60);
        if (ret == 0)
        {
            ++ times;

        } else if (ret > 0)
        {
            if (buff[0] == BL_CMD_OK)
            {
                return true;
            } else
            {
                return false;
            }
        }
    }
    return false;
}

bool hidAnalyzer::update (QIODevice *fw)
{
    m_hidReadTimer->stop();
    if(!m_bootMode)
    {
        unsigned char buff[64] = {0};

        //Enter to BOOT mode-----------------------------
        memset(buff, 0, sizeof(buff));
        buff[0] = 1;
        memcpy(&buff[1], "RESET", strlen("RESET"));
        hid_write(m_hidDevice, buff, sizeof(buff));
        while(1)//for(int i = 0; i < 565535; ++i)
        {
            if(m_bootMode)
                break;
            QCoreApplication::processEvents();
        }
        if(!m_bootMode)
        {
            QMessageBox::warning(NULL,"Warning","Can't enter to boot mode!");
            return false;
        }
    }
    //Updating--------------------------------------
    unsigned char buff[65];
    bool firstWrite = true;
    QByteArray arr;
    int hret;
    bool res = true;
    int totsize = fw->bytesAvailable();

    if (totsize == 0) {
        return false;
    }

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
        hret = hid_write(m_hidDevice, buff, sizeof(buff));

        if (hret <= 0)
        {
            return false;
            break;
        }

        emit updatePercentChanged(i*100/totsize);
        QCoreApplication::processEvents();

        if (firstWrite)
        {
            res = waitAnswer();
            firstWrite = false;
            if (!res)
            {
                emit updatePercentChanged(100);
                return false;
                break;
            }
        }
    }
    emit updatePercentChanged(100);
    if (res)
    {
        //check firmware
        memset(buff, 0, sizeof(buff));
        buff[1] = BL_CMD_CHECK;

        hret = hid_write(m_hidDevice, buff, sizeof(buff));
        if (hret > 0)
        {
            res = waitAnswer();
        } else
        {
            res = false;
        }

        if (res)
        {
            //start application
            memset(buff, 0, sizeof(buff));
            buff[1] = BL_CMD_START;
            hid_write(m_hidDevice, buff, sizeof(buff));
            QMessageBox::information(NULL,tr("Finish"),tr("Successfully updated!"));
        }else
        {
            QMessageBox::warning(NULL,tr("Warning"),tr("Update failed!"));
        }
    }
    m_hidReadTimer->start(1);
    return res;
}

void hidAnalyzer::stopMeasure()
{
    sendData("off\r");
}

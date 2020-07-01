#include "hidanalyzer.h"
#include "customanalyzer.h"
#include <QtConcurrent/QtConcurrentRun>
#include <QThread>
#include "analyzer.h"

QString hidError(hid_device* _device)
{
    const wchar_t* perr;
    perr = hid_error(_device);
    return QString::fromWCharArray(perr);
}

hidAnalyzer::hidAnalyzer(QObject *parent) : QObject(parent),
      m_parseState(1),
      m_analyzerModel(0),
      m_chartTimer(nullptr),
      m_ok(false),
      m_isMeasuring(false),
      m_isContinuos(false),
      m_analyzerPresent(false),
      m_bootMode(false),
      m_devices(nullptr),
      m_futureRefresh(nullptr),
      m_watcherRefresh(nullptr)
{
    m_hidDevice = nullptr;
    m_checkTimer = new QTimer(this);
    QObject::connect(m_checkTimer, SIGNAL(timeout()), this, SLOT(checkTimerTick()));
    m_checkTimer->start(1000);

    m_chartTimer = new QTimer(this);
    QObject::connect(m_chartTimer, SIGNAL(timeout()), this, SLOT(timeoutChart()));
    m_chartTimer->start(5);

    m_sendTimer = new QTimer(this);
    QObject::connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(continueMeasurement()));

    m_hidReadTimer = new QTimer(this);
    QObject::connect(m_hidReadTimer, SIGNAL(timeout()), this, SLOT(hidRead()));
    m_hidReadTimer->start(1);

    QTimer::singleShot(1000, this, &hidAnalyzer::startResresh);
}

#if 0
void hidAnalyzer::startResresh()
{
    qDebug() << "hidAnalyzer::startResresh()";
    if (future != nullptr) {
        watcher->disconnect();
        delete watcher;
        delete future;
        watcher = nullptr;
        future = nullptr;
    }
    future = new QFuture<struct hid_device_info*>;
    watcher = new QFutureWatcher<struct hid_device_info*>;

    QObject::connect(watcher, SIGNAL(finished()), this, SLOT(refreshReady()));

    *future = QtConcurrent::run(this, &hidAnalyzer::refreshThreadStarted, this);
    watcher->setFuture(*future);
}
#endif
void hidAnalyzer::startResresh()
{
    //qDebug() << "hidAnalyzer::startResresh()";
    if (m_futureRefresh == nullptr) {
        m_futureRefresh = new QFuture<struct hid_device_info*>;
        m_watcherRefresh = new QFutureWatcher<struct hid_device_info*>;
        QObject::connect(m_watcherRefresh, SIGNAL(finished()), this, SLOT(refreshReady()));
    }

    *m_futureRefresh = QtConcurrent::run(this, &hidAnalyzer::refreshThreadStarted);
    m_watcherRefresh->setFuture(*m_futureRefresh);
}




hidAnalyzer::~hidAnalyzer()
{
    if (m_devices != nullptr) {
        hid_free_enumeration(m_devices);
        m_devices = nullptr;
    }
    delete m_futureRefresh;
    m_futureRefresh = nullptr;
    delete m_watcherRefresh;
    m_watcherRefresh = nullptr;

    if(m_checkTimer != nullptr)
    {
        m_checkTimer->stop();
        delete m_checkTimer;
        m_checkTimer = nullptr;
    }
    if(m_chartTimer != nullptr)
    {
        m_chartTimer->stop();
        delete m_chartTimer;
        m_chartTimer = nullptr;
    }
    if(m_sendTimer != nullptr)
    {
        m_sendTimer->stop();
        delete m_sendTimer;
        m_sendTimer = nullptr;
    }
    if(m_hidReadTimer != nullptr)
    {
        m_hidReadTimer->stop();
        delete m_hidReadTimer;
        m_hidReadTimer = nullptr;
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
    //qDebug() << "hidAnalyzer::searchAnalyzer";
    bool result = false;
    if(arrival)//add device
    {
        struct hid_device_info *devs, *cur_dev;

        QMutexLocker locker(&m_mutexSearch);
        devs = m_devices;
        if (devs == nullptr)
            return false;

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
//                if (number.isEmpty()) {
//                    number = "165000011";
//                }
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
                }else if(serialNumber == PREFIX_SERIAL_NUMBER_AA650_ZOOM)
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
                        if(names[i] == "AA-650 ZOOM")
                        {
                            m_analyzerModel = i;
                            emit analyzerFound(i);
                            break;
                        }
                    }
                    break;
                }else if(serialNumber == PREFIX_SERIAL_NUMBER_AA1500_ZOOM)
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
                        if(names[i] == "AA-1500 ZOOM")
                        {
                            m_analyzerModel = i;
                            emit analyzerFound(i);
                            break;
                        }
                    }
                    break;
                }else if(serialNumber == PREFIX_SERIAL_NUMBER_AA230_STICK)
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
                        if(names[i] == "Stick 230")
                        {
                            m_analyzerModel = i;
                            emit analyzerFound(i);
                            break;
                        }
                    }
                    break;
                }else if(serialNumber == PREFIX_SERIAL_NUMBER_STICK_PRO)
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
                        if(names[i] == "StickPro")
                        {
                            m_analyzerModel = i;
                            emit analyzerFound(i);
                            break;
                        }
                    }
                    break;
                }else if(serialNumber == PREFIX_SERIAL_NUMBER_AA2000)
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
                        if(names[i] == "AA-2000")
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
                }else if((serialNumber == PREFIX_SERIAL_NUMBER_AA2000))
                {
                    m_serialNumber = QString::fromWCharArray(cur_dev->serial_number);
                    connect(RE_BOOT_VID, RE_BOOT_PID);
                    for(quint32 i = 0; i < QUANTITY; ++i)
                    {
                        if(names[i] == "AA-2000")
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
        //hid_free_enumeration(devs);
        return result;
    }else//delete device
    {
        struct hid_device_info *devs, *cur_dev;
        if (!m_mutexSearch.tryLock())
            return false;

        devs = m_devices;
        if (devs == nullptr) {
            m_mutexSearch.unlock();
            return false;
        }

        cur_dev = devs;
        while (cur_dev != nullptr) {
            if (QString::fromWCharArray(cur_dev->serial_number) == m_serialNumber) {
                if (cur_dev->vendor_id == RE_VID && cur_dev->product_id == RE_PID ||
                    cur_dev->vendor_id == RE_BOOT_VID && cur_dev->product_id == RE_BOOT_PID ) {
                    m_mutexSearch.unlock();
                    return false;
                }
            }
            cur_dev = cur_dev->next;
        }
        m_mutexSearch.unlock();

        m_bootMode = false;
        m_analyzerModel = 0;
        emit analyzerDisconnected();
        disconnect();

        return false;
    }
}

bool hidAnalyzer::connect(quint32 vid, quint32 pid)
{
    if(m_hidDevice != nullptr)
    {
        return false;
    }

    wchar_t* serial = new wchar_t[m_serialNumber.length()+1];
    m_serialNumber.toWCharArray(serial);
    serial[m_serialNumber.length()] = 0;
    m_hidDevice = hid_open(vid, pid, serial);//m_hidDevice = hid_open(RE_VID, RE_PID, serial);

    if(m_hidDevice != nullptr)
    {
        hid_set_nonblocking(m_hidDevice, 1);
        m_parseState = WAIT_VER;
        sendData("VER\r\n");
        sendData("FULLINFO\r\n");
        return true;
    }
    else
    {
        return false;
    }
}

bool hidAnalyzer::disconnect(void)
{
    if(m_hidDevice != nullptr)
    {
        hid_close(m_hidDevice);
        m_hidDevice = nullptr;
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
        //startResresh();
    }
}

void hidAnalyzer::on_screenshotComplete()
{
    m_parseState = WAIT_NO;
    m_isMeasuring = false;
}

void hidAnalyzer::on_measurementComplete()
{
    m_isMeasuring = false;
}

void hidAnalyzer::sendData(QString data)
{
    if(m_hidDevice == nullptr)
    {
        return;
    }
    //qDebug() << "hidAnalyzer::sendData: " << data;
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

void hidAnalyzer::startMeasureOneFq(qint64 fqFrom, int dotsNumber)
{
    startMeasure(fqFrom, fqFrom, dotsNumber);
}

void hidAnalyzer::startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber)
{
    static qint32 state = 1;
    static QString FQ;
    static QString SW;
    static QString FRX;
    qint64 center;
    qint64 band;

    if(dotsNumber > 0)
    {
        state = 1;
    }

    switch(state)
    {
    case 1:
        m_isMeasuring = true;
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
        FRX = (m_isFRX ? "FRX" : "EFRX") + QString::number(dotsNumber) + 0x0D;
        m_ok = false;
        sendData(FQ);
        m_sendTimer->start(10);
        state++;
        break;
    case 2:
        if(m_ok)
        {
            m_isMeasuring = true;
            m_ok = false;
            sendData(SW);
            state++;
        }
        break;
    case 3:
        if(m_ok)
        {
            m_isMeasuring = true;
            m_ok = false;
            sendData(FRX);
            m_parseState = m_isFRX ? WAIT_DATA : WAIT_USER_DATA;
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

    if (!m_isMeasuring)
    {
        m_stringList.clear();
        return;
    }

    if (m_parseState == WAIT_USER_DATA)
    {
        timeoutChartUser();
        return;
    }

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
            bool ok;
            rawData data;

            str = stringList.takeFirst();
            data.fq = str.toDouble(&ok);
            if (!ok)
                qDebug() << "***** ERROR: " << str;
            str = stringList.takeFirst();
            data.r = str.toDouble(&ok);
            if (!ok ||  qIsNaN(data.r))
                qDebug() << "***** ERROR: " << str;

            str = stringList.takeFirst();
            data.x = str.toDouble(&ok);
            if (!ok ||  qIsNaN(data.x))
                qDebug() << "***** ERROR: " << str;

            emit newData(data);
        }
    }
}

void hidAnalyzer::timeoutChartUser()
{
    quint32 len = m_stringList.length();
    if(len >=1)
    {
        QString str = m_stringList.takeFirst();

        bool isHeader = str[0] == '#';
        str.replace('#', ' ');
        QStringList fields = str.split(',');

        if (isHeader) {
            emit newUserDataHeader(fields);
            return;
        }

        rawData rdata;
        UserData udata;
        bool ok;
        QString field = fields.takeFirst();
        rdata.fq = field.toDouble(&ok);
        if (!ok) {
            qDebug() << "***** ERROR: " << str;
            return;
        }
        udata.fq = rdata.fq;
        field = fields.takeFirst();
        rdata.r = field.toDouble(&ok);
        if (!ok) {
            qDebug() << "***** ERROR: " << str;
            return;
        }
        field = fields.takeFirst();
        rdata.x = field.toDouble(&ok);
        if (!ok) {
            qDebug() << "***** ERROR: " << str;
            return;
        }
        while (!fields.isEmpty()) {
            udata.values.append(fields.takeFirst().toDouble(&ok));
            if (!ok) {
                qDebug() << "***** ERROR: " << str;
                return;
            }
        }
        emit newUserData(rdata, udata);
    }
}


void hidAnalyzer::hidRead (void)
{
    if(m_hidDevice == nullptr)
    {
        return;
    }
    unsigned char readBuff[64];
    int read = hid_read(m_hidDevice, readBuff, 64);
    m_mutexRead.lock();
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
    m_mutexRead.unlock();
}

qint32 hidAnalyzer::parse (QByteArray arr)
{
    //qDebug() << "hidAnalyzer::parse() " << arr;
    quint32 retVal = 0;
    if (getParseState() == WAIT_CALFIVEKOHM_START)
    {
        setParseState(WAIT_CALFIVEKOHM);
        Analyzer* analyzer = qobject_cast<Analyzer*>(parent());
        int pos = arr.indexOf("\r\n");
        QString result = arr.left(pos);
        int num = result.toInt();
        result = " START";
        emit analyzer->updateAutocalibrate5(num, result);
        return (result.length()+2);
    } else if (getParseState() == WAIT_CALFIVEKOHM)
    {
        Analyzer* analyzer = qobject_cast<Analyzer*>(parent());
        int pos = arr.indexOf("\r\n");
        QString result = arr.left(pos);
        if (result.contains("OK") || result.contains("ERROR")) {
            setParseState(WAIT_NO);
            emit analyzer->stopAutocalibrate5();
        } else {
            int num = result.toInt();
            Analyzer* analyzer = qobject_cast<Analyzer*>(parent());
            emit analyzer->updateAutocalibrate5(num, result);
        }
        return (result.length()+2);

    } else if (getParseState() == WAIT_LICENSE_LIST)
    {
        int pos = arr.indexOf("\r\n");
        QString result = arr.left(pos);
        if (result == "OK") {
            setParseState(WAIT_NO);
            return 4;
        }
        Analyzer* analyzer = qobject_cast<Analyzer*>(parent());
        if (analyzer != nullptr) {
            emit analyzer->licensesList(result);
        }
        return result.length()+2;
    } else if (getParseState() == WAIT_LICENSE_REQUEST)
    {
        int pos = arr.indexOf("\r\n");
        QString result = arr.left(pos);
        Analyzer* analyzer = qobject_cast<Analyzer*>(parent());
        if (analyzer != nullptr) {
            emit analyzer->licenseRequest(result);
        }
        setParseState(WAIT_NO);
        return result.length()+2;
    } else if (getParseState() == WAIT_LICENSE_APPLY1)
    {
        setParseState(WAIT_LICENSE_APPLY2);
        QString cmd = "ALICS-" + m_license.mid(m_license.length()/2);
        sendData(cmd);
        retVal += arr.length();
        return retVal;
    } else if (getParseState() == WAIT_LICENSE_APPLY2)
    {
        setParseState(WAIT_NO);
        int pos = arr.indexOf("\r\n");
        QString result = arr.left(pos);
        Analyzer* analyzer = qobject_cast<Analyzer*>(parent());
        if (analyzer != nullptr) {
            m_license.clear();
            emit analyzer->licenseApplyResult(result);
        }
        retVal += arr.length();
        return retVal;
    } else if(m_parseState == WAIT_SCREENSHOT_DATA)
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

        //qDebug() << "-----------------------";
        //qDebug() << stringList;

        for(int i = 0; i < stringList.length(); ++i)
        {
            QString str = stringList.at(i);
            retVal += str.length();
            str.replace("\r", "");
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
//                if (m_parseState == WAIT_DATA)
//                    emit signalMeasurementError();
                continue;
            }
            if(m_parseState == WAIT_VER)
            {
                if (str.indexOf("MAC\t") == 0) {
                    emit signalFullInfo(str);
                    continue;
                } else if (str.indexOf("SN\t") == 0) {
                    emit signalFullInfo(str);
                    continue;
                } else if (str.contains("NAME")) {
                    // skip FULLINFO field `NAME` to keep version obtained in VER
                    continue;
                }

                for(quint32 i = QUANTITY-1; i > 0; i--)
                {
                    QString analyzer = names[i];
                    int namePos = str.indexOf(analyzer);
                    if(namePos >= 0 )
                    {
                        int pos = analyzer.length() + 1;
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
                            //qDebug() << "FULLINFO: " << str;
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
                        break;
                    }
                }
            }else if(m_parseState == WAIT_DATA || m_parseState == WAIT_USER_DATA)
            {
                m_stringList.append(str);
                str.clear();
            }else if(m_parseState == WAIT_ANALYZER_DATA)
            {
                emit analyzerDataStringArrived(str);
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
    //qDebug() << "========== hidAnalyzer::makeScreenshot()";
    sendData(str);
    //QThread::currentThread()->msleep(100);
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

void hidAnalyzer::preUpdate ()
{
    hid_close(m_hidDevice);
    m_hidDevice = nullptr;
    searchAnalyzer(true);
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
        qDebug() << "RESET: " << hidError(m_hidDevice);

        QTimer::singleShot(5000, this, [this]() {
            this->preUpdate();
        });
        while(1)//for(int i = 0; i < 565535; ++i)
        {
            if(m_bootMode)
                break;
            QCoreApplication::processEvents();
        }
        if(!m_bootMode)
        {
            QMessageBox::warning(nullptr,tr("Warning"),tr("Can't enter to boot mode!"));
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
            QString err = hidError(m_hidDevice);
            qDebug() << err;
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
            QMessageBox::information(nullptr,tr("Finish"),tr("Successfully updated!"));
        }else
        {
            QMessageBox::warning(nullptr,tr("Warning"),tr("Update failed!"));
        }
    }
    preUpdate();
    m_hidReadTimer->start(1);
    return res;
}

void hidAnalyzer::stopMeasure()
{
    sendData("off\r");
    m_isMeasuring = false;
}

void hidAnalyzer::setIsMeasuring (bool isMeasuring)
{
    m_isMeasuring = isMeasuring;
}

struct hid_device_info* hidAnalyzer::refreshThreadStarted()
{
    struct hid_device_info *devs = hid_enumerate(0x0, 0x0);
    return devs;
}

void hidAnalyzer::refreshReady()
{
    //qDebug() << "hidAnalyzer::refreshReady()";
    QTimer::singleShot(1000, this, &hidAnalyzer::startResresh);

    struct hid_device_info* devs = m_futureRefresh->result();
    if (m_mutexSearch.tryLock()) {
        if (m_devices != nullptr) {
            hid_free_enumeration(m_devices);
        }
        m_devices = devs;
        m_mutexSearch.unlock();
    }
}

void hidAnalyzer::applyLicense(QString _license)
{
    m_license = _license;
    setParseState(WAIT_LICENSE_APPLY1);
    QString cmd = "ALICF-" + _license.left(_license.length()/2) + "\r\n";;
    sendData(cmd);
}



#include "com_analyzer.h"
#include "customanalyzer.h"
#include <qserialport.h>
#include "analyzer.h"
#include "AA55BTPacket.h"

extern bool g_bAA55modeNewProtocol;

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



ComAnalyzer::ComAnalyzer(QObject *parent) : BaseAnalyzer(parent),
    m_chartTimer(NULL),
    //m_ok(false),
    m_updateOK(false),
    m_updateErr(0),
    m_analyzerPresent(false),
    m_autoDetectMode(true)
{
    m_type = ReDeviceInfo::Serial;
    m_comPort = new QSerialPort(this);

    m_chartTimer = new QTimer(this);
    connect(m_chartTimer, SIGNAL(timeout()), this, SLOT(timeoutChart()));
    m_chartTimer->start(10);

    //m_sendTimer = new QTimer(this);
    //connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(continueMeasurement()));
    QTimer::singleShot(10000, this, SLOT(searchAnalyzer()));

    m_pingTimer = new QTimer(this);
    connect(m_pingTimer, &QTimer::timeout, this, &ComAnalyzer::handlePing);
}

ComAnalyzer::~ComAnalyzer()
{
    m_chartTimer->stop();
    delete m_chartTimer;
    m_chartTimer = NULL;

    if(m_comPort->isOpen())
    {
        m_comPort->close();
    }
    delete m_comPort;
    m_comPort = NULL;
}

bool ComAnalyzer::openComPort(const QString& portName, quint32 portSpeed)
{
    if(m_comPort->isOpen())
    {
        disconnect(m_comPort, SIGNAL(readyRead()), this, SLOT(dataArrived()));
        m_comPort->close();
    }
    m_comPort->setPortName(portName);
    m_comPort->setBaudRate(portSpeed);//QSerialPort::Baud38400);
    m_comPort->setFlowControl(QSerialPort::NoFlowControl);
    m_comPort->setDataBits(QSerialPort::Data8);
    m_comPort->setParity(QSerialPort::NoParity);
    m_comPort->setStopBits(QSerialPort::OneStop);

    bool result = m_comPort->open(QSerialPort::ReadWrite);
    if (!result) {
        QString str = m_comPort->errorString();
        //qDebug() << "ComAnalyzer::openComPort: " << portName << " " << str << " [" << m_comPort->error() << "]";
        // TODO show dialog
        // ...
    }
    else {
        // << "ComAnalyzer::openComPort: " << portName << (m_comPort ? " opened" : "NOT opened");
        connect(m_comPort, SIGNAL(readyRead()), this, SLOT(dataArrived()));
    }
    return result;
}


void ComAnalyzer::closeComPort()
{
    if(m_comPort != NULL)
    {
        if(m_comPort->isOpen())
        {
            disconnect(m_comPort, SIGNAL(readyRead()), this, SLOT(dataArrived()));
            m_comPort->close();
        }
    }
    m_bAA55mode = false;
    m_bAA55modeNewProtocol = false;
    g_bAA55modeNewProtocol = false;
}

void ComAnalyzer::dataArrived()
{
    m_lastReadTimeMS = QDateTime::currentMSecsSinceEpoch();
    QByteArray ar = m_comPort->readAll();
    m_incomingBuffer += ar;

    //qDebug() << "com dataArrived: " << m_lastReadTimeMS << QString::fromLatin1(ar);

    int count = parse(m_incomingBuffer);
    m_incomingBuffer.remove(0, count);
}

qint32 ComAnalyzer::parse (QByteArray arr)
{
    QString rrr = QString(arr);
    //qDebug() << "ComAnalyzer::parse " << rrr;

    quint32 retVal = 0;
    QString model = AnalyzerParameters::getName();
    if(m_parseState == WAIT_SCREENSHOT_DATA)
    {
        int pos = arr.indexOf("screencomp: ");
        if(pos == 0 || pos == 2)
        {
            return 12 + pos;
        }else
        {
            pos = arr.indexOf("screenshot: ");
            if(pos == 0 || pos == 2)
            {
                return 12 + pos;
            }else
            {
                emit analyzerScreenshotDataArrived(arr);
                retVal += arr.length();
                return retVal;
            }
        }
    }else if(m_parseState == WAIT_ANALYZER_UPDATE)
    {
        if(model == "AA-230 ZOOM")
        {
            if(arr.length() >= 1)
            {
                if(arr.at(0) == 0)
                {
                    m_updateOK = true;
                }else
                {
                    if(!m_updateErr)
                    {
                        m_updateErr = (qint32)arr.at(0);
                        QString warn = qApp->applicationName() + tr(" can not update firmware.\n");
                        QString error = textError((ReturnCode) m_updateErr);
                        if(error.isEmpty())
                        {
                            return arr.length();
                        }
                        warn += tr("Error: ") + error;
                        QMessageBox::warning(NULL, tr("Warning"), warn);
                    }else
                    {
                        return arr.length();
                    }
                }
            }
        }else if(model == "AA-30 ZERO" || model == "AA-30.ZERO")
        {
            if(arr.length() >= 4)
            {
                if((unsigned char)arr.at(0) == (unsigned char)175)
                {
                    QByteArray test = arr;
                    test.remove(0,2);
                    test.remove(arr.at(1)-1,1);
                    if( ((unsigned char)arr.at(2) == 7) &&
                        ((unsigned char)arr.at(arr.at(1) + 1) == crc8(&test)) )
                    {
                        m_updateOK = true;
                    }
                }
                return arr.length();
            }
        }
    }else if(arr.length() >= 1)
    {
        QStringList stringList;
        QString tempString;
//------- Belarus EU1KY analyzer support --------------------
        for(qint32 i = 0; i < arr.length(); ++i)
        {
            if(arr.at(i) == '\r')
            {
                retVal++;
                stringList.append(tempString);
                tempString.clear();
            } else if (arr.at(i) == '\n')
            {
                retVal++;
            } else
            {
                tempString.append(arr.at(i));
            }
        }
//-------------------------
        if(stringList.isEmpty())
        {
            return 0;
        }
        for(int i = 0; i < stringList.length(); ++i)
        {
            QString str = stringList.at(i);
            retVal += str.length();
            str.replace("\r", "");
            if(str.isEmpty())
            {
                continue;
            }
            //if(str == "OK")
            if(str.contains("OK"))
            {
                m_ok = true;
                if (m_parseState == WAIT_ANALYZER_DATA)
                    m_parseState = WAIT_VER;
                continue;
            }
            if(str == "ERROR")
            {
                if (m_parseState == WAIT_DATA)
                    emit signalMeasurementError();
                continue;
            }
            if(m_parseState == WAIT_DATA || m_parseState == WAIT_USER_DATA)
            {
                m_stringList.append(str);
                str.clear();
            }else if(m_isTakeData && m_parseState == WAIT_ANALYZER_DATA)
            {
                emit analyzerDataStringArrived(str);
                str.clear();
            } else if(m_parseState == WAIT_VER)
            {
                if (m_isTakeData)
                    continue;

                if (str.indexOf("MAC\t") == 0) {
                    emit signalFullInfo(str);
                    continue;
                } else if (str.indexOf("SN\t") == 0) {
                    emit signalFullInfo(str);
                    continue;
                }
#ifdef NEW_ANALYZER
                AnalyzerParameters* param = AnalyzerParameters::current();
                if (param != nullptr) {
                    QString analyzer = param->name();
                    //int namePos = str.indexOf(analyzer);
                    int namePos = analyzer=="COMPORT" ? str.indexOf("AA-") : str.indexOf(analyzer);
                    if(namePos >= 0 )
                    {
                        int name_end = str.indexOf(' ');
                        QString name = str.mid(namePos, name_end - namePos);
                        AnalyzerParameters* param = AnalyzerParameters::byName(name);
                        AnalyzerParameters::setCurrent(param);
                        if(m_analyzerModel != 0)
                        {
                            m_analyzerPresent = true;
                        }else
                        {
                            // ??? SKIP PING response
                            // if (m_analyzerModel == idx)
                            //    break;
                            //m_analyzerModel = idx;
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
                            if (analyzer.contains("AA-55")) {
                                m_bAA55mode = true;
                                bool ok = true;
                                int ver = m_version.toInt(&ok);
                                if (ok && ver > 125) {
                                    m_bAA55modeNewProtocol = true;
                                    g_bAA55modeNewProtocol = true;
                                }
                            }
                            emit analyzerFound (param->index());
                            //QTimer::singleShot(1000, this, SLOT(checkAnalyzer()));
                        }
                    }
                }
#else
                for(quint32 idx = QUANTITY-1; idx > 0; idx--)
                {
                    if(str.indexOf(names[idx]) >= 0 )
                    {
                        if(m_analyzerModel != 0)
                        {
                            m_analyzerPresent = true;
                        }else
                        {
                            // ??? SKIP PING response
                            // if (m_analyzerModel == idx)
                            //    break;
                            m_analyzerModel = idx;
                            int pos = names[idx].length() + 1;
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
                            if (names[m_analyzerModel].contains("AA-55")) {
                                m_bAA55mode = true;
                                bool ok = true;
                                int ver = m_version.toInt(&ok);
                                if (ok && ver > 125) {
                                    m_bAA55modeNewProtocol = true;
                                    g_bAA55modeNewProtocol = true;
                                }
                            }
                            emit analyzerFound (m_analyzerModel);
                            //QTimer::singleShot(1000, this, SLOT(checkAnalyzer()));
                        }
                        break;
                    }
                }
#endif
            }
        }
        return retVal;
    }
    return 0;
}

quint32 ComAnalyzer::compareStrings (QString arr, QString arr1)
{
    if(arr.length() < arr1.length())
    {
        return false;
    }
    int result;
    for(result = 0; result < arr.length(); result++)
    {
        if(result < arr1.length())
        {
            if(arr.at(result) == QChar(10) || arr.at(result) == QChar(13))
            {
                arr.remove(result,1);
            }else if(arr.at(result) != arr1.at(result))
            {
                break;
            }
        }
    }
    return result;
}

#ifdef NEW_ANALYZER
void ComAnalyzer::searchAnalyzer()
{
}
#else
void ComAnalyzer::searchAnalyzer()
{
    static bool analyzerDetected = false;
    static int messageWasShown = 5;
    static int state = 0;

    if (m_isMeasuring)
        return;

    if(!m_autoDetectMode)
    {
        if( m_analyzerModel != 0)
        {
            return;
        }

qDebug() << "ComAnalyzer::searchAnalyzer() 0";

        if(state == 0)
        {
            state = 1;
            openComPort(m_serialPortName,38400);
            if(m_comPort->isOpen())
            {
                versionRequest();
                m_lastReadTimeMS = QDateTime::currentMSecsSinceEpoch();
                m_pingTimer->start(1000);
                qDebug() << "com::searchAnalyzer 1: m_pingTimer->start(1000)";
            }

        }else if(state == 1)
        {
            state = 0;
            openComPort(m_serialPortName,115200);
            if(m_comPort->isOpen())
            {
                versionRequest();
                m_lastReadTimeMS = QDateTime::currentMSecsSinceEpoch();
                m_pingTimer->start(1000);
                qDebug() << "com::searchAnalyzer 2: m_pingTimer->start(1000)";
            }
        }
    }else
    {
        if( m_analyzerModel != 0)
        {
            QTimer::singleShot(4000, this, SLOT(searchAnalyzer()));
            return;
        }
        if(analyzerDetected && (--messageWasShown == 0))
        {
            //messageWasShown = -1;
            QMessageBox::information(NULL,tr("Analyzer detected"),tr("The program has detected an analyzer connected to your PC, but it is either turned off or is not in the PC mode. The program will now work in the offline mode (i.e. without the analyzer).\n\nIf you still want the program to talk to the analyzer, turn it on and enter the PC mode."));
            QTimer::singleShot(4000, this, SLOT(searchAnalyzer()));
            return;
        }
        //qDebug() << "messageWasShown" << messageWasShown;

        analyzerDetected = false;
        closeComPort();
        QList<ReDeviceInfo> list;
        list = ReDeviceInfo::availableDevices(ReDeviceInfo::Serial);
        if(list.isEmpty())
        {
            QTimer::singleShot(4000, this, SLOT(searchAnalyzer()));
            return;
        }else
        {
            QString name = ReDeviceInfo::deviceName(list.at(0));
            for(quint32 i = QUANTITY-1; i > 0; i--)
            {
                if(names[i].indexOf(name) >= 0 )
                {
                    bool opened = false;
                    if(name == "AA-230 ZOOM")
                    {
                        opened = openComPort(list.at(0).portName(),115200);
                        //{ TODO debug BLE
                        //opened = openComPort(m_serialPortName,115200);
                        //}
                    } else if (name == "AA-30 ZERO" || name == "AA-30.ZERO")
                    {
                        opened = openComPort(m_serialPortName,38400);

                    } else
                    {
                        opened = openComPort(list.at(0).portName(),38400);
                    }
                    if (opened) {
                        analyzerDetected = true;
                        versionRequest();
                        m_lastReadTimeMS = QDateTime::currentMSecsSinceEpoch();
                        m_pingTimer->start(1000);
                        qDebug() << "com::searchAnalyzer 3: m_pingTimer->start(1000)";
                    } else {
                        QTimer::singleShot(4000, this, SLOT(searchAnalyzer()));
                    }
                    return;
                }
            }
            QTimer::singleShot(4000, this, SLOT(searchAnalyzer()));
        }
    }
}
#endif
qint64 ComAnalyzer::sendData(const QByteArray& data)
{
    //qDebug() << "ComAnalyzer::sendData> " << data;
    qint64 res = m_comPort->write(data);
    return res;
}

qint64 ComAnalyzer::sendCommand(const QString& data)
{
    //qDebug() << "comAnalyzer::sendData> " << data;
    qint64 res = m_comPort->write(data.toLocal8Bit());
    return res;
}

/*
void ComAnalyzer::startMeasureOneFq(qint64 fqFrom, int dotsNumber, bool frx)
{
    startMeasure(fqFrom, fqFrom, dotsNumber, frx);
}


void ComAnalyzer::startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber, bool frx)
{
    Q_UNUSED (frx)

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
        setIsMeasuring(true);
        m_parseState = WAIT_DATA;
        if(dotsNumber != 0)
        {
            band = fqTo - fqFrom;
            center = band/2 + fqFrom;

            //qDebug() << "ComAnalyzer::startMeasure: " << band << dotsNumber << (band / dotsNumber);
            AA55BTPacket::start(fqFrom, band / dotsNumber);
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
            m_parseState = m_isFRX ? WAIT_DATA : WAIT_USER_DATA;
            state = 1;
            m_sendTimer->stop();
        }
        break;
    default:
        break;
    }
}

void ComAnalyzer::stopMeasure()
{
    sendData("off\r");
    m_isMeasuring = false;
}
*/
void ComAnalyzer::timeoutChart()
{
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

    if (m_stringList.isEmpty())
        return;

    if (m_bAA55modeNewProtocol) {
        str = m_stringList.takeFirst();
        if (str.size() != 14) {
            QString err = QString("z%1\r\n").arg(AA55BTPacket::dot());
            AA55BTPacket::setWait(true);
            sendData(err.toLocal8Bit());
            return;
        }

        // RRRRXXXXNNNNCC
        AA55BTPacket packet(str);
        if (!packet.valid()) {
            QString err = QString("z%1\r\n").arg(packet.dot());
            packet.setWait(true);
            sendData(err.toLocal8Bit());
        } else {
            if (packet.waitForLost() && packet.id() != packet.dot()) {
                if (packet.disconnect()) {
                    on_changedSerialPort(m_serialPortName);
                    return;
                }
                if (packet.repeat()) {
                    QString err = QString("z%1\r\n").arg(packet.dot());
                    sendData(err.toLocal8Bit());
                }
                return; // skip queued packets
            }
          RawData data;
          data.fq = packet.fq()/1000000.0;
          data.r = packet.r();
          data.x = packet.x();

          //qDebug() << "com::timeoutChart ADD" << packet.id() << QString::number(data.fq, 'f') << data.r << data.x;

          packet.setNext();
          emit newData(data);
        }
        return;
    }

    str = m_stringList.takeFirst();
    stringList = str.split(',');
    int count = (stringList.size() / 3) * 3;
    for (int idx=0; idx<count; idx+=3)
    {
        bool ok=true;
        RawData data;
        data.fq = stringList[idx+0].toDouble(&ok);
        if (!ok)
            qDebug() << "***** ERROR: " << stringList[idx+0];
        data.r  = stringList[idx+1].toDouble(&ok);
        if (!ok)
            qDebug() << "***** ERROR: " << stringList[idx+1];
        data.x  = stringList[idx+2].toDouble(&ok);
        if (!ok)
            qDebug() << "***** ERROR: " << stringList[idx+2];
        emit newData(data);
    }
}

void ComAnalyzer::timeoutChartUser()
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

        RawData rdata;
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

/*
void ComAnalyzer::continueMeasurement()
{
    startMeasure(0,0,0);
}

void ComAnalyzer::getAnalyzerData()
{
    setTakeData(true);
    setIsMeasuring(true);
    m_parseState = WAIT_ANALYZER_DATA;
    m_incomingBuffer.clear();
    sendData("FLASHH\r");
}

void ComAnalyzer::getAnalyzerData(QString number)
{
    setTakeData(true);
    m_parseState = WAIT_DATA;
    m_incomingBuffer.clear();
    QString str = "FLASHFRX" + number + "\r";
    sendData(str);
}
*/
void ComAnalyzer::makeScreenshot()
{
    setIsMeasuring(true);
    m_parseState = WAIT_SCREENSHOT_DATA;
    m_incomingBuffer.clear();
#ifndef NEW_ANALYZER
    QString model = CustomAnalyzer::customized() ? CustomAnalyzer::currentPrototype() : names[m_analyzerModel];
#else
    QString model = AnalyzerParameters::getName();
#endif
    if(model == "AA-230 ZOOM" || model == "AA-55 ZOOM")
    {
        QString str = "screenshot\r";
        sendData(str.toLocal8Bit());
    }
}
/*
void ComAnalyzer::on_screenshotComplete()
{
    m_parseState = WAIT_NO;
    setIsMeasuring(false);
}

void ComAnalyzer::on_measurementComplete()
{
    if(!m_isContinuos)
    {
        setIsMeasuring(false);
    }
}
*/
bool ComAnalyzer::waitAnswer()
{
    int times = 1;
#ifndef NEW_ANALYZER
    QString model = CustomAnalyzer::customized() ? CustomAnalyzer::currentPrototype() : names[m_analyzerModel];
#else
    QString model = AnalyzerParameters::getName();
#endif
    if(model == "AA-230 ZOOM")
    {
        while (times < 100)
        {
            m_comPort->waitForReadyRead(50);
            if(m_updateOK)
            {
                m_updateOK = false;
                return true;
            }else if(m_updateErr != 0)
            {
                return false;
            }

            if ((times%10) == 0) {
                QCoreApplication::processEvents();
            }

            ++ times;
        }
    }else if(model == "AA-30 ZERO" || model == "AA-30.ZERO")
    {
        while (times < 100)
        {
            m_comPort->waitForReadyRead(50);
            if(m_updateOK)
            {
                m_updateOK = false;
                return true;
            }else if(m_updateErr != 0)
            {
                return false;
            }

            if ((times%10) == 0) {
                QCoreApplication::processEvents();
            }
            ++ times;
        }
    }
    return false;
}

bool ComAnalyzer::update (QIODevice *fw)
{
#ifndef NEW_ANALYZER
    QString model = CustomAnalyzer::customized() ? CustomAnalyzer::currentPrototype() : names[m_analyzerModel];
#else
    QString model = AnalyzerParameters::getName();
#endif
    if(model == "AA-230 ZOOM")
    {
        setIsMeasuring(true);
        m_parseState = WAIT_ANALYZER_UPDATE;
        m_incomingBuffer.clear();

        //enter to bootloader
        sendData("BOOTLOADER\n");
        m_comPort->waitForReadyRead(500);

        //updating----------------------------

        sendData("4");
        m_comPort->waitForReadyRead(500);
        QString name = m_comPort->portName();
        m_comPort->close();
        openComPort(name,1500000);


        QByteArray arr;
        qint64 totalsize = fw->bytesAvailable();
        sendData("3");
        m_comPort->waitForReadyRead(500);

        for (int i = 0; i <= totalsize; i += arr.length())
        {
            if(!waitAnswer())
            {
                emit updatePercentChanged(100);
                setIsMeasuring(false);
                return false;
            }

            emit updatePercentChanged(i*100/totalsize);
            QCoreApplication::processEvents();

            arr = fw->read(512);

            if (!arr.isEmpty())
            {
                m_comPort->write(arr);
            } else
            {
                //complete
                break;
            }
        }
        QMessageBox::information(NULL,tr("Finish"),tr("Successfully updated!"));
        m_comPort->close();
        openComPort(name,115200);
        setIsMeasuring(false);
    }else if(model == "AA-30 ZERO" || model == "AA-30.ZERO")
    {
        QByteArray arr;
        QString name;
        setIsMeasuring(true);
        m_parseState = WAIT_ANALYZER_UPDATE;
        m_incomingBuffer.clear();

        //enter to bootloader
        sendData("BOOTLOADER\n");
        m_comPort->waitForReadyRead(1000);
        //updating----------------------------
         name = m_comPort->portName();
        if(m_comPort->baudRate() != 115200)
        {
            m_comPort->close();
            openComPort(name,115200);
        }
        m_comPort->waitForReadyRead(1000);

#define BLOCK_SIZE 64
        qint64 totalsize = fw->bytesAvailable();

        int i;
        for (i = 0; i < totalsize; i += (arr.length()-4))
        {
            if(i != 0)
            {
                if(!waitAnswer())
                {
                    QMessageBox::warning(NULL,tr("Error"),tr("Error while update, please try again."));
                    emit updatePercentChanged(100);
                    emit updatePercentChanged(0);
                    setIsMeasuring(false);
                    return false;
                }
            }
            emit updatePercentChanged(i*100/totalsize);

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
            arr.insert(0,(unsigned char)0xAF);

            m_comPort->write(arr);

            QCoreApplication::processEvents();
        }

        if(!waitAnswer())
        {
            emit updatePercentChanged(100);
            setIsMeasuring(false);
            return false;
        }

        arr.clear();
        arr.append((unsigned char)0xAF);
        arr.append((unsigned char)0x02);
        arr.append((unsigned char)0x06);
        arr.append((unsigned char)0x12);
        m_comPort->write(arr);

        emit updatePercentChanged(100);

        QMessageBox::information(NULL,tr("Finish"),tr("Successfully updated!"));
        m_comPort->close();
        openComPort(name,38400);
        setIsMeasuring(false);
        emit aa30updateComplete();
    }
    return false;
}

QString ComAnalyzer::textError(ReturnCode code)
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
        break;
    }

    return str;
}

unsigned char ComAnalyzer::crc8(QByteArray *buf)//, unsigned int length)
{
    unsigned char crc8_calculation_register = 0;

    for(int i = 0; i < buf->length(); ++i)
    {
        crc8_calculation_register = crc8_table[crc8_calculation_register ^ (unsigned char)(buf->at(i))];
        crc8_calculation_register &= 0xFF;
    }
    /* Return the CRC computed value */
    return crc8_calculation_register;
}

void ComAnalyzer::on_changedAutoDetectMode(bool state)
{
    m_autoDetectMode = state;
    closeComPort();
#ifndef NEW_ANALYZER
    m_analyzerModel = 0;
#else
    AnalyzerParameters::setCurrent(nullptr);
#endif
    emit analyzerDisconnected();
}

void ComAnalyzer::on_changedSerialPort(QString portName)
{
    m_serialPortName = portName;
    closeComPort();
    m_analyzerModel = 0;
    emit analyzerDisconnected();
}

void ComAnalyzer::versionRequest()
{
    if (m_isTakeData)
        return;

    setParseState(WAIT_VER);

    // TODO
    sendData("\r\nVER\r\n");
    //sendData("VER\n");
    // TODO

}

void ComAnalyzer::sendPing()
{
    m_bWaitingPing = true;
    if (m_bAA55mode || m_bAA55modeNewProtocol)
        sendData("VER\r\n");
    else
        sendData("ping\r\n");
}

void ComAnalyzer::handlePing()
{
    if (!(m_bAA55mode || m_bAA55modeNewProtocol)) {
        m_pingTimer->stop();
        return;
    }

    long cur = QDateTime::currentMSecsSinceEpoch();
    //qDebug() << "ComAnalyzer::handlePing()" << cur << (cur - m_lastReadTimeMS);
    if ((cur - m_lastReadTimeMS) >= PING_TIMEOUT_MS) {
        if (m_bWaitingPing) {
            // error
#ifndef NEW_ANALYZER
    m_analyzerModel = 0;
#else
    AnalyzerParameters::setCurrent(nullptr);
#endif
            m_pingTimer->stop();
            emit analyzerDisconnected();
        } else {
            sendPing();
        }
    } else {
        m_bWaitingPing = false;
    }
}

bool ComAnalyzer::connectAnalyzer()
{
    static int messageWasShown = 5;
    static int state = 0;

    AnalyzerParameters* analyzer = AnalyzerParameters::byIndex(SelectionParameters::selected.modelIndex);
    if (analyzer == nullptr)
        return false;

    QString _serialPortName = SelectionParameters::selected.id;

    // TODO
    //_serialPortName = "COM4";
    //

    bool opened = false;
    QString name = analyzer->name();
    if(name == "AA-230 ZOOM")
    {
        opened = openComPort(_serialPortName,115200);
        //{ TODO debug BLE
        //opened = openComPort(m_serialPortName,115200);
        //}
    } else if (name == "AA-30 ZERO" || name == "AA-30.ZERO")
    {
        opened = openComPort(_serialPortName,38400);

    } else
    {
        opened = openComPort(_serialPortName,38400);
    }
    if (opened) {
        m_serialPortName = _serialPortName;
        versionRequest();
        m_lastReadTimeMS = QDateTime::currentMSecsSinceEpoch();
        m_pingTimer->start(1000);
        //qDebug() << "ComAnalyzer::connectAnalyzer " << name << "opened " << m_serialPortName;
    }
    return false;
}

void ComAnalyzer::disconnectAnalyzer()
{

}

bool ComAnalyzer::refreshConnection()
{
    return connectAnalyzer();
}


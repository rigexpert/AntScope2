#include "nanovna_analyzer.h"
#include <qserialport.h>
#include <QMessageBox>

// static members
QList<QSerialPortInfo> NanovnaAnalyzer::m_listNanovnaPorts;
bool NanovnaAnalyzer::m_isConnected = false;

NanovnaAnalyzer::NanovnaAnalyzer(QObject *parent) : QObject(parent),
    m_parseState(WAIT_NANO_NO),
    m_isMeasuring(false),
    m_ok(false),
    m_analyzerPresent(false)
{
    m_comPort = new QSerialPort(this);

    qDebug() << "NanovnaAnalyzer::NanovnaAnalyzer";
    //QTimer::singleShot(5000, this, SLOT(searchAnalyzer()));
}

NanovnaAnalyzer::~NanovnaAnalyzer()
{
    if(m_comPort->isOpen())
    {
        m_comPort->close();
    }
    delete m_comPort;
    m_comPort = NULL;
    qDebug() << "NanovnaAnalyzer::~NanovnaAnalyzer";
}

QString NanovnaAnalyzer::getVersion() const
{
//    if(m_version.isEmpty())
//        return "0";
    return m_version;
}

QString NanovnaAnalyzer::getRevision() const
{
    return m_revision;
}

QString NanovnaAnalyzer::getSerial() const
{
    return m_serialNumber;
}

bool NanovnaAnalyzer::openComPort(const QString& portName, quint32 portSpeed)
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

    connect(m_comPort, SIGNAL(readyRead()), this, SLOT(dataArrived()));
    connect(m_comPort, &QSerialPort::aboutToClose, this, &NanovnaAnalyzer::portClosed);
    bool result = m_comPort->open(QSerialPort::ReadWrite);
    if (!result) {
        QString str = m_comPort->errorString();
        qDebug() << "comAnalyzer::openComPort: " << portName << " " << str << " [" << m_comPort->error() << "]";
        // TODO show dialog
        // ...
    }
    m_isConnected = result;
    return result;
}

void NanovnaAnalyzer::closeComPort()
{
    if(m_comPort != NULL)
    {
        if(m_comPort->isOpen())
        {
            disconnect(m_comPort, SIGNAL(readyRead()), this, SLOT(dataArrived()));
            m_comPort->close();
            m_isConnected = false;
            emit analyzerDisconnected();
        }
    }
}

void NanovnaAnalyzer::dataArrived()
{
    QByteArray ar = m_comPort->readAll();
    m_incomingBuffer += ar;

    //qDebug() << "NANO dataArrived: " << QString::fromLatin1(ar);

    int count = parse(m_incomingBuffer);
    m_incomingBuffer.remove(0, count);
}

qint32 NanovnaAnalyzer::parse (QByteArray arr)
{
    int ret = 0;

    QString str(arr);
    int pos=0;
    while((pos=str.indexOf("\r\n")) != -1 || str =="ch> ") {
        QString data = str.left(pos);
        ret += data.length()+2;
        str = str.mid(pos+2);
        if (data.trimmed().isEmpty())
            continue;

        //qDebug() << "parse: state   " << getParseState() << data;

        if (getParseState() == WAIT_NANO_VER) {
            if (data.contains("info"))
                setParseState(WAIT_NANO_VER_COMPLETE);
        } else if (getParseState() == WAIT_NANO_VER_COMPLETE) {
            if (data.contains("ch>")) {
                setParseState(WAIT_NANO_NO);
            } else {
                if (data.contains("Board:")) {
                    QString board = data.replace("Board:", "");
                    emit analyzerFound(board.trimmed());
                }
            }
        } else if (getParseState() == WAIT_NANO_SWEEP) {
            if (data.contains("ch>")) {
                setParseState(WAIT_NANO_FQ);
                sendData("frequencies\r\n");
                return arr.size();
            }
        } else if (getParseState() == WAIT_NANO_FQ) {
            if (data.contains("ch>")) {
                setParseState(WAIT_NANO_DATA);
                sendData("data\r\n");
                return arr.size();
            } else {
              m_listFQ << data;
            }
        } else if (getParseState() == WAIT_NANO_DATA) {
            if (data.contains("ch>")) {
                setParseState(WAIT_NANO_NO);
                //qDebug() << "NanovnaAnalyzer::parse STOP";
                emit completeMeasurement();
                if (getContinuos()) {
                    startMeasure(m_fqFrom, m_fqTo, m_dotsNumber);
                } else {
                    setIsMeasuring(false);
                    setContinuos(false);
                }
                return arr.size();
            } else {
                if (m_isMeasuring) {
                    if (m_listFQ.isEmpty()) {
                        emit completeMeasurement();
                    } else {
                      QString s1p = m_listFQ.takeFirst() + " " + data;
                      rawData raw = toRawData(s1p);
                      if (raw.fq > 0) {
                          emit newData(raw);
                      } else {
                        //qDebug() << "NanovnaAnalyzer::parse SKIP dot " << raw.fq;
                      }
                    }
                }
            }
        } else {
            emit dataReceived(data);
        }
    }
    return ret;
}

void NanovnaAnalyzer::searchAnalyzer()
{
    QList<QSerialPortInfo> listPorts = QSerialPortInfo::availablePorts();
    for (int idx=0; idx<listPorts.size(); idx++) {
        QSerialPortInfo info = listPorts.at(idx);

        QString portName = info.portName();
        QString systemLocation =info.systemLocation() ;
        QString description = info.description() ;
        QString manufacturer = info.manufacturer() ;
        QString serialNumber = info.serialNumber() ;

        quint16 vendorIdentifier = info.vendorIdentifier() ;
        quint16 productIdentifier = info.productIdentifier() ;

        bool hasVendorIdentifier = info.hasVendorIdentifier() ;
        bool hasProductIdentifier = info.hasProductIdentifier() ;
        QString _vendorIdentifier; _vendorIdentifier.setNum(
                    hasVendorIdentifier?vendorIdentifier:-1, 16);
        QString _productIdentifier; _productIdentifier.setNum(
                    hasProductIdentifier?productIdentifier:-1, 16);
        qDebug() << " portName: " << portName << "\n"
                 << "systemLocation: " << systemLocation << "\n"
                 << "description: " << description << "\n"
                 << "manufacturer: " << manufacturer << "\n"
                 << "serialNumber: " << serialNumber << "\n"
                 << "vendorIdentifier: " << _vendorIdentifier << "\n"
                 << "productIdentifier: " << _productIdentifier << "\n"
                 << "------------------------------------"
                    ;

        info.description();
        QString name = info.description();
        //ui->comboBox->addItem(name);
        if (vendorIdentifier == NANOVNA_VID && productIdentifier == NANOVNA_PID) {
           openComPort(portName, 115200);
           if (!m_comPort->isOpen()) {
               QTimer::singleShot(2000, this, [this]() {
                   this->searchAnalyzer();
               });
           } else {
               emit analyzerFound(0);
           }
           return;
        }
    }
}

/*
void NanovnaAnalyzer::checkAnalyzer()
{
    static qint32 state = 0;
    if(!m_isMeasuring)
    {
        if(state == 0)
        {
            m_analyzerPresent = false;
            m_parseState = WAIT_VER;
            versionRequest();
            state++;
            QTimer::singleShot(1000, this, SLOT(checkAnalyzer()));
        }else if(state == 1)
        {
            if(m_analyzerPresent == false)
            {
                m_parseState = WAIT_VER;
                versionRequest();
                //sendData("FULLINFO\r\n");
                state++;
            }else
            {
                state--;
            }
            QTimer::singleShot(1000, this, SLOT(checkAnalyzer()));
        }else if(state == 2)
        {
            state = 0;
            if(m_analyzerPresent == false)
            {
                //m_analyzerModel = 0;
                emit analyzerDisconnected();
            }else
            {
                QTimer::singleShot(1000, this, SLOT(checkAnalyzer()));
            }
        }
    }else
    {
        state = 0;
    }
}
*/
void NanovnaAnalyzer::checkAnalyzer()
{
    QTimer::singleShot(2000, this, [this](){ versionRequest(); });
}


qint64 NanovnaAnalyzer::sendData(QString data)
{
    qDebug() << "NanovnaAnalyzer::sendData> " << data;

    qint64 res = m_comPort->write(data.toLocal8Bit());
    return res;
}

void NanovnaAnalyzer::startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber, bool frx)
{
    qDebug() << "NanovnaAnalyzer::startMeasure" << fqFrom << fqTo << dotsNumber;
    if (getParseState() == WAIT_NANO_SWEEP) {
        qDebug() << "getParseState() == WAIT_NANO_SWEEP";
        return;
    }
    Q_UNUSED (frx)
    m_fqFrom = fqFrom;
    m_fqTo = fqTo;
    m_dotsNumber = dotsNumber;
    m_isMeasuring = true;
    m_listFQ.clear();

    setParseState(WAIT_NANO_SWEEP);
    QString cmd = QString("sweep %1 %2 %3\r\n").arg(fqFrom).arg(fqTo).arg(dotsNumber);
    //QString cmd = QString("sweep 100 1000000000 101\r\n").arg(fqFrom).arg(fqTo).arg(dotsNumber);
    qDebug() << "NanovnaAnalyzer::startMeasure " << cmd;
    sendData(cmd);
}

void NanovnaAnalyzer::stopMeasure()
{
    m_isMeasuring = false;
}

void NanovnaAnalyzer::makeScreenshot()
{
    setIsMeasuring(true);
    m_parseState = WAIT_SCREENSHOT_DATA;
    m_incomingBuffer.clear();
    // TODO
}

void NanovnaAnalyzer::on_screenshotComplete()
{
    m_parseState = WAIT_NO;
    setIsMeasuring(false);
}

void NanovnaAnalyzer::on_measurementComplete()
{
    setIsMeasuring(false);
}

void NanovnaAnalyzer::on_changedSerialPort(QString portName)
{
    m_serialPortName = portName;
    closeComPort();
    //m_analyzerModel = 0;
    emit analyzerDisconnected();
}

void NanovnaAnalyzer::versionRequest()
{
    setParseState(WAIT_NANO_VER);
    sendData("info\r\n");
}

void NanovnaAnalyzer::portClosed()
{
    qDebug() << "Port closed";
}


void NanovnaAnalyzer::detectPorts()
{
    NanovnaAnalyzer::m_listNanovnaPorts.clear();
    QList<QSerialPortInfo> listPorts = QSerialPortInfo::availablePorts();
    for (int idx=0; idx<listPorts.size(); idx++) {
        QSerialPortInfo info = listPorts.at(idx);
        quint16 vendorIdentifier = info.vendorIdentifier() ;
        quint16 productIdentifier = info.productIdentifier() ;
        if (vendorIdentifier == NANOVNA_VID && productIdentifier == NANOVNA_PID) {
            NanovnaAnalyzer::m_listNanovnaPorts << info;
        }
    }
}

rawData NanovnaAnalyzer::toRawData(QString& s1p)
{
    rawData data;
    data.fq = -1;
    data.r = 0;
    data.x = 0;

    QString str;
    QStringList stringList = s1p.split(' ');
    if (stringList.size() < 3)
        return data;

    bool ok;
    str = stringList.takeFirst();
    data.fq = str.toDouble(&ok) * 0.000001;
    if (!ok)
        qDebug() << "***** ERROR: " << str;

    str = stringList.takeFirst();
    double param1 = str.toDouble(&ok);
    if (!ok)
        qDebug() << "***** ERROR: " << str;
    str = stringList.takeFirst();
    double param2 = str.toDouble(&ok);
    if (!ok)
        qDebug() << "***** ERROR: " << str;
    data.r = (1-param1*param1-param2*param2)/((1-param1)*(1-param1)+param2*param2);
    data.x = (2*param2)/((1-param1)*(1-param1)+param2*param2);
    data.r *= 50;
    data.x *= 50;
    return data;
}

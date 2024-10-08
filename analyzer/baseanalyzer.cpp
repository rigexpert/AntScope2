#include "baseanalyzer.h"
#include "AA55BTPacket.h"

BaseAnalyzer::BaseAnalyzer(QObject *parent)
    : QObject(parent),
      m_parseState(1),
      m_analyzerModel(0),
      m_isMeasuring(false),
      m_isContinuos(false)
{
    m_sendTimer = new QTimer(this);
    connect(m_sendTimer, SIGNAL(timeout()), this, SLOT(continueMeasurement()));
}

void BaseAnalyzer::startMeasure(qint64 fqFrom, qint64 fqTo, int dotsNumber, bool frx)
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

    //qDebug() << "BaseAnalyzer::startMeasure:" << fqFrom << fqTo << dotsNumber << "state" << state << "m_ok" << m_ok;
    switch(state)
    {
    case 1:
        setIsMeasuring(true);
        setParseState(WAIT_DATA);
        if(dotsNumber != 0)
        {
            band = fqTo - fqFrom;
            center = band/2 + fqFrom;

            //qDebug() << "comAnalyzer::startMeasure: " << band << dotsNumber << (band / dotsNumber);
            AA55BTPacket::start(fqFrom, band / dotsNumber);
        }else
        {
            band = 0;
            center = fqFrom;
        }
        FQ  = "FQ"  + QString::number(center) + QChar(0x0D);
        SW  = "SW"  + QString::number(band) + QChar(0x0D);
        if (m_isS21) {
            FRX = "FDB" + QString::number(dotsNumber) + QChar(0x0D);
        } else {
            FRX = (m_isFRX ? "FRX" : "EFRX") + QString::number(dotsNumber) + QChar(0x0D);
        }

        m_ok = false;

        sendCommand(FQ);
        m_sendTimer->start(10);
        state++;
        break;
    case 2:
        if(m_ok)
        {
            setIsMeasuring(true);
            m_ok = false;
            sendCommand(SW);
            state++;
        }
        break;
    case 3:
        if(m_ok)
        {
            setIsMeasuring(true);
            m_ok = false;
            sendCommand(FRX);
            if (m_isS21)
                setParseState(WAIT_S21_DATA);
            else
                setParseState(m_isFRX ? WAIT_DATA : WAIT_USER_DATA);

            state = 1;
            m_sendTimer->stop();
        }
        break;
    default:
        break;
    }
}

void BaseAnalyzer::continueMeasurement()
{
    startMeasure(0,0,0);
}

void BaseAnalyzer::stopMeasure()
{
    sendData("off\r");
    setIsMeasuring(false);
}

void BaseAnalyzer::startMeasureOneFq(qint64 fqFrom, int dotsNumber, bool frx)
{
    startMeasure(fqFrom, fqFrom, dotsNumber, frx);
}

void BaseAnalyzer::on_screenshotComplete()
{
    m_parseState = WAIT_NO;
    setIsMeasuring(false);
}

void BaseAnalyzer::getAnalyzerData()
{
    setTakeData(true);
    setIsMeasuring(true);
    m_parseState = WAIT_ANALYZER_DATA;
    m_incomingBuffer.clear();
    sendCommand("FLASHH\r");
}

void BaseAnalyzer::getAnalyzerData(QString number)
{
    setTakeData(true);
    setIsMeasuring(true);
    m_parseState = WAIT_DATA;
    m_incomingBuffer.clear();
    QString str = "FLASHFRX" + number + "\r";
    sendCommand(str);
}

void BaseAnalyzer::on_measurementComplete()
{
    if(!m_isContinuos)
    {
        setIsMeasuring(false);
    }
}

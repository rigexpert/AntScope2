#include "analyzer.h"
#include "popupindicator.h"
#include "customanalyzer.h"
#include <QDateTime>
#include "Notification.h"

// static member
#ifdef NEW_ANALYZER
QList<AnalyzerParameters*> AnalyzerParameters::m_analyzers;
AnalyzerParameters* AnalyzerParameters::m_current=nullptr;
#endif

Analyzer::Analyzer(QObject *parent) : QObject(parent),
    m_hidAnalyzer(nullptr),
    m_comAnalyzer(nullptr),
    m_analyzerModel(0),
    m_comAnalyzerFound(false),
    m_hidAnalyzerFound(false),
    m_nanovnaAnalyzerFound(false),
    m_chartCounter(0),
    m_isMeasuring(false),
    m_isContinuos(false),
    m_dotsNumber(100),
    m_downloader(nullptr),
    m_updateDialog(nullptr),
    m_pfw(nullptr),
    m_INFOSIZE(512),
    m_MAGICAA230Z(0xFE02A185),
    m_MAGICHID(0x5c620202),
    m_INTERNALMAGICAA230Z(0x87654321),
    m_MAGICAA30ZERO(0x5c623002),
    m_calibrationMode(false)
{
    m_pfw = new QByteArray;

#ifdef NEW_ANALYZER
    AnalyzerParameters::fill();
#endif

    on_comAnalyzerDisconnected(); // create hidAnalyzer
}

Analyzer::~Analyzer()
{
    if(m_downloader)
    {
        delete m_downloader;
        m_downloader = nullptr;
    }
    delete m_pfw;
    if(m_hidAnalyzer)
    {
        delete m_hidAnalyzer;
        m_hidAnalyzer = nullptr;
    }
    if(m_comAnalyzer)
    {
#ifdef NEW_CONNECTION
        ComAnalyzer* tmp = m_comAnalyzer;
#else
        comAnalyzer* tmp = m_comAnalyzer;
#endif
        m_comAnalyzer = nullptr;
        delete tmp;
    }
}

double Analyzer::getVersion() const
{
    if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
    {
        return m_comAnalyzer->getVersion().toDouble();
    }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
    {
        return m_hidAnalyzer->getVersion().toDouble();
    }
    return 0;
}

QString Analyzer::getVersionString() const
{
    if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
    {
        return m_comAnalyzer->getVersion();
    }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
    {
        return m_hidAnalyzer->getVersion();
    }
    return QString();
}

QString Analyzer::getRevision() const
{
    if(m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
    {
        return m_hidAnalyzer->getRevision();
    }else if (m_comAnalyzerFound && m_comAnalyzer != nullptr)
    {
        return m_comAnalyzer->getRevision();
    }
    return QString();
}

void Analyzer::on_downloadInfoComplete()
{
    QString ver = m_downloader->version();
    if (m_bManualUpdate)
    {
        if(ver.isEmpty())
        {
            QMessageBox::information(nullptr, tr("Latest version"),
                                 tr("Can not get the latest version.\nPlease try later."));
        }else
        {
            double internetVersion = ver.toDouble();//ver.remove(".").toInt();
            m_updateDialog = new UpdateDialog();
            m_updateDialog->setAttribute(Qt::WA_DeleteOnClose);
            m_updateDialog->setWindowTitle(tr("Updating"));
            connect(m_updateDialog,SIGNAL(update()),this,SLOT(on_internetUpdate()));
            connect(this, SIGNAL(updatePercentChanged(int)),m_updateDialog,SLOT(on_percentChanged(qint32)));
            if(internetVersion > getVersion())
            {
                m_updateDialog->setMainText(tr("New version of firmware is available now!"));
            }else
            {
                m_updateDialog->setMainText(tr("You have the latest version of firmware."));
            }
            m_updateDialog->exec();
        }
    } else {  // auto check for new firmvare
        if (!ver.isEmpty())
        {
            double internetVersion = ver.toDouble();
            if(internetVersion > getVersion())
            {
                const qint64 interval = 24*60*60;
                QString serialNumber = getSerialNumber();
                QString key = "firmware_" + serialNumber;

                QSettings settings(Settings::setIniFile(), QSettings::IniFormat);
                settings.beginGroup("Update");
                qint64 last_notify = settings.value(key, 0).toLongLong();

                QDateTime last_dt;
                last_dt.setMSecsSinceEpoch(last_notify);
                QDateTime current_dt = QDateTime::currentDateTime();
                if (last_dt.secsTo(current_dt) > interval)
                {
                    settings.setValue(key, QDateTime::currentMSecsSinceEpoch());
                    emit showNotification(
                                QString(tr("New version of firmware is available! Click here to get more details")),
                                m_downloader->downloadLink());
                }
                settings.endGroup();
            }
        }
    }
    QSettings settings(Settings::setIniFile(), QSettings::IniFormat);
    settings.beginGroup("Update");
    settings.setValue("statistics_time", QDateTime::currentMSecsSinceEpoch());
    settings.endGroup();
}
/*
bool Analyzer::needCheckForUpdate()
{
    const qint64 interval = 24*60*60;
    QString serialNumber = getSerialNumber();
    QString key = "firmware_" + serialNumber;

    QSettings settings(Settings::setIniFile(), QSettings::IniFormat);
    settings.beginGroup("Update");
    qint64 last_notify = settings.value(key, 0).toLongLong();

    QDateTime last_dt;
    last_dt.setMSecsSinceEpoch(last_notify);
    QDateTime current_dt = QDateTime::currentDateTime();

    return last_dt.secsTo(current_dt) > interval;
}
*/

bool Analyzer::needCheckForUpdate()
{
    QSettings settings(Settings::setIniFile(), QSettings::IniFormat);
    settings.beginGroup("Update");
    qlonglong time = settings.value("statistics_time", 0).toLongLong();
    settings.endGroup();

    QDateTime dtPrev;
    dtPrev.setMSecsSinceEpoch(time);
    QDateTime dtCur = QDateTime::currentDateTime();

    return (dtPrev.date().dayOfYear() != dtCur.date().dayOfYear());
}

void Analyzer::on_downloadFileComplete()
{
    *m_pfw = m_downloader->file();
    QBuffer fwdata(m_pfw);

    fwdata.open(QIODevice::ReadOnly);
    fwdata.seek(m_INFOSIZE);

    updateFirmware(&fwdata);
}

void Analyzer::on_internetUpdate()
{
    m_downloader->startDownloadFw();
    m_updateDialog->setStatusText(tr("Downloading firmware..."));
}

void Analyzer::readFile(QString pathToFw)
{
    QFile file(pathToFw);
    bool state = true;

    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, tr("Warning"), tr("Can not open firmware file."));
        return;
    }

    *m_pfw = file.readAll();

    if (m_pfw->isEmpty())
    {
        QMessageBox::warning(nullptr, tr("Warning"), tr("Can not read firmware file."));
        state = false;
    }

    file.close();

    if(state)
    {        
        //m_updateDialog->setStatusText(tr("Updating, please wait..."));
        QBuffer fwdata(m_pfw);
        fwdata.open(QIODevice::ReadOnly);
        fwdata.seek(m_INFOSIZE);
        updateFirmware(&fwdata);
    }
}

bool Analyzer::checkFile(QString path)
{
#if 0
    // obsolete
    QFile fwfile(path);
    QByteArray arr;
    QByteArray fw;
    quint32 magic = 0;
    quint32 len = 0;
    quint32 readCrc = 0;
    quint32 revision = 0;
    const char *pd;

    if (!fwfile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(nullptr, tr("Warning"),
                             tr("Firmware file can not open"));
        return false;
    }

    arr = fwfile.read(m_INFOSIZE);

    if ((qint32)arr.length() < m_INFOSIZE)
    {
        fwfile.close();
        return false;
    }

    pd = arr.constData();

    magic = qFromLittleEndian<quint32>(*((quint32*)pd));
    len = qFromLittleEndian<quint32>(*((quint32*)&pd[4]));
    readCrc = qFromLittleEndian<quint32>(*((quint32*)&pd[8]));
    revision = qFromLittleEndian<quint32>(*((quint32*)&pd[20]));

    if(m_hidAnalyzer)
    {
        //m_hidAnalyzer->setRevision(revision); // old
        m_hidAnalyzer->setRevision(QString::number(revision));
    }

    QString prototype = CustomAnalyzer::currentPrototype();
    if(((names[getAnalyzerModel()] == "AA-230 ZOOM" || prototype == "AA-230 ZOOM") && (magic == m_MAGICAA230Z)) ||
            ((names[getAnalyzerModel()] == "AA-650 ZOOM" || prototype == "AA-650 ZOOM") && (magic == m_MAGICAA230Z)) ||
            ((names[getAnalyzerModel()] == "AA-55 ZOOM" || prototype == "AA-55 ZOOM") && (magic == m_MAGICHID)) ||
            ((names[getAnalyzerModel()] == "AA-35 ZOOM" || prototype == "AA-35 ZOOM") && (magic == m_MAGICHID)) ||
            ((names[getAnalyzerModel()] == "AA-30 ZERO" || prototype == "AA-30 ZERO") && (magic == m_MAGICAA30ZERO)) ||
            ((names[getAnalyzerModel()] == "AA-30.ZERO" || prototype == "AA-30.ZERO") && (magic == m_MAGICAA30ZERO)))
    {
    }else
    {
        QMessageBox::warning(nullptr, tr("Warning"),
                             tr("Firmware file has wrong format"));
        fwfile.close();
        return false;
    }

    if (!fwfile.seek(m_INFOSIZE))
    {
        QMessageBox::warning(nullptr, tr("Warning"),
                             tr("Firmware file is too short."));
        fwfile.close();
        return false;
    }

    fw = fwfile.readAll();

    fwfile.close();

    if ((quint32)fw.length() != len) {
        QMessageBox::warning(nullptr, tr("Warning"),
                             tr("Firmware file has wrong length."));
        return false;
    }

    if (readCrc != CRC32::crc(0xffffffff, fw)) {
        QMessageBox::warning(nullptr, tr("Warning"),
                             tr("Firmware file has wrong CRC."));
        return false;
    }
#endif
    return true;
}

QString Analyzer::getModelString( void )
{    
#ifndef NEW_ANALYZER
    return CustomAnalyzer::customized() ? CustomAnalyzer::currentPrototype() : names[m_analyzerModel];
#else
    return CustomAnalyzer::customized() ? CustomAnalyzer::currentPrototype() : AnalyzerParameters::getName();
#endif
}

quint32 Analyzer::getModel( void )
{
    return m_analyzerModel;
}

QString Analyzer::getSerialNumber(void) const
{
    if(m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
    {
        return m_hidAnalyzer->getSerial();
    }else if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
    {
        return m_comAnalyzer->getSerial();
    }
    return QString();
}

#ifdef NEW_CONNECTION
void Analyzer::on_measure (qint64 fqFrom, qint64 fqTo, qint32 dotsNumber)
{
    qDebug() << "Analyzer::on_measure()";
    m_getAnalyzerData = false;
    if(!m_isMeasuring)
    {
        setIsMeasuring(true);
        QDateTime datetime = QDateTime::currentDateTime();
        QString name = datetime.toString("##dd.MM.yyyy-hh:mm:ss");
        emit newMeasurement(name, fqFrom, fqTo, dotsNumber);
        m_dotsNumber = dotsNumber;
        m_chartCounter = 0;
        if (m_baseAnalyzer != nullptr)
        {
            m_baseAnalyzer->setIsFRXMode(true);
            m_baseAnalyzer->startMeasure(fqFrom, fqTo, m_dotsNumber);
            PopUpIndicator::setIndicatorVisible(true);
            return;
        }
    }
    on_stopMeasure();
}

void Analyzer::on_measureContinuous(qint64 fqFrom, qint64 fqTo, qint32 dotsNumber)
{
    if(!m_isMeasuring)
    {
        setIsMeasuring(true);
        emit continueMeasurement(fqFrom, fqTo, dotsNumber);
        m_dotsNumber = dotsNumber;
        m_chartCounter = 0;
        if (m_baseAnalyzer != nullptr && m_baseAnalyzer->type() != ReDeviceInfo::NANO)
        {
            m_baseAnalyzer->startMeasure(fqFrom,fqTo,dotsNumber);
            PopUpIndicator::setIndicatorVisible(true);
            return;
        }
    }
    on_stopMeasure();
}

void Analyzer::on_measureUser (qint64 fqFrom, qint64 fqTo, qint32 dotsNumber)
{
    if(!m_isMeasuring)
    {
        setIsMeasuring(true);
        QDateTime datetime = QDateTime::currentDateTime();
        QString name = datetime.toString("##dd.MM.yyyy-hh:mm:ss");
        emit newMeasurement(name, fqFrom, fqTo, dotsNumber);
        m_dotsNumber = dotsNumber;
        m_chartCounter = 0;
        if (m_baseAnalyzer != nullptr && m_baseAnalyzer->type() != ReDeviceInfo::NANO)
        {
            m_baseAnalyzer->setIsFRXMode(false);
            m_baseAnalyzer->startMeasure(fqFrom,fqTo,dotsNumber);
            PopUpIndicator::setIndicatorVisible(true);
            return;
        }
    }
    on_stopMeasure();
}

void Analyzer::on_measureOneFq(QWidget* /*parent*/, qint64 fqFrom, qint32 /*dotsNumber*/)
{
    setIsMeasuring(true);
    m_dotsNumber = 100000;
    m_chartCounter = 0;
    if (m_baseAnalyzer != nullptr && m_baseAnalyzer->type() != ReDeviceInfo::NANO)
    {
        m_baseAnalyzer->setIsFRXMode(true);
        m_baseAnalyzer->startMeasureOneFq(fqFrom,m_dotsNumber);
    }
}

void Analyzer::on_stopMeasure()
{
    PopUpIndicator::setIndicatorVisible(false);
    setIsMeasuring(false);
    m_chartCounter = 0;
    if (m_baseAnalyzer != nullptr)
    {
        m_baseAnalyzer->stopMeasure();
    }
    emit measurementComplete();
}
#else
void Analyzer::on_measure (qint64 fqFrom, qint64 fqTo, qint32 dotsNumber)
{
    m_getAnalyzerData = false;
    if(!m_isMeasuring)
    {
        m_isMeasuring = true;
        QDateTime datetime = QDateTime::currentDateTime();
        QString name = datetime.toString("##dd.MM.yyyy-hh:mm:ss");
        emit newMeasurement(name, fqFrom, fqTo, dotsNumber);
        //emit newMeasurement(name);
        m_dotsNumber = dotsNumber;
        m_chartCounter = 0;
        if (m_nanovnaAnalyzerFound && m_NanovnaAnalyzer != nullptr)
        {
          m_dotsNumber = 101;
          m_NanovnaAnalyzer->startMeasure(fqFrom, fqTo, m_dotsNumber);
        } else  if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
        {
            m_comAnalyzer->setIsFRXMode(true);
            m_comAnalyzer->startMeasure(fqFrom,fqTo,m_dotsNumber);
        }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
        {
            m_hidAnalyzer->setIsFRXMode(true);
            m_hidAnalyzer->startMeasure(fqFrom,fqTo,m_dotsNumber);
        }
        PopUpIndicator::setIndicatorVisible(true);
    } else {
        on_stopMeasure();
    }
}


void Analyzer::on_measureContinuous(qint64 fqFrom, qint64 fqTo, qint32 dotsNumber)
{
    if(!m_isMeasuring)
    {
        m_isMeasuring = true;
        //QThread::msleep(500);
        emit continueMeasurement(fqFrom, fqTo, dotsNumber);
        m_dotsNumber = dotsNumber;
        m_chartCounter = 0;
        if (m_nanovnaAnalyzerFound && m_NanovnaAnalyzer != nullptr)
        {
          //m_NanovnaAnalyzer->startMeasure(fqFrom, fqTo, dotsNumber);
        } else  if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
        {
            m_comAnalyzer->startMeasure(fqFrom,fqTo,dotsNumber);
        }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
        {
            //m_hidAnalyzer->setIsFRXMode(true);
            m_hidAnalyzer->startMeasure(fqFrom,fqTo,dotsNumber);
        }
        PopUpIndicator::setIndicatorVisible(true);
    } else {
        qDebug() << "Analyzer::on_measureContinuous  STOP";
        on_stopMeasure();
    }
}

void Analyzer::on_measureUser (qint64 fqFrom, qint64 fqTo, qint32 dotsNumber)
{
    if(!m_isMeasuring)
    {
        m_isMeasuring = true;
        QDateTime datetime = QDateTime::currentDateTime();
        QString name = datetime.toString("##dd.MM.yyyy-hh:mm:ss");
        emit newMeasurement(name, fqFrom, fqTo, dotsNumber);
        m_dotsNumber = dotsNumber;
        m_chartCounter = 0;
        if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
        {
            m_comAnalyzer->setIsFRXMode(false);
            m_comAnalyzer->startMeasure(fqFrom,fqTo,dotsNumber);
        }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
        {
            m_hidAnalyzer->setIsFRXMode(false);
            m_hidAnalyzer->startMeasure(fqFrom,fqTo,dotsNumber);
        }
        PopUpIndicator::setIndicatorVisible(true);
    } else {
        on_stopMeasure();
    }
}

void Analyzer::on_measureOneFq(QWidget* /*parent*/, qint64 fqFrom, qint32 /*dotsNumber*/)
{
    m_isMeasuring = true;
    m_dotsNumber = 100000;
    m_chartCounter = 0;
    if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
    {
        m_comAnalyzer->setIsFRXMode(true);
        m_comAnalyzer->startMeasureOneFq(fqFrom,m_dotsNumber);
    }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
    {
        m_hidAnalyzer->setIsFRXMode(true);
        m_hidAnalyzer->startMeasureOneFq(fqFrom,m_dotsNumber);
    }
}

void Analyzer::on_stopMeasure()
{
    PopUpIndicator::setIndicatorVisible(false);
    m_isMeasuring = false;
    m_chartCounter = 0;
    if (m_nanovnaAnalyzerFound && m_NanovnaAnalyzer != nullptr)
    {
        m_NanovnaAnalyzer->stopMeasure();
    } else if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
    {
        m_comAnalyzer->stopMeasure();
    }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
    {
        m_hidAnalyzer->stopMeasure();
    }
    emit measurementComplete();
}
#endif
void Analyzer::updateFirmware (QIODevice *fw)
{
    if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
    {
        m_comAnalyzer->update(fw);
    }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
    {
        m_hidAnalyzer->update(fw);
    }
}

void Analyzer::setAutoCheckUpdate( bool state)
{
    m_autoCheckUpdate = state;
}


#ifndef NEW_CONNECTION
void Analyzer::makeScreenshot()
{
    if(!m_isMeasuring)
    {
        if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
        {
            QTimer::singleShot(100, m_comAnalyzer, SLOT(makeScreenshot()));
        }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
        {
            QTimer::singleShot(100, m_hidAnalyzer, SLOT(makeScreenshot()));
        }
    }
}
#else
void Analyzer::makeScreenshot()
{
    if(!m_isMeasuring)
    {

        if(m_baseAnalyzer != nullptr)
        {
            QTimer::singleShot(100, m_baseAnalyzer, SLOT(makeScreenshot()));
        }
    }
}
#endif
void Analyzer::on_hidAnalyzerFound (quint32 analyzerNumber)
{
    if(m_comAnalyzer)
    {
#ifdef NEW_CONNECTION
        ComAnalyzer* tmp = m_comAnalyzer;
#else
        comAnalyzer* tmp = m_comAnalyzer;
#endif
        m_comAnalyzer = nullptr;
        delete tmp;
    }
    m_hidAnalyzerFound = true;
    m_analyzerModel = analyzerNumber;

#ifndef NEW_ANALYZER
    QString str = CustomAnalyzer::customized() ? CustomAnalyzer::currentAlias() : names[m_analyzerModel];
#else
    QString str = CustomAnalyzer::customized() ? CustomAnalyzer::currentAlias() : AnalyzerParameters::getName();
#endif
    applyAnalyzer();
    emit analyzerFound(str);

    // ------- moved to MainWindow::on_analyzerFound
    //
    //if(m_autoCheckUpdate)
//    extern bool g_developerMode;
//    if (!g_developerMode)
//    {
//        QTimer::singleShot(5000, [this]() {
//            this->checkFirmwareUpdate();
//        });
//    }
}

void Analyzer::on_hidAnalyzerDisconnected ()
{
    AnalyzerParameters::setCurrent(nullptr);
    emit analyzerDisconnected();
}

void Analyzer::on_comAnalyzerFound (quint32 analyzerNumber)
{
    Q_UNUSED(analyzerNumber);

    if(m_hidAnalyzer)
    {
        delete m_hidAnalyzer;
        m_hidAnalyzer = nullptr;
    }
    m_comAnalyzerFound = true;
#ifndef NEW_ANALYZER
    m_analyzerModel = analyzerNumber;
    QString str = CustomAnalyzer::customized() ? CustomAnalyzer::currentPrototype() : names[m_analyzerModel];
#else
    QString str = CustomAnalyzer::customized() ? CustomAnalyzer::currentPrototype() : AnalyzerParameters::getName();
#endif
    applyAnalyzer();
    emit analyzerFound(str);
}

void Analyzer::on_comAnalyzerDisconnected ()
{
    delete m_comAnalyzer;
    m_comAnalyzer = nullptr;

    if(!m_hidAnalyzer)
    {
#ifdef NEW_CONNECTION
        m_hidAnalyzer = new HidAnalyzer(this);
#else
        m_hidAnalyzer = new hidAnalyzer(this);
#endif

        connect(m_hidAnalyzer,SIGNAL(newData(rawData)),this,SLOT(on_newData(rawData)));
        connect(m_hidAnalyzer,SIGNAL(newUserData(rawData,UserData)),this,SLOT(on_newUserData(rawData,UserData)));
        connect(m_hidAnalyzer,SIGNAL(newUserDataHeader(QStringList)),this,SLOT(on_newUserDataHeader(QStringList)));
        connect(m_hidAnalyzer,SIGNAL(analyzerFound(quint32)),this,SLOT(on_hidAnalyzerFound(quint32)));
        connect(m_hidAnalyzer,SIGNAL(analyzerDisconnected()),this,SLOT(on_hidAnalyzerDisconnected()));
        connect(m_hidAnalyzer,SIGNAL(analyzerDataStringArrived(QString)),this,SLOT(on_analyzerDataStringArrived(QString)));
        connect(m_hidAnalyzer,SIGNAL(analyzerScreenshotDataArrived(QByteArray)),this,SLOT(on_analyzerScreenshotDataArrived(QByteArray)));
        connect(this, SIGNAL(screenshotComplete()),m_hidAnalyzer,SLOT(on_screenshotComplete()));
        connect(this, SIGNAL(measurementComplete()), m_hidAnalyzer, SLOT(on_measurementComplete()));//, Qt::QueuedConnection);
#ifdef NEW_CONNECTION
        connect(m_hidAnalyzer, &HidAnalyzer::signalFullInfo, this, &Analyzer::slotFullInfo);
        connect(m_hidAnalyzer, &HidAnalyzer::signalMeasurementError, this, &Analyzer::signalMeasurementError);
#else
        connect(m_hidAnalyzer, &hidAnalyzer::signalFullInfo, this, &Analyzer::slotFullInfo);
        connect(m_hidAnalyzer, &hidAnalyzer::signalMeasurementError, this, &Analyzer::signalMeasurementError);
#endif
    }

    m_comAnalyzerFound = false;
#ifndef NEW_ANALYZER
    m_analyzerModel = 0;
    if(m_comAnalyzer != nullptr)
    {
        m_comAnalyzer->setAnalyzerModel(0);
    }
#else
    AnalyzerParameters::setCurrent(nullptr);
#endif
    emit analyzerDisconnected();
}

void Analyzer::on_nanovnaAnalyzerFound (QString name)
{
    if(m_hidAnalyzer)
    {
        delete m_hidAnalyzer;
        m_hidAnalyzer = nullptr;
    }
    if(m_comAnalyzer)
    {
        delete m_comAnalyzer;
        m_comAnalyzer = nullptr;
    }
    m_hidAnalyzerFound = false;
    m_comAnalyzerFound = false;
    m_nanovnaAnalyzerFound = true;
#ifndef NEW_ANALYZER
    for (int i=0; i<QUANTITY; i++) {
        if (names[i] == "NanoVNA") {
            m_analyzerModel = i;
            break;
        }
    }
#else
    AnalyzerParameters::setCurrent(AnalyzerParameters::byName("NanoVNA"));
#endif
    applyAnalyzer();
    emit analyzerFound(name);
}

void Analyzer::on_nanovnaAnalyzerDisconnected()
{
    delete m_NanovnaAnalyzer;
    m_NanovnaAnalyzer = nullptr;

    if(!m_hidAnalyzer)
    {
#ifdef NEW_CONNECTION
        m_hidAnalyzer = new HidAnalyzer(this);
#else
        m_hidAnalyzer = new hidAnalyzer(this);
#endif
        connect(m_hidAnalyzer,SIGNAL(newData(rawData)),this,SLOT(on_newData(rawData)));
        connect(m_hidAnalyzer,SIGNAL(newUserData(rawData,UserData)),this,SLOT(on_newUserData(rawData,UserData)));
        connect(m_hidAnalyzer,SIGNAL(newUserDataHeader(QStringList)),this,SLOT(on_newUserDataHeader(QStringList)));
        connect(m_hidAnalyzer,SIGNAL(analyzerFound(quint32)),this,SLOT(on_hidAnalyzerFound(quint32)));
        connect(m_hidAnalyzer,SIGNAL(analyzerDisconnected()),this,SLOT(on_hidAnalyzerDisconnected()));
        connect(m_hidAnalyzer,SIGNAL(analyzerDataStringArrived(QString)),this,SLOT(on_analyzerDataStringArrived(QString)));
        connect(m_hidAnalyzer,SIGNAL(analyzerScreenshotDataArrived(QByteArray)),this,SLOT(on_analyzerScreenshotDataArrived(QByteArray)));
        connect(this, SIGNAL(screenshotComplete()),m_hidAnalyzer,SLOT(on_screenshotComplete()));
        connect(this, SIGNAL(measurementComplete()), m_hidAnalyzer, SLOT(on_measurementComplete()));//, Qt::QueuedConnection);
#ifdef NEW_CONNECTION
        connect(m_hidAnalyzer, &HidAnalyzer::signalFullInfo, this, &Analyzer::slotFullInfo);
        connect(m_hidAnalyzer, &HidAnalyzer::signalMeasurementError, this, &Analyzer::signalMeasurementError);
#else
        connect(m_hidAnalyzer, &hidAnalyzer::signalFullInfo, this, &Analyzer::slotFullInfo);
        connect(m_hidAnalyzer, &hidAnalyzer::signalMeasurementError, this, &Analyzer::signalMeasurementError);
#endif
    }
    m_nanovnaAnalyzerFound = false;
#ifndef NEW_ANALYZER
    m_analyzerModel = 0;
#else
    AnalyzerParameters::setCurrent(nullptr);
#endif
    emit analyzerDisconnected();
}

void Analyzer::on_newData(rawData _rawData)
{
    qDebug() << "Analyzer::on_newData" << (m_chartCounter) << (m_dotsNumber);
    if (m_getAnalyzerData) {
        emit newAnalyzerData (_rawData);
    } else {
        emit newData (_rawData);
    }

    // ???? if(m_chartCounter >= m_dotsNumber || !m_isMeasuring)
    quint32 finNum = m_calibrationMode ? m_dotsNumber : (m_dotsNumber-1);
    if(m_chartCounter > finNum || !m_isMeasuring)
    {
        qDebug() << "Analyzer::on_newData COMPLETE";
        m_chartCounter = 0;
        setIsMeasuring(false);
        PopUpIndicator::setIndicatorVisible(false);
        if(!m_calibrationMode)
        {
            emit measurementComplete();
        }
        return;
    }
    m_chartCounter++;
}

void Analyzer::on_newUserData(rawData _rawData, UserData _userData)
{
    if(++m_chartCounter == m_dotsNumber+1 || !m_isMeasuring)
    {
        emit newUserData (_rawData, _userData);
        setIsMeasuring(false);
        m_chartCounter = 0;
        PopUpIndicator::setIndicatorVisible(false);
        if(!m_calibrationMode)
        {
            emit measurementComplete();
        }
    }else
    {
        emit newUserData (_rawData, _userData);
    }
}

void Analyzer::on_newUserDataHeader(QStringList fields)
{
    emit newUserDataHeader (fields);
}

void Analyzer::on_analyzerDataStringArrived(QString str)
{
    emit analyzerDataStringArrived(str);
}

#ifndef NEW_CONNECTION
void Analyzer::getAnalyzerData()
{
    if(!m_isMeasuring)
    {
        if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
        {
            QTimer::singleShot(100, m_comAnalyzer, SLOT(getAnalyzerData()));
        }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
        {
            QTimer::singleShot(100, m_hidAnalyzer, SLOT(getAnalyzerData()));
        }
    }
}

void Analyzer::closeAnalyzerData()
{
    if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
    {
        m_comAnalyzer->setTakeData(false);
    }
}

void Analyzer::on_itemDoubleClick(QString number, QString dotsNumber, QString name)
{
    setIsMeasuring(true);
    if (name.trimmed().isEmpty()) {
        name = number;
    }
    m_getAnalyzerData = true;
    if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
    {
        m_chartCounter = 0;
        m_dotsNumber = dotsNumber.toInt();
        emit newMeasurement(name);
        m_comAnalyzer->getAnalyzerData(number);
    }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
    {
        m_chartCounter = 0;
        m_dotsNumber = dotsNumber.toInt();
        emit newMeasurement(name);
        m_hidAnalyzer->getAnalyzerData(number);
    }
}
#else
void Analyzer::getAnalyzerData()
{
    if(!m_isMeasuring)
    {
        if(!isMeasuring() && m_baseAnalyzer != nullptr)
        {
            QTimer::singleShot(100, m_baseAnalyzer, SLOT(getAnalyzerData()));
        }
    }
}

void Analyzer::closeAnalyzerData()
{
    if(m_baseAnalyzer != nullptr)
    {
        m_baseAnalyzer->setTakeData(false);
    }
}

void Analyzer::on_itemDoubleClick(QString number, QString dotsNumber, QString name)
{
    setIsMeasuring(true);
    if (name.trimmed().isEmpty()) {
        name = number;
    }
    m_getAnalyzerData = true;
    if(m_baseAnalyzer != nullptr)
    {
        m_chartCounter = 0;
        m_dotsNumber = dotsNumber.toInt();
        emit newMeasurement(name);
        m_baseAnalyzer->getAnalyzerData(number);
    }
}
#endif
void Analyzer::on_dialogClosed()
{
    //setIsMeasuring(false);
}

void Analyzer::on_stopMeasuring()
{
    setIsMeasuring(false);
}

void Analyzer::on_analyzerScreenshotDataArrived(QByteArray arr)
{
    emit analyzerScreenshotDataArrived(arr);
}

void Analyzer::on_screenshotComplete(void)
{
    emit screenshotComplete();
}

void Analyzer::on_updatePercentChanged(int number)
{
    if (m_updateDialog != nullptr)
        m_updateDialog->on_percentChanged(number);
    emit updatePercentChanged(number);
}

void Analyzer::checkFirmwareUpdate()
{
    if (needCheckForUpdate())
    {
        on_checkUpdatesBtn_clicked();
        m_bManualUpdate = false;
    }
}

void Analyzer::on_checkUpdatesBtn_clicked()
{
    m_bManualUpdate = true;
    if(m_downloader == nullptr)
    {
        m_downloader = new Downloader();
        connect(m_downloader, SIGNAL(downloadInfoComplete()),
                this, SLOT(on_downloadInfoComplete()));
        connect(m_downloader, SIGNAL(sendStatisicsComplete()),
                this, SLOT(on_statisticsComplete()));
        connect(m_downloader, SIGNAL(downloadFileComplete()),
                this, SLOT(on_downloadFileComplete()));
        connect(m_downloader, SIGNAL(progress(qint64,qint64)),
                this, SLOT(on_progress(qint64,qint64)));
    }

    QString url = "https://www.rigexpert.com/getfirmware?app=antscope2&model=";
#ifndef NEW_ANALYZER
    url += names[m_analyzerModel].toLower().remove(" ").remove("-");
#else
    QString name = AnalyzerParameters::getName();
    url += name.toLower().remove(" ").remove("-");
#endif
    url += "&sn=" + getSerialNumber();
    url += "&revision=" + getRevision();
    url += "&os=" + QSysInfo::prettyProductName().replace(" ", "-").toLower();
    url += "&cpu=" + QSysInfo::currentCpuArchitecture();
    url += "&lang=" + QLocale::languageToString(QLocale::system().language());
    url += "&sw=" + QString(ANTSCOPE2VER);
    url += "&fw=" + getVersionString();

    m_downloader->startDownloadInfo(QUrl(url));
}

void Analyzer::on_progress(qint64 downloaded,qint64 total)
{
    int percent = downloaded*100/total;
    if (percent == 100)
    {
        emit updatePercentChanged(0);
        m_updateDialog->setStatusText(tr("Updating, please wait..."));
    }else
    {
        emit updatePercentChanged(percent);
    }
}

void Analyzer::closeComPort()
{
    if(m_comAnalyzer != nullptr)
    {
        m_comAnalyzer->closeComPort();
    }
}

void Analyzer::on_measureCalib(int dotsNumber)
{
    setIsMeasuring(true);
    m_dotsNumber = dotsNumber;
    m_chartCounter = 0;
#ifndef NEW_ANALYZER
    qint64 minFq_ = minFq[m_analyzerModel].toULongLong()*1000;
    qint64 maxFq_ = maxFq[m_analyzerModel].toULongLong()*1000;
#else
    qint64 minFq_ = AnalyzerParameters::getMinFq().toULongLong()*1000;
    qint64 maxFq_ = AnalyzerParameters::getMaxFq().toULongLong()*1000;
#endif
    if (CustomAnalyzer::customized()) {
        CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
        if (ca != nullptr) {
            minFq_ = ca->minFq().toULongLong()*1000;
            maxFq_ = ca->maxFq().toULongLong()*1000;
        }
    }
    if(m_comAnalyzerFound && m_comAnalyzer != nullptr)
    {
        m_comAnalyzer->startMeasure(minFq_, maxFq_, dotsNumber);
    }else if (m_hidAnalyzerFound && m_hidAnalyzer != nullptr)
    {
        m_hidAnalyzer->startMeasure(minFq_, maxFq_, dotsNumber);
    }
}

void Analyzer::setCalibrationMode(bool enabled)
{
    m_calibrationMode = enabled;
}

void Analyzer::setIsMeasuring (bool _isMeasuring)
{
    m_isMeasuring = _isMeasuring;
#ifdef NEW_CONNECTION
    if(m_baseAnalyzer != nullptr)
    {
        m_baseAnalyzer->setIsMeasuring(_isMeasuring);
    }
#else
    if(m_comAnalyzer != nullptr)
    {
        m_comAnalyzer->setIsMeasuring(_isMeasuring);
    }
    if(m_hidAnalyzer != nullptr)
    {
        m_hidAnalyzer->setIsMeasuring(_isMeasuring);
    }
    if(m_NanovnaAnalyzer != nullptr)
    {
        m_NanovnaAnalyzer->setIsMeasuring(_isMeasuring);
    }
#endif
    PopUpIndicator::setIndicatorVisible(isMeasuring());
}

void Analyzer::slotFullInfo(QString str)
{
    QStringList list = str.split("\t");
    if (list.size() < 2)
        return;
    m_mapFullInfo.insert(list[0], list[1]);    
}

void Analyzer::searchAnalyzer()
{
    // TODO
//    if (!isMeasuring())
//    {
//        if (m_hidAnalyzer != nullptr)
//            m_hidAnalyzer->searchAnalyzer(true);
//        if (m_comAnalyzer != nullptr)
//            m_comAnalyzer->searchAnalyzer();
//    }
    //
}


bool Analyzer::sendCommand(QString cmd)
{
    bool ret = true;
    if (getHidAnalyzer() != 0) {
        getHidAnalyzer()->sendData(cmd);
    } else if (getComAnalyzer() != 0) {
        getComAnalyzer()->sendData(cmd);
    } else {
        ret = false;
    }
    return ret;
}

void Analyzer::setParseState(int _state)
{
    if (getHidAnalyzer() != 0) {
        getHidAnalyzer()->setParseState(_state);
    } else if (getComAnalyzer() != 0) {
        getComAnalyzer()->setParseState(_state);
    }
}

int Analyzer::getParseState()
{
    int ret = WAIT_NO;
    if (getHidAnalyzer() != 0) {
        getHidAnalyzer()->getParseState();
    } else if (getComAnalyzer() != 0) {
        getComAnalyzer()->getParseState();
    }
    return ret;
}

void Analyzer::on_getLicenses()
{
    QString cmd = "LLIC\r\n";
    setParseState(WAIT_LICENSE_LIST);
    sendCommand(cmd);
}

void Analyzer::on_generateLicence()
{
    QString cmd = "GLIC\r\n";
    setParseState(WAIT_LICENSE_REQUEST);
    sendCommand(cmd);
}

void Analyzer::on_applyLicense(QString& _license)
{
    if (getHidAnalyzer() != 0) {
        getHidAnalyzer()->applyLicense(_license);
    } else if (getComAnalyzer() != 0) {
        getComAnalyzer()->applyLicense(_license);
    }
}

void Analyzer::on_disconnectNanoNVA()
{
    if (m_NanovnaAnalyzer != nullptr) {
        m_NanovnaAnalyzer->closeComPort();
        m_NanovnaAnalyzer->disconnect();
        m_NanovnaAnalyzer->deleteLater();
        m_NanovnaAnalyzer = nullptr;
    }
    m_connectionType = ReDeviceInfo::WRONG;
}

void Analyzer::on_connectNanoNVA(QString portName)
{
    m_NanovnaAnalyzer = new NanovnaAnalyzer(this);
    bool connected = m_NanovnaAnalyzer->openComPort(portName);
    if (connected) {
        connect(m_NanovnaAnalyzer,SIGNAL(analyzerFound(QString)),this,SLOT(on_nanovnaAnalyzerFound(QString)));
        connect(m_NanovnaAnalyzer,SIGNAL(analyzerDisconnected()),this,SLOT(on_nanovnaAnalyzerDisconnected()));
        connect(this, SIGNAL(measurementComplete()), m_NanovnaAnalyzer, SLOT(on_measurementComplete()));//, Qt::QueuedConnection);
        connect(m_NanovnaAnalyzer, &NanovnaAnalyzer::signalFullInfo, this, &Analyzer::slotFullInfo);
        connect(m_NanovnaAnalyzer, &NanovnaAnalyzer::signalMeasurementError, this, &Analyzer::signalMeasurementError);
        connect(m_NanovnaAnalyzer,SIGNAL(newData(rawData)),this,SLOT(on_newData(rawData)));
        connect(m_NanovnaAnalyzer, &NanovnaAnalyzer::completeMeasurement, this, [=](){
           emit measurementCompleteNano();
        });

        m_NanovnaAnalyzer->checkAnalyzer();
        m_connectionType = ReDeviceInfo::NANO;
    }
}

void Analyzer::on_disconnectHid()
{
    if (m_hidAnalyzer != nullptr) {
        m_hidAnalyzer->closeHid();
        m_hidAnalyzer->deleteLater();
        m_hidAnalyzer = nullptr;
    }
    m_connectionType = ReDeviceInfo::WRONG;
}

void Analyzer::on_connectHid()
{
    m_connectionType = ReDeviceInfo::HID;
    m_hidAnalyzer = new hidAnalyzer(this);

    connect(m_hidAnalyzer,SIGNAL(analyzerFound(quint32)),this,SLOT(on_hidAnalyzerFound(quint32)));
    connect(m_hidAnalyzer,SIGNAL(analyzerDisconnected()),this,SLOT(on_hidAnalyzerDisconnected()));
    connect(this, SIGNAL(measurementComplete()), m_hidAnalyzer, SLOT(on_measurementComplete()));//, Qt::QueuedConnection);
    connect(m_hidAnalyzer, &hidAnalyzer::signalFullInfo, this, &Analyzer::slotFullInfo);
    connect(m_hidAnalyzer, &hidAnalyzer::signalMeasurementError, this, &Analyzer::signalMeasurementError);
    connect(m_hidAnalyzer,SIGNAL(newData(rawData)),this,SLOT(on_newData(rawData)));
    connect(m_hidAnalyzer,SIGNAL(newUserDataHeader(QStringList)),this,SLOT(on_newUserDataHeader(QStringList)));
    connect(m_hidAnalyzer,SIGNAL(analyzerDataStringArrived(QString)),this,SLOT(on_analyzerDataStringArrived(QString)));
    connect(m_hidAnalyzer,SIGNAL(analyzerScreenshotDataArrived(QByteArray)),this,SLOT(on_analyzerScreenshotDataArrived(QByteArray)));
    connect(m_hidAnalyzer,SIGNAL(updatePercentChanged(int)),this,SLOT(on_updatePercentChanged(int)));
    connect(this, SIGNAL(screenshotComplete()),m_hidAnalyzer,SLOT(on_screenshotComplete()));
}

void Analyzer::on_disconnectSerial()
{
    if (m_comAnalyzer != nullptr) {
        m_comAnalyzer->closeComPort();
        m_comAnalyzer->disconnect();
        m_comAnalyzer->deleteLater();
        m_comAnalyzer = nullptr;
    }
    m_connectionType = ReDeviceInfo::WRONG;
}

void Analyzer::on_connectSerial(QString portName)
{
    m_connectionType = ReDeviceInfo::Serial;

    m_comAnalyzer = new comAnalyzer(this);
    m_comAnalyzer->setSerialPort(portName);
    m_comAnalyzer->searchAnalyzer();

    connect(m_comAnalyzer,SIGNAL(analyzerFound(quint32)),this,SLOT(on_comAnalyzerFound(quint32)));
    connect(m_comAnalyzer,SIGNAL(analyzerDisconnected()),this,SLOT(on_comAnalyzerDisconnected()));
    connect(this, SIGNAL(measurementComplete()), m_comAnalyzer, SLOT(on_measurementComplete()));//, Qt::QueuedConnection);
    connect(m_comAnalyzer, &comAnalyzer::signalFullInfo, this, &Analyzer::slotFullInfo);
    connect(m_comAnalyzer, &comAnalyzer::signalMeasurementError, this, &Analyzer::signalMeasurementError);
    connect(m_comAnalyzer,SIGNAL(newData(rawData)),this,SLOT(on_newData(rawData)));
    connect(m_comAnalyzer,SIGNAL(newUserData(rawData,UserData)),this,SLOT(on_newUserData(rawData,UserData)));
    connect(m_comAnalyzer,SIGNAL(newUserDataHeader(QStringList)),this,SLOT(on_newUserDataHeader(QStringList)));
    connect(m_comAnalyzer,SIGNAL(analyzerDataStringArrived(QString)),this,SLOT(on_analyzerDataStringArrived(QString)));
    connect(m_comAnalyzer,SIGNAL(analyzerScreenshotDataArrived(QByteArray)),this,SLOT(on_analyzerScreenshotDataArrived(QByteArray)));
    connect(m_comAnalyzer,SIGNAL(updatePercentChanged(int)),this,SLOT(on_updatePercentChanged(int)));
    connect(this, SIGNAL(screenshotComplete()),m_comAnalyzer,SLOT(on_screenshotComplete()));
    connect(m_comAnalyzer, SIGNAL(aa30bootFound()), this, SIGNAL(aa30bootFound()));
    connect(m_comAnalyzer, SIGNAL(aa30updateComplete()), this, SIGNAL(aa30updateComplete()));
}

void Analyzer::on_disconnectBluetooth()
{
    if (m_comAnalyzer != nullptr) {
        m_comAnalyzer->closeComPort();
        m_comAnalyzer->disconnect();
        m_comAnalyzer->deleteLater();
        m_comAnalyzer = nullptr;
    }
    m_connectionType = ReDeviceInfo::WRONG;
}

void Analyzer::on_connectBluetooth(QString portName)
{
    on_connectSerial(portName);
    m_connectionType = ReDeviceInfo::BT;
}

void Analyzer::applyAnalyzer()
{
    if(m_downloader == nullptr)
    {
        m_downloader = new Downloader();
        connect(m_downloader, SIGNAL(downloadInfoComplete()),
                this, SLOT(on_downloadInfoComplete()));
        connect(m_downloader, SIGNAL(sendStatisicsComplete()),
                this, SLOT(on_statisticsComplete()));
        connect(m_downloader, SIGNAL(downloadFileComplete()),
                this, SLOT(on_downloadFileComplete()));
        connect(m_downloader, SIGNAL(progress(qint64,qint64)),
                this, SLOT(on_progress(qint64,qint64)));
    }

    QString url = "https://www.rigexpert.com/getfirmware?part=antscope2&model=";
#ifndef NEW_ANALYZER
    url += names[m_analyzerModel].toLower().remove(" ").remove("-");
#else
    QString name = AnalyzerParameters::getName();
    url += name.toLower().remove(" ").remove("-");
#endif
    url += "&sn=" + getSerialNumber();
    url += "&revision=" + getRevision();
    url += "&os=" + QSysInfo::prettyProductName().replace(" ", "-").toLower();
    url += "&cpu=" + QSysInfo::currentCpuArchitecture();
    url += "&lang=" + QLocale::languageToString(QLocale::system().language());
    url += "&sw=" + QString(ANTSCOPE2VER);
    url += "&fw=" + getVersionString();

    qDebug() << url;
    m_downloader->startSendStatistics(QUrl(url));
}


void Analyzer::on_statisticsComplete()
{
    QSettings settings(Settings::setIniFile(), QSettings::IniFormat);
    settings.beginGroup("Update");
    settings.setValue("statistics_time", QDateTime::currentMSecsSinceEpoch());
    settings.endGroup();
}
//}

#ifdef NEW_CONNECTION
void Analyzer::connectDevice()
{
    SelectionParameters param = SelectionParameters::selected;
    BaseAnalyzer* tmp = m_baseAnalyzer;
    if (tmp != nullptr) {
        emit tmp->analyzerDisconnected();
        tmp->disconnect();
        tmp->deleteLater();
    }
    switch(param.type) {
    case ReDeviceInfo::HID:
    {
        m_baseAnalyzer = new HidAnalyzer(this);
    }
        break;
    case ReDeviceInfo::Serial:
    {
        m_baseAnalyzer = new ComAnalyzer(this);
    }
        break;
    case ReDeviceInfo::NANO:
    {
        m_baseAnalyzer = new NanovnaAnalyzer(this);
    }
        break;
    case ReDeviceInfo::BLE:
    {
        //m_baseAnalyzer = new HidAnalyzer(this);
    }
        break;
    default:
        return;
    }
    if (m_baseAnalyzer == nullptr)
        return;

    connect(m_baseAnalyzer, &BaseAnalyzer::analyzerFound, [=](quint32 analyzerNumber){
        m_analyzerModel = analyzerNumber;
        QString str = CustomAnalyzer::customized() ? CustomAnalyzer::currentAlias() : AnalyzerParameters::getName();
        emit analyzerFound(str);

    });
    connect(m_baseAnalyzer,SIGNAL(analyzerDisconnected()),this,SLOT(on_hidAnalyzerDisconnected()));

    connect(m_baseAnalyzer,SIGNAL(newData(rawData)),this,SLOT(on_newData(rawData)));
    connect(m_baseAnalyzer,SIGNAL(newUserData(rawData,UserData)),this,SLOT(on_newUserData(rawData,UserData)));
    connect(m_baseAnalyzer,SIGNAL(newUserDataHeader(QStringList)),this,SLOT(on_newUserDataHeader(QStringList)));
    connect(m_baseAnalyzer,SIGNAL(analyzerDataStringArrived(QString)),this,SLOT(on_analyzerDataStringArrived(QString)));
    connect(m_baseAnalyzer,SIGNAL(analyzerScreenshotDataArrived(QByteArray)),this,SLOT(on_analyzerScreenshotDataArrived(QByteArray)));
    connect(this, SIGNAL(screenshotComplete()),m_baseAnalyzer,SLOT(on_screenshotComplete()));
    connect(this, SIGNAL(measurementComplete()), m_baseAnalyzer, SLOT(on_measurementComplete()));//, Qt::QueuedConnection);
    connect(m_baseAnalyzer, &BaseAnalyzer::signalFullInfo, this, &Analyzer::slotFullInfo);
    connect(m_baseAnalyzer, &BaseAnalyzer::signalMeasurementError, this, [&](){
        emit this->signalMeasurementError();
    });
    connect(m_baseAnalyzer, &BaseAnalyzer::completeMeasurement, this, [&](){
        if (m_baseAnalyzer->type() == ReDeviceInfo::NANO)
            emit measurementCompleteNano();
    });
    m_baseAnalyzer->connectAnalyzer();
}
#endif

void Analyzer::on_disconnectDevice()
{
    //qDebug() << "Analyzer::on_disconnectDevice()";
    emit analyzerDisconnected();
    if (m_hidAnalyzer != nullptr)
        on_disconnectHid();
    if (m_comAnalyzer != nullptr)
        on_disconnectSerial();
    if (m_NanovnaAnalyzer != nullptr)
        on_disconnectNanoNVA();
    AnalyzerParameters::setCurrent(nullptr);
    m_connectionType = ReDeviceInfo::WRONG;

    QTimer::singleShot(2000, this, [&](){
        if (m_comAnalyzer != nullptr && m_comAnalyzer->connected())
            return;
        if (m_NanovnaAnalyzer != nullptr && m_NanovnaAnalyzer->isConnected())
            return;
        on_connectHid();
    });
}

#include "analyzerpro.h"
#include "popupindicator.h"
#include "customanalyzer.h"
#include <QDateTime>
#include "Notification.h"
#include "hid_analyzer.h"
#include "com_analyzer.h"
#include "nanovna_analyzer.h"
#include "ble_analyzer.h"
#include "settings.h"

// static member
#ifdef NEW_ANALYZER
QList<AnalyzerParameters*> AnalyzerParameters::m_analyzers;
AnalyzerParameters* AnalyzerParameters::m_current=nullptr;
#endif

AnalyzerPro::AnalyzerPro(QObject *parent) : QObject(parent),
    m_baseAnalyzer(nullptr),
    m_analyzerModel(0),
    m_chartCounter(0),
    m_isMeasuring(false),
    m_isContinuos(false),
    m_dotsNumber(100),
    m_downloader(nullptr),
    m_updateDialog(nullptr),
    m_pfw(nullptr),
    m_INFOSIZE(512),
    m_calibrationMode(false)
{
    m_pfw = new QByteArray;

#ifdef NEW_ANALYZER
    AnalyzerParameters::fill();
#endif
    // TODO
    //on_comAnalyzerDisconnected(); // create hidAnalyzer
}

AnalyzerPro::~AnalyzerPro()
{
    if(m_downloader)
    {
        delete m_downloader;
        m_downloader = nullptr;
    }
    delete m_pfw;
    if (m_baseAnalyzer != nullptr) {
        m_baseAnalyzer->deleteLater();
        m_baseAnalyzer = nullptr;
    }
}

ReDeviceInfo::InterfaceType AnalyzerPro::connectionType()
{
    if (m_baseAnalyzer != nullptr)
        return m_baseAnalyzer->connectionType();
    return ReDeviceInfo::WRONG;
}

double AnalyzerPro::getVersion() const
{
    if(m_baseAnalyzer != nullptr)
    {
        return m_baseAnalyzer->getVersion().toDouble();
    }
    return 0;
}

QString AnalyzerPro::getVersionString() const
{
    if(m_baseAnalyzer != nullptr)
    {
        return m_baseAnalyzer->getVersion();
    }
    return QString();
}

QString AnalyzerPro::getRevision() const
{
    if(m_baseAnalyzer != nullptr)
    {
        return m_baseAnalyzer->getRevision();
    }
    return QString();
}

void AnalyzerPro::on_downloadInfoComplete()
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

bool AnalyzerPro::needCheckForUpdate()
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

void AnalyzerPro::on_downloadFileComplete()
{
    *m_pfw = m_downloader->file();
    QBuffer fwdata(m_pfw);

    fwdata.open(QIODevice::ReadOnly);
    fwdata.seek(m_INFOSIZE);

    updateFirmware(&fwdata);
}

void AnalyzerPro::on_internetUpdate()
{
    m_downloader->startDownloadFw();
    m_updateDialog->setStatusText(tr("Downloading firmware..."));
}

void AnalyzerPro::readFile(QString pathToFw)
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

QString AnalyzerPro::getModelString( void )
{    
    return CustomAnalyzer::customized() ? CustomAnalyzer::currentPrototype() : AnalyzerParameters::getName();
}

quint32 AnalyzerPro::getModel( void )
{
    return m_analyzerModel;
}

QString AnalyzerPro::getSerialNumber(void) const
{
    if(m_baseAnalyzer != nullptr)
    {
        return m_baseAnalyzer->getSerial();
    }
    return QString();
}

void AnalyzerPro::on_measure (qint64 fqFrom, qint64 fqTo, qint32 dotsNumber)
{
    //qDebug() << "AnalyzerPro::on_measure()";
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

void AnalyzerPro::on_measureS21 (qint64 fqFrom, qint64 fqTo, qint32 dotsNumber)
{
    //qDebug() << "AnalyzerPro::on_measureS21()";
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
            m_baseAnalyzer->setIsS21Mode(true);
            m_baseAnalyzer->startMeasure(fqFrom, fqTo, m_dotsNumber);
            PopUpIndicator::setIndicatorVisible(true);
            return;
        }
    }
    on_stopMeasure();
}

void AnalyzerPro::on_measureContinuous(qint64 fqFrom, qint64 fqTo, qint32 dotsNumber)
{
    if(!m_isMeasuring)
    {
        setIsMeasuring(true);
        emit continueMeasurement(fqFrom, fqTo, dotsNumber);
        m_dotsNumber = dotsNumber;
        m_chartCounter = 0;
        if (m_baseAnalyzer != nullptr && m_baseAnalyzer->connectionType() != ReDeviceInfo::NANO)
        {
            m_baseAnalyzer->startMeasure(fqFrom,fqTo,dotsNumber);
            PopUpIndicator::setIndicatorVisible(true);
            return;
        }
    }
    on_stopMeasure();
}

void AnalyzerPro::on_measureUser (qint64 fqFrom, qint64 fqTo, qint32 dotsNumber)
{
    if(!m_isMeasuring)
    {
        setIsMeasuring(true);
        QDateTime datetime = QDateTime::currentDateTime();
        QString name = datetime.toString("##dd.MM.yyyy-hh:mm:ss");
        emit newMeasurement(name, fqFrom, fqTo, dotsNumber);
        m_dotsNumber = dotsNumber;
        m_chartCounter = 0;
        if (m_baseAnalyzer != nullptr && m_baseAnalyzer->connectionType() != ReDeviceInfo::NANO)
        {
            m_baseAnalyzer->setIsFRXMode(false);
            m_baseAnalyzer->startMeasure(fqFrom,fqTo,dotsNumber);
            PopUpIndicator::setIndicatorVisible(true);
            return;
        }
    }
    on_stopMeasure();
}

void AnalyzerPro::on_measureOneFq(QWidget* /*parent*/, qint64 fqFrom, qint32 /*dotsNumber*/)
{
    setIsMeasuring(true);
    m_dotsNumber = 100000;
    m_chartCounter = 0;
    if (m_baseAnalyzer != nullptr && m_baseAnalyzer->connectionType() != ReDeviceInfo::NANO)
    {
        m_baseAnalyzer->setIsFRXMode(true);
        m_baseAnalyzer->startMeasureOneFq(fqFrom,m_dotsNumber);
    }
}

void AnalyzerPro::on_stopMeasure()
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

void AnalyzerPro::updateFirmware (QIODevice *fw)
{
    if(m_baseAnalyzer != nullptr)
    {
        m_baseAnalyzer->update(fw);
    }
}

void AnalyzerPro::makeScreenshot()
{
    if(!m_isMeasuring)
    {

        if(m_baseAnalyzer != nullptr)
        {
            QTimer::singleShot(100, m_baseAnalyzer, &BaseAnalyzer::makeScreenshot);
        }
    }
}


void AnalyzerPro::on_newData(RawData _rawData)
{
    //qDebug() << "AnalyzerPro::on_newData" << _rawData.fq << _rawData.r << _rawData.x << (m_chartCounter) << (m_dotsNumber);
    if (m_getAnalyzerData) {
        emit newAnalyzerData (_rawData);
    } else {
        emit newData (_rawData);
    }

    // ???? if(m_chartCounter >= m_dotsNumber || !m_isMeasuring)
    quint32 finNum = m_calibrationMode ? m_dotsNumber : (m_dotsNumber-1);
    if(m_chartCounter > finNum || !m_isMeasuring)
    {
        //qDebug() << "AnalyzerPro::on_newData COMPLETE";
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

void AnalyzerPro::on_newS21Data(S21Data _s21Data)
{
    emit newS21Data (_s21Data);

    // ???? if(m_chartCounter >= m_dotsNumber || !m_isMeasuring)
    quint32 finNum = m_calibrationMode ? m_dotsNumber : (m_dotsNumber-1);
    if(m_chartCounter > finNum || !m_isMeasuring)
    {
        qDebug() << "AnalyzerPro::on_newS21Data COMPLETE";
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

void AnalyzerPro::on_newUserData(RawData _rawData, UserData _userData)
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

void AnalyzerPro::on_newUserDataHeader(QStringList fields)
{
    emit newUserDataHeader (fields);
}

void AnalyzerPro::on_analyzerDataStringArrived(QString str)
{
    emit analyzerDataStringArrived(str);
}

void AnalyzerPro::getAnalyzerData()
{
    if(!m_isMeasuring)
    {
        if(!isMeasuring() && m_baseAnalyzer != nullptr)
        {
            QTimer::singleShot(100, m_baseAnalyzer, SLOT(getAnalyzerData()));
        }
    }
}

void AnalyzerPro::closeAnalyzerData()
{
    if(m_baseAnalyzer != nullptr)
    {
        m_baseAnalyzer->setTakeData(false);
    }
}

void AnalyzerPro::on_itemDoubleClick(QString number, QString dotsNumber, QString name)
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

void AnalyzerPro::on_analyzerScreenshotDataArrived(QByteArray arr)
{
    emit analyzerScreenshotDataArrived(arr);
}

void AnalyzerPro::on_screenshotComplete(void)
{
    emit screenshotComplete();
}

void AnalyzerPro::on_updatePercentChanged(int number)
{
    if (m_updateDialog != nullptr)
        m_updateDialog->on_percentChanged(number);
    emit updatePercentChanged(number);
}

void AnalyzerPro::checkFirmwareUpdate()
{
    if (needCheckForUpdate())
    {
        on_checkUpdatesBtn_clicked();
        m_bManualUpdate = false;
    }
}

void AnalyzerPro::on_checkUpdatesBtn_clicked()
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
    QString name = AnalyzerParameters::getName();
    url += name.toLower().remove(" ").remove("-");
    url += "&sn=" + getSerialNumber();
    url += "&revision=" + getRevision();
    url += "&os=" + QSysInfo::prettyProductName().replace(" ", "-").toLower();
    url += "&cpu=" + QSysInfo::currentCpuArchitecture();
    url += "&lang=" + QLocale::languageToString(QLocale::system().language());
    url += "&sw=" + QString(ANTSCOPE2VER);
    url += "&fw=" + getVersionString();

    m_downloader->startDownloadInfo(QUrl(url));
}

void AnalyzerPro::on_progress(qint64 downloaded,qint64 total)
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

void AnalyzerPro::on_measureCalib(int dotsNumber)
{
    setIsMeasuring(true);
    m_dotsNumber = dotsNumber;
    m_chartCounter = 0;
    qint64 minFq_ = AnalyzerParameters::getMinFq().toULongLong()*1000;
    qint64 maxFq_ = AnalyzerParameters::getMaxFq().toULongLong()*1000;
    if (CustomAnalyzer::customized()) {
        CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
        if (ca != nullptr) {
            minFq_ = ca->minFq().toULongLong()*1000;
            maxFq_ = ca->maxFq().toULongLong()*1000;
        }
    }
    if(m_baseAnalyzer != nullptr)
    {
        m_baseAnalyzer->startMeasure(minFq_, maxFq_, dotsNumber);
    }
}

void AnalyzerPro::setCalibrationMode(bool enabled)
{
    m_calibrationMode = enabled;
}

void AnalyzerPro::setIsMeasuring (bool _isMeasuring)
{
    m_isMeasuring = _isMeasuring;
    if(m_baseAnalyzer != nullptr)
    {
        m_baseAnalyzer->setIsMeasuring(_isMeasuring);
    }
    PopUpIndicator::setIndicatorVisible(_isMeasuring);
}

void AnalyzerPro::setContinuos(bool _isContinuos)
{
    m_isContinuos = _isContinuos;
    if(m_baseAnalyzer != nullptr)
    {
        m_baseAnalyzer->setContinuos(_isContinuos);
    }
}

void AnalyzerPro::searchAnalyzer()
{
    // TODO
    if (!isMeasuring())
    {
        if (m_baseAnalyzer != nullptr)
            m_baseAnalyzer->searchAnalyzer();
    }
    //
}

bool AnalyzerPro::refreshConnection()
{
    bool ret = createDevice(SelectionParameters::selected);
    if (ret) {
        connectSignals();
        ret = m_baseAnalyzer->refreshConnection();
    }
    return ret;
}

bool AnalyzerPro::sendData(const QByteArray& data)
{
    bool ret = true;
    if (m_baseAnalyzer != nullptr) {
        m_baseAnalyzer->sendData(data);
    } else {
        ret = false;
    }
    return ret;
}

bool AnalyzerPro::sendCommand(const QString& _command)
{
    bool ret = true;
    if (m_baseAnalyzer != nullptr) {
        m_baseAnalyzer->sendCommand(_command);
    } else {
        ret = false;
    }
    return ret;
}

void AnalyzerPro::setParseState(int _state)
{
    if (m_baseAnalyzer != nullptr) {
        m_baseAnalyzer->setParseState(_state);
    }
}

int AnalyzerPro::getParseState()
{
    if (m_baseAnalyzer != nullptr) {
        return m_baseAnalyzer->getParseState();
    }
    return WAIT_NO;
}


void AnalyzerPro::on_connectDevice(BaseAnalyzer* analyzer)
{
    if (! createDevice(SelectionParameters::selected, analyzer)) {
        return;
    }
    connectSignals();
    m_baseAnalyzer->connectAnalyzer();
}

void AnalyzerPro::connectSignals()
{
    connect(m_baseAnalyzer, &BaseAnalyzer::analyzerFound, this, &AnalyzerPro::on_analyzerFound);
    connect(m_baseAnalyzer, &BaseAnalyzer::analyzerDisconnected, this, &AnalyzerPro::on_disconnectDevice);
    connect(this, &AnalyzerPro::measurementComplete, m_baseAnalyzer, &BaseAnalyzer::on_measurementComplete);//, Qt::QueuedConnection);
    connect(m_baseAnalyzer, &BaseAnalyzer::signalFullInfo, this, &AnalyzerPro::slotFullInfo);
    connect(m_baseAnalyzer, &BaseAnalyzer::signalMeasurementError, this, &AnalyzerPro::signalMeasurementError);
    connect(m_baseAnalyzer, &BaseAnalyzer::newData,this,&AnalyzerPro::on_newData);
    connect(m_baseAnalyzer, &BaseAnalyzer::newS21Data,this, &AnalyzerPro::on_newS21Data);
    connect(m_baseAnalyzer, &BaseAnalyzer::newUserData,this, &AnalyzerPro::on_newUserData);
    connect(m_baseAnalyzer,&BaseAnalyzer::newUserDataHeader,this, &AnalyzerPro::on_newUserDataHeader);
    connect(m_baseAnalyzer, &BaseAnalyzer::analyzerDataStringArrived,this, &AnalyzerPro::on_analyzerDataStringArrived);
    connect(m_baseAnalyzer,&BaseAnalyzer::analyzerScreenshotDataArrived,this, &AnalyzerPro::on_analyzerScreenshotDataArrived);
    connect(this, &AnalyzerPro::screenshotComplete, m_baseAnalyzer, &BaseAnalyzer::on_screenshotComplete);
    connect(m_baseAnalyzer, &BaseAnalyzer::signalAnalyzerError, this, &AnalyzerPro::signalAnalyzerError);
    connect(m_baseAnalyzer, &BaseAnalyzer::completeMeasurement, this, [=](){
        if (m_baseAnalyzer != nullptr) {
            m_baseAnalyzer->on_measurementComplete();
        }
       emit measurementCompleteNano();
    });
    connect(m_baseAnalyzer, &BaseAnalyzer::receivedMatch_12, this, [=](QByteArray data){
        emit signalMatch_12Received(data);
    });
    connect(m_baseAnalyzer, &BaseAnalyzer::receivedMatch_ProfileB16, this, [=](QByteArray data){
        emit signalMatch_Profile_B16Received(data);
    });
}

void AnalyzerPro::applyAnalyzer()
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
    QString name = AnalyzerParameters::getName();
    url += name.toLower().remove(" ").remove("-");
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


void AnalyzerPro::on_statisticsComplete()
{
    QSettings settings(Settings::setIniFile(), QSettings::IniFormat);
    settings.beginGroup("Update");
    settings.setValue("statistics_time", QDateTime::currentMSecsSinceEpoch());
    settings.endGroup();
}

bool AnalyzerPro::createDevice(const SelectionParameters& param, BaseAnalyzer* analyzer)
{
    BaseAnalyzer* tmp = m_baseAnalyzer;
    if (tmp != nullptr) {
        emit tmp->analyzerDisconnected();
        tmp->disconnect();
        tmp->deleteLater();
    }
    m_baseAnalyzer = nullptr;
    if (analyzer != nullptr) {
        m_baseAnalyzer = analyzer;
        return true;
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
        m_baseAnalyzer = new BleAnalyzer(this);
    }
        break;
    default:
        return false;
    }

    return m_baseAnalyzer != nullptr;
}

void AnalyzerPro::on_disconnectDevice()
{
    if (m_baseAnalyzer != nullptr) {
        BaseAnalyzer* tmp = m_baseAnalyzer;
        m_baseAnalyzer = nullptr;
        tmp->deleteLater();
    }
    emit deviceDisconnected();
}

void AnalyzerPro::slotFullInfo(const QString& _info)
{
    int index = _info.indexOf("LIC");
    if (index != -1) {
        QString _name = _info.mid(index, 4);
        qInfo() << "AnalyzerPro::slotFullInfo" << _name;
        if (_name == "LIC1")
            m_license = "ADVANCED";
        else if (_name == "LIC2")
            m_license = "RFE";
        else if (_name == "LIC3")
            m_license = "PRO";
        else
            m_license = "BASE";
    }
}

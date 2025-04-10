#include "ble_analyzer.h"
#include <QSysInfo>
#include <QOperatingSystemVersion>
#include <QDateTime>
#include "crc32.h"


BleAnalyzer::BleAnalyzer(QObject *parent)
    : BaseAnalyzer(parent)
{
    m_type = ReDeviceInfo::BLE;

    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(5000);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BleAnalyzer::addDevice);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &BleAnalyzer::scanError);

    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BleAnalyzer::scanFinished);
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &BleAnalyzer::scanFinished);

    m_pingTimer = new QTimer(this);
    connect(m_pingTimer, &QTimer::timeout, this, &BleAnalyzer::handlePing);
}

BleAnalyzer::~BleAnalyzer()
{
    stopPing();
    qDeleteAll(m_devices);
    m_devices.clear();
    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = nullptr;
    }
}

int BleAnalyzer::supported()
{
#ifdef Q_OS_DARWIN
    return BLE_SUPPORT_FULL;
#endif

#ifdef Q_OS_LINUX
    return BLE_SUPPORT_FULL;
#endif

    auto OSInfo = QOperatingSystemVersion ::current();
    auto OSType= OSInfo.type();
    if (OSType != 1)
        return BLE_SUPPORT_NONE;
    if (OSInfo < QOperatingSystemVersion::Windows10)
        return BLE_SUPPORT_NONE;
    if (OSInfo.microVersion() < 1607)
        return BLE_SUPPORT_PARTIAL;
    return BLE_SUPPORT_FULL;
}

QString BleAnalyzer::error() const
{
    return m_error;
}

void BleAnalyzer::setError(const QString& error)
{
    if (m_error != error) {
        m_error = error;
        emit errorChanged();
#ifdef _DEBUG
        emit signalAnalyzerError(error);
#endif
    }
}

QString BleAnalyzer::info() const
{
    return m_info;
}

void BleAnalyzer::setInfo(const QString& info)
{
    if (m_info != info) {
        m_info = info;
        emit infoChanged();
        qInfo() << "BleAnalyzer::SetInfo: " << m_info;
    }
}

void BleAnalyzer::setResponse(QString &text)
{
    emit response(text);
}

void BleAnalyzer::setRequest(QString &text)
{
    emit request(text);
}

void BleAnalyzer::clearMessages()
{
    setInfo("");
    setError("");
}

void BleAnalyzer::searchAnalyzer()
{
    qInfo() << "BleAnalyzer::searchAnalyzer()";
    m_serviceFound = false;
    clearMessages();
    setDevice(nullptr);
    qDeleteAll(m_devices);
    m_devices.clear();

    emit devicesChanged();

    m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    emit scanningChanged(1);
    setInfo(tr("Scanning for devices..."));
}

bool BleAnalyzer::connectAnalyzer()
{
    AnalyzerParameters* analyzer = AnalyzerParameters::byName(SelectionParameters::selected.name);
    if (analyzer == nullptr)
        return false;
    QString address = SelectionParameters::selected.id;
    connectToService(address);
    return true;
}

void BleAnalyzer::disconnectAnalyzer()
{

}

void BleAnalyzer::addDevice(const QBluetoothDeviceInfo &device)
{
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        auto deviceInfo = new BleDeviceInfo(device);
        QString name = deviceInfo->getName();
        qInfo() << "BleAnalyzer::addDevice.name: " << name;
        if (!AnalyzerParameters::supported(name))
            return;
        m_devices.append(deviceInfo);
        setInfo(tr("Low Energy device found. Scanning more..."));
        emit devicesChanged(deviceInfo);
    }
}

void BleAnalyzer::scanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        setError(tr("The Bluetooth adapter is powered off."));
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        setError(tr("Writing or reading from the device resulted in an error."));
    else
        setError(tr("An unknown error has occurred."));
}

void BleAnalyzer::scanFinished()
{
    if (m_devices.isEmpty())
        setError(tr("No Low Energy devices found."));
    else
        setInfo(tr("Scanning done."));

    if (m_innerScan) {
        connectAnalyzer();
    } else {
        emit scanningChanged(0);
    }
    setInnerScan(false);
    //emit devicesChanged();
}

void BleAnalyzer::connectToService(const QString &address)
{
    m_deviceDiscoveryAgent->stop();

    BleDeviceInfo *currentDevice = nullptr;
    for (BleDeviceInfo *entry : qAsConst(m_devices)) {
        auto device = entry;
        if (device && device->getAddress() == address ) {
            currentDevice = device;
            break;
        }
    }

    if (currentDevice)
        setDevice(currentDevice);

    clearMessages();
}

bool BleAnalyzer::scanning() const
{
   return m_deviceDiscoveryAgent->isActive();
}

void BleAnalyzer::setAddressType(AddressType type)
{
    switch (type) {
    case BleAnalyzer::AddressType::PublicAddress:
        m_addressType = QLowEnergyController::PublicAddress;
        break;
    case BleAnalyzer::AddressType::RandomAddress:
        m_addressType = QLowEnergyController::RandomAddress;
        break;
    }
}

BleAnalyzer::AddressType BleAnalyzer::addressType() const
{
    if (m_addressType == QLowEnergyController::RandomAddress)
        return BleAnalyzer::AddressType::RandomAddress;

    return BleAnalyzer::AddressType::PublicAddress;
}

QList<BleDeviceInfo*>& BleAnalyzer::devices()
{
    return m_devices;
}

void BleAnalyzer::setDevice(BleDeviceInfo *device)
{
    clearMessages();
    m_currentDevice = device;

    // Disconnect and delete old connection
    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = nullptr;
    }

    // Create new controller and connect it if device available
    if (m_currentDevice) {
        m_control = QLowEnergyController::createCentral(m_currentDevice->getDevice(), this);
        m_control->setRemoteAddressType(m_addressType);

        connect(m_control, &QLowEnergyController::serviceDiscovered,
                this, &BleAnalyzer::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished,
                this, &BleAnalyzer::serviceScanDone, Qt::QueuedConnection);

        connect(m_control, &QLowEnergyController::errorOccurred, this,
                [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    setError("Cannot connect to remote device.");
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            setInfo("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            setError("LowEnergy controller disconnected");
        });

        // Connect
        m_control->connectToDevice();
    }
}

void BleAnalyzer::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt == QBluetoothUuid(uuidService)) {
        setInfo("Service discovered. Waiting for service scan to be done...");
        m_serviceFound = true;
    }
}

void BleAnalyzer::serviceScanDone()
{
    // Delete old service if available
    if (m_service) {
        delete m_service;
        m_service = nullptr;
    }

    m_service = m_control->createServiceObject(QBluetoothUuid(uuidService), this);
    if (m_service) {
        connect(m_service, &QLowEnergyService::stateChanged, this, &BleAnalyzer::serviceStateChanged);
        connect(m_service, &QLowEnergyService::characteristicChanged, this, &BleAnalyzer::dataReceived);
        connect(m_service, &QLowEnergyService::descriptorWritten, this, &BleAnalyzer::confirmedDescriptorWrite);
        m_service->discoverDetails();
     } else {
        QString msg = tr("REU BLE Service not found.");
        setError(msg);
    }
}

// Service functions
void BleAnalyzer::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::RemoteServiceDiscovering:
        setInfo(tr("Discovering services..."));
        break;
    case QLowEnergyService::RemoteServiceDiscovered:
    {
        setInfo(tr("Service discovered."));
        setInfo("Characteristics:");
//        for (auto chara : m_service->characteristics()) {
//            QString type;
//            chara.properties();
//            setInfo(chara.uuid().toString() + QString(" 0x%1").arg(chara.properties(), 2, 16, QChar('0')));
//        }
        const QLowEnergyCharacteristic hrChar = m_service->characteristic(QBluetoothUuid(uuidRead));
        if (!hrChar.isValid()) {
            setError("'Read characteristic' not found.");
            break;
        }
        const QLowEnergyCharacteristic hrReturn = m_service->characteristic(QBluetoothUuid(uuidReturn));
        m_reuChip = hrReturn.isValid();

        m_notificationDesc = hrChar.descriptor(QBluetoothUuid(uuidDescriptor));
        if (m_notificationDesc.isValid())
            m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
        int model = SelectionParameters::selected.modelIndex;
        if (model != -1)
            emit analyzerFound(model);
        startPing();
        m_lastReadTimeMS = QDateTime::currentMSecsSinceEpoch();
        break;
    }
    default:
        //nothing for now
        break;
    }
    emit aliveChanged();
}

void BleAnalyzer::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    setInfo("Descriptor:");
    setInfo(d.uuid().toString() + "  " + m_notificationDesc.uuid().toString());
    if (d.isValid() && d == m_notificationDesc && value == QByteArray::fromHex("0000")) {
        stopPing();
        setInfo("disabled notifications -> assume disconnect intent");
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = nullptr;
    }
}

void BleAnalyzer::startPing()
{
    m_pingTimer->start(1000);
    sendFullInfo();
}

void BleAnalyzer::stopPing()
{
    m_pingTimer->stop();
}

void BleAnalyzer::sendPing()
{
    m_bWaitingPing = true;
    QByteArray ping;
    ping.fill(0, BLE_PACKET_SIZE);
    ping[0] = (quint8)0x5a;
    ping[BLE_PACKET_SIZE - 1] = CRC32::crc8(ping);
    write(ping);
}
/*
void BleAnalyzer::handlePing() //1sec timer
{
    long cur = QDateTime::currentMSecsSinceEpoch();
    if ((cur - m_lastReadTimeMS) >= PING_TIMEOUT_MS) {
        if (m_bWaitingPing) {
            // error
            // TODO...
            AnalyzerParameters::setCurrent(nullptr);

            m_pingTimer->stop();
            QString err = tr("Analyzer disconnected");
            setError(err);
            emit analyzerDisconnected();
        } else {
            sendPing();
        }
    } else {
        m_bWaitingPing = false;
    }
}
*/
void BleAnalyzer::handlePing() //vnn_05 1sec timer
{
    long cur = QDateTime::currentMSecsSinceEpoch();
    long t_noRx =cur - m_lastReadTimeMS;
    long t_noFRX =cur - m_frxTime;

    //-----------for FRX point miss----
    if ((t_noFRX >= 900)&&(isMeasuring())&&(m_frxGo==1)) {
      m_frxGo=0;
      QString str = QString("LostFRX ! %1 / %2").arg(m_frxCur).arg(m_requestRecord.m_requestPoints);
      qInfo() << str;
      emit completeMeasurement();
     }

    //-----------for ping--------------
    if (t_noRx >= PING_TIMEOUT_MS) {
        if ((m_bWaitingPing)&&(t_noRx>(2*PING_TIMEOUT_MS))) {
            // error
            // TODO...
            AnalyzerParameters::setCurrent(nullptr);
            m_pingTimer->stop();
            QString err = tr("Analyzer disconnected");
            setError(err);
            emit analyzerDisconnected();
        } else {
            sendPing();// m_bWaitingPing = true;
        }
    } else {
        m_bWaitingPing = false;
    }
}



void BleAnalyzer::write(QByteArray& arr)
{
    //qInfo() << trace("write: ", arr);

    if (m_service == nullptr) {
        setError("REU BLE service not found");
        return;
    }
    const QLowEnergyCharacteristic hrChar = m_service->characteristic(QBluetoothUuid(uuidWrite));
    if (!hrChar.isValid()) {
        setError("Write chracteristic doesn't exist!");
        //qInfo() << "BleAnalyzer::write " << error();
        return;
    }
    m_insideWrite = true;
    m_service->writeCharacteristic(hrChar, arr);
    m_insideWrite = false;
}

void BleAnalyzer::dataReceived(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    if (c.uuid() != QBluetoothUuid(uuidRead))
        return;
    qInfo() << trace("dataReceived: ", const_cast<QByteArray&>(value));
    m_lastReadTimeMS = QDateTime::currentMSecsSinceEpoch();
    if (!checkCRC(value)) {
        qInfo() << "errorCRC";
        QString err = tr("Analyzer error: wrong CRC");
        setError(err);
        emit crcError();
        return;
    }
    if (value[0] == (quint8)BLE_PING_CMD) {
        returnCRC(value);
        return;
    }
    parseResponse(value);
}

void BleAnalyzer::returnCRC(const QByteArray &data)
{
    if (!m_reuChip || m_insideWrite) {
        return;
    }
    QByteArray crc;
    crc.append(data.at(BLE_PACKET_SIZE-1));

    m_charReturn = m_service->characteristic(QBluetoothUuid(uuidReturn));
    if (m_charReturn.isValid())
        m_service->writeCharacteristic(m_charReturn, crc);
}

bool BleAnalyzer::checkCRC(const QByteArray &data)
{
    quint8 crc = CRC32::crc8(data);
    quint8 crc0 = (quint8)data[BLE_PACKET_SIZE-1];
    //qInfo() << "checkCRC" << crc0 << crc;
    return crc == crc0;
}

void BleAnalyzer::parseResponse(const QByteArray& arr)
{
    returnCRC(arr);

    QString str;
    QDataStream stream = QDataStream(arr);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    quint8 cmd;
    stream >> cmd;
    // fullinfo
    switch (cmd) {
    case (quint8)BLE_FULLINFO_CMD: {
        parseFullInfo(stream);
    }
    break;
    case (quint8)BLE_FRX_CMD: {
        parseFRX(stream);
        return;
    }
    break;
    case (quint8)BLE_SCREENSHOT_CMD: {
        parseScreenShot(arr);
    }
    break;
    case (quint8)BLE_REC_LIST_CMD: {
        parseRecList(stream);
    }
    break;
    case (quint8)BLE_REC_DATA_CMD: {
        parseRecData(stream);
    }
    break;
    default:
        break;
    }
}

void BleAnalyzer::parseRecList(QDataStream& stream)
{
    qint8 msg_num;
    stream >> msg_num;
    switch (msg_num) {
    case 0: {
        m_requestRecord.reset();
        qint8 cell;
        stream >> m_requestRecord.m_requestCenter >> m_requestRecord.m_requestRange >> cell;
        m_requestRecord.m_requestCenter /= 1000;
        m_requestRecord.m_requestRange /= 1000;
        m_requestRecord.m_recordCell = int(cell & 0xFF);
    }
        break;
    case 1: {
        stream >> m_requestRecord.m_requestPoints ;
        m_requestRecord.m_requestPoints--; //HUCK for measurement complete
        m_requestRecord.m_recordName = bytesToString(stream);
    }
        break;
    case 2: {
        QString name = bytesToString(stream);
        if (!name.isEmpty()) {
            m_requestRecord.m_recordName += name;
        }
        m_analyzerRecords.insert(QString::number(m_requestRecord.m_recordCell), m_requestRecord);
        QString str = m_requestRecord.record();
        emit analyzerDataStringArrived(str);
    }
        break;
    default: {

    }
        break;
    }
}

void BleAnalyzer::parseRecData(QDataStream& stream)
{
    parseFRX(stream);
}

void BleAnalyzer::parseFRX(QDataStream& stream)
{
    RawData data;
    m_frxTime= QDateTime::currentMSecsSinceEpoch();
    if (!m_reuChip) {
        quint64 fq_Hz =0;
        float r1, x1;
        qint16 count;

        stream >> fq_Hz >> r1 >> x1 >> count;

        float fq_MHz = fq_Hz / 1000000.0f;
        QString str = QString("parseFRX: %1, %2, %3, %4").arg(fq_MHz).arg(r1).arg(x1).arg(count);
        qInfo() << str;
//        setResponse(str);

        data.fq = fq_MHz;
        data.r = r1;
        data.x = x1;
        m_frxCur++;
        emit newData(data);
    } else {
        float start = m_requestRecord.m_requestCenter - m_requestRecord.m_requestRange / 2;
        float stop = start + m_requestRecord.m_requestRange;
        float step = (stop - start) / (float)m_requestRecord.m_requestPoints;
        qint16 id;
        stream >> id;
        // if (m_name == "Match")
        //    id -= 1;
        // if (m_name == "MATCH U")
        //    id -= 1;
        if((id%2)>0){  //for: Match, MATCH U, AA-3000, AA-2000, AA-1500...
            id -= 1;
        }
        float fq_kHz;
        for (int i=0; i<4; i++) {
            qint16 sr, sx,n_curPnt;
            stream >> sr >> sx;
            n_curPnt=id+i;
            if( (!((sr==0x0000)&&(sx==0x0000))) && (n_curPnt<=m_requestRecord.m_requestPoints) ){
                float dr = shortToDouble(sr);
                float dx = shortToDouble(sx);
                fq_kHz = (start + (id+i)*step);
                data.fq = fq_kHz / 1000.0;
                data.r = dr;
                data.x = dx;
                m_frxCur++;
                emit newData(data);
            } else {
                data.r =1;
            }
        }
    }
}

void BleAnalyzer::parseScreenShot(const QByteArray &arr)
{
    QByteArray ar = arr.mid(1, arr.length()-2);
    emit analyzerScreenshotDataArrived(ar);
}

void BleAnalyzer::parseFullInfo(QDataStream& stream)
{
    QString str;
    quint8 field;
    stream >> field;
    // TODO license support
    // ...

    switch (field) {
    case (quint8)BLE_FULLINFO_NAME:
    {
        str = bytesToString(stream);
        m_name = str;
        setResponse(str);
        //qInfo() << str;
    }
        break;
    case (quint8)BLE_FULLINFO_FLAGS:
    {
        quint64 flags;
        stream >> flags;
        QString str = QString("Flags: 0x%1").arg(flags, 8, 16, QChar('0'));
        setResponse(str);
    }
        break;
    case (quint8)BLE_FULLINFO_MEASURER:
    {
        quint32 minFq, maxFq;
        qint8 extra_mult;
        quint16 maxPoints;
        stream >> minFq >> maxFq >> extra_mult >> maxPoints;
        if (m_name == "Match") {
            AnalyzerParameters* par = AnalyzerParameters::byName(m_name);
            if (par != nullptr) {
                par->setMinFq(QString::number(minFq));
                par->setMaxFq(QString::number(maxFq));
            }
        }
        double mult = qPow(10, 3-extra_mult);
        QString str = QString("FQ min = %1 kHz, FQ max = %2 kHz, max Points = %3")
                .arg(long(minFq * mult)).arg(long(maxFq*mult)).arg(maxPoints);
        setResponse(str);
    }
        break;
    case (quint8)BLE_FULLINFO_SERIAL_VER:
    {
        QString serial = bytesToString(stream);
        m_serialNumber = serial;
        quint16 major, minor, build;
        stream >> major >> minor >> build;
        str = "S/N: " + serial;
        setResponse(str);
        str = QString("Version: %1.%2.%3").arg(major).arg(minor).arg(build);
        setResponse(str);
    }
        break;
    case (quint8)BLE_FULLINFO_MCU_TYPE_STR:
    {
        QString mcu = bytesToString(stream);
        str = "MCU type: " + mcu;
        setResponse(str);
    }
        break;
    case (quint8)BLE_FULLINFO_DISPLAY:
    {
        quint16 wd, ht;
        quint8 bpp, pix;
        stream >> wd >> ht >> bpp >> pix;
        str = "Display:"; setResponse(str);
        str = QString("  width %1").arg(wd); setResponse(str);
        str = QString("  height %1").arg(ht); setResponse(str);
        str = QString("  bytes per pixel %1").arg(bpp); setResponse(str);
        str = QString("  pixel compression %1").arg(pix); setResponse(str);
    }
        break;
    case (quint8)BLE_FULLINFO_FLASH:
    {
        QString flash = bytesToString(stream);
        str = "Flash type: " + flash;
        setResponse(str);
    }
        break;
    case (quint8)BLE_FULLINFO_HW_STR:
    {
        QString hw = bytesToString(stream);
        str = "HW: " + hw;
        setResponse(str);
    }
        break;
    }
}

QString BleAnalyzer::trace(QString _title, QByteArray& _data) {
     QString msg = "";
     for (int idx = 0; idx < _data.size(); idx++) {
         msg += QString("%1 ").arg((quint8)_data[idx], 2, 16, QChar('0'));
     }
     //qInfo() << _title << msg;
     return QString("%1: %2").arg(_title, msg);
}

QString BleAnalyzer::bytesToString(QDataStream& stream)
{
    QString str;
    for(;;) {
        if (stream.atEnd())
            break;
        quint8 ch;
        stream >> ch;
        if (ch == 0x00)
            break;
        str += QChar(ch);
    }
    return str;
}

double BleAnalyzer::shortToDouble(qint16 src)
{
    int div_pow = ((src & (3 << 13)) >> 13);
    int sign = (src & (1 << 15));
    int value = (src & 0x1FFF);
    double result = value * 123.0 / pow(10, div_pow + 2);
    if (sign != 0)
        result *= -1;
    return result;
}

void BleAnalyzer::startMeasure(qint64 from_hz, qint64 to_hz, int dotsNumber, bool _frx)
{
    Q_UNUSED(_frx)
    QByteArray data;
    data.append((quint8)BLE_FRX_CMD);

    int start = (int)(from_hz / 1000);
    int stop = (int)(to_hz / 1000);

    int sw = stop-start;
    int fq = start + sw/2;
    int frx = dotsNumber;

    m_requestRecord.m_requestCenter = fq;
    m_requestRecord.m_requestRange = sw;
    m_requestRecord.m_requestPoints = frx;

    QByteArray bytesFQ = QByteArray::fromRawData(reinterpret_cast<const char *>(&fq), sizeof(fq));
    data.append(bytesFQ);
    QByteArray bytesSW = QByteArray::fromRawData(reinterpret_cast<const char *>(&sw), sizeof(sw));
    data.append(bytesSW);
    QByteArray bytesFRX = QByteArray::fromRawData(reinterpret_cast<const char *>(&frx), sizeof(frx));
    data.append(bytesFRX);
    quint8 zero = 0;
    for (;;) {
        if (data.size() == 20)
            break;
        data.append(zero);
    }
    data[19] = CRC32::crc8(data);

    QString msg = QString("FQ: %1, SW: %2, FRX: %3").arg(fq).arg(sw).arg(frx);
    qInfo() << "BleAnalyzer::startMeasure" << msg;
    setRequest(msg);
    QString sss = trace("FRX", data);
    setRequest(sss);

    write(data);
    m_frxCur=0;
    m_frxTime= QDateTime::currentMSecsSinceEpoch();
    m_frxGo=true;
}

void BleAnalyzer::startMeasureOneFq(qint64 fqFrom_hz, int dotsNumber, bool frx)
{
    startMeasure(fqFrom_hz, fqFrom_hz, dotsNumber, frx);
}

void BleAnalyzer::sendFullInfo()
{
    QByteArray data;
    data.fill(0, BLE_PACKET_SIZE);
    data[0] = (quint8)BLE_FULLINFO_CMD;
    data[BLE_PACKET_SIZE - 1] = CRC32::crc8(data);
    write(data);
}

void BleAnalyzer::getAnalyzerData()
{
    QByteArray data;
    data.fill(0, BLE_PACKET_SIZE);
    data[0] = (quint8)BLE_REC_LIST_CMD;
    data[BLE_PACKET_SIZE - 1] = CRC32::crc8(data);
    write(data);
    m_analyzerRecords.clear();
}

void BleAnalyzer::getAnalyzerData(QString number)
{
    if (!m_analyzerRecords.contains(number))
        return;

    setTakeData(true);
    setIsMeasuring(true);

    m_requestRecord = m_analyzerRecords[number];

    QByteArray data;
    data.append((quint8)BLE_REC_DATA_CMD);
    data.append((quint8)m_requestRecord.m_recordCell);

    qint32 center = (qint32)m_requestRecord.m_requestCenter;
    qint32 range = (qint32)m_requestRecord.m_requestRange;
    qint32 points = (qint32)m_requestRecord.m_requestPoints;
    QByteArray bytesFQ = QByteArray::fromRawData(reinterpret_cast<const char *>(&center), sizeof(center));
    data.append(bytesFQ);
    QByteArray bytesSW = QByteArray::fromRawData(reinterpret_cast<const char *>(&range), sizeof(range));
    data.append(bytesSW);
    QByteArray bytesFRX = QByteArray::fromRawData(reinterpret_cast<const char *>(&points), sizeof(points));
    data.append(bytesFRX);
    quint8 zero = 0;
    for (;;) {
        if (data.size() == 20)
            break;
        data.append(zero);
    }
    data[19] = CRC32::crc8(data);
    //qInfo() << "getAnalyzerData: " << number << center << range << points << m_requestRecord.m_recordName;
    //qInfo() << trace(number, data);
    write(data);
}

void BleAnalyzer::makeScreenshot()
{
    QByteArray data;
    data.fill(0, BLE_PACKET_SIZE);
    data[0] = (quint8)BLE_SCREENSHOT_CMD;
    data[BLE_PACKET_SIZE - 1] = CRC32::crc8(data);
    write(data);
}

void BleAnalyzer::stopMeasure()
{
    m_isMeasuring = false;
}

bool BleAnalyzer::refreshConnection()
{
    setInnerScan(true);
    searchAnalyzer();
    return true;
}

#include "licenseagent.h"
#include "popup.h"
#include "modelesspopup.h"
#include "mainwindow.h"
#include "unitrequestdialog.h"
#include "encodinghelpers.h"

LicenseAgent::LicenseAgent(QObject *parent) :
    QObject(parent),
    m_state(Finished),
    m_reply(NULL)
{
    connect(&m_mng, &QNetworkAccessManager::finished, this, &LicenseAgent::onReplyFinished);
    connect(m_reply, &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError error) {
        qInfo() << "***       QNetworkReply::errorOccurred" << error;
        QString str = QString("QNetworkReply::errorOccurred: %1").arg(error);
        showModeless(QString(tr("Error")), str, "Cancel");
    });
    connect(&m_timer, &QTimer::timeout, this, [=](){
        m_timer.stop();
        timeout();
        showModeless(QString(tr("Timeout")), tr("Network timeout"), "Cancel");
    });
    m_timer.setSingleShot(true);
}

LicenseAgent::~LicenseAgent()
{

}

void LicenseAgent::registerApllication(QString user, QString email)
{
    if (m_state == WaitEmailStatusWeb) {
        return;
    }

    m_userName = user;
    m_email = email;
    showModeless(tr("Register application"), tr("Waiting for email confirmation..."), tr("Cancel"));
    connect(m_modelessPopup, &ModelessPopup::rejected, this, [=](){
        m_timer.stop();
    });
    qInfo() << "LicenseAgent::registerApllication";
    m_dtEmailStatus = QDateTime::currentMSecsSinceEpoch();
    requestEmailStatus();
}

void LicenseAgent::requestEmailStatus()
{
    QString url = SERVER_NAME;
    QString strRaw = QString("Name=%1&&&Eml=%2&&&").arg(m_userName, m_email);
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=1&nRaw=1&raw=%1").arg(strData);

    qInfo() << "    LicenseAgent::requestEmailStatus" << url;

    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(TRANSFER_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    m_state = WaitEmailStatusWeb;
    m_mng.get(request);
}

void LicenseAgent::parseEmailStatus()
{
    QString status(m_arr);
    qInfo() << "LicenseAgent::parseEmailStatus" << status;
    QString rez = EncodingHelpers::decodeString_nRaw1(status);
    qInfo() << "LicenseAgent::parseEmailStatus decoded" << rez;
    QStringList list = rez.split("&&&");
    for (int var = 0; var < list.size(); ++var) {
        QString field = list.at(var);
        QStringList lst = field.split('=');
        if (lst.size() != 2) {
            continue;
        }
        if (lst.at(0) == "Status" && lst.at(1) == "1") {
            m_emailStatusWeb = true;
        }
    }
    if (m_emailStatusWeb)
        return;
    qInfo() << "LicenseAgent::parseEmailStatus";
    //requestEmailStatus();
}

void LicenseAgent::registerDevice(QString device_name, QString serial, QString license)
{
    m_infoRequest.deviceName = device_name;
    m_infoRequest.serialNumber = serial;
    m_infoRequest.licenseName = license;
    requestInfo();
}

void LicenseAgent::updateLicense()
{
    QString key = QInputDialog::getText((QWidget*)MainWindow::m_mainWindow, tr("Renew license"), tr("Enter key"));
    if (key.isNull())
        return;
    requestLicense(key);
}

void LicenseAgent::requestLicense(QString key)
{
    // send special command instead of HELLO
    QString name = MainWindow::m_mainWindow->analyzer()->getModelString();
    QString serial = MainWindow::m_mainWindow->analyzer()->getSerialNumber();
    QString url = SERVER_NAME;
    QString strRaw = QString("dvName=%1&&&dvSN=%2&&&lcCode=%3&&&").arg(name, serial, key);
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=4&nRaw=1&raw=%1").arg(strData);

    qInfo() << "   LicenseAgent::requestLicense" << url;

    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(TRANSFER_TIMEOUT);

    m_mng.clearAccessCache();
//    QSslConfiguration conf = request.sslConfiguration();
//    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
//    request.setSslConfiguration(conf);

    m_state = WaitLicense;
    m_mng.get(request);

}

void LicenseAgent::parseLicense()
{
    QString licenseWeb(m_arr);

    qInfo() << "    LicenseAgent::parseLicense" << licenseWeb;

    QString rez = EncodingHelpers::decodeString_nRaw1(licenseWeb);
    QStringList list = rez.split("&&&");
    for (int var = 0; var < list.size(); ++var) {
        QString field = list.at(var);
        QStringList lst = field.split('=');
        if (lst.size() != 2) {
            continue;
        }
        if (lst.at(0) == "dvName") {
            m_licenseWeb.deviceName = lst.at(1);
        } else  if (lst.at(0) == "dvSN") {
            m_licenseWeb.serialNumber = lst.at(1);
        } else  if (lst.at(0) == "Status") {
            m_licenseWeb.status = lst.at(1);
        }
    }
}

void LicenseAgent::reset()
{
    m_arr.clear();
}

void LicenseAgent::timeout()
{
    if (m_reply != NULL) {
        m_reply->abort();
        emit m_reply->finished();
        reset();
    }
}

void LicenseAgent::updateUserData()
{
    ManualInfoWeb infoWeb;
    infoWeb.email = m_unitWeb.email.isEmpty() ? m_email : m_unitWeb.email;
    infoWeb.purchargeDate = m_unitWeb.purchargeDate.isEmpty() ? QDate::currentDate().toString("dd.MM.yyyy") : m_unitWeb.purchargeDate;
    infoWeb.serialNumber = m_unitWeb.serialNumber.isEmpty() ? MainWindow::m_mainWindow->analyzer()->getSerialNumber() : m_unitWeb.serialNumber;
    infoWeb.userName = m_unitWeb.userName.isEmpty() ? m_userName : m_unitWeb.userName;
    infoWeb.licenseName = MainWindow::m_mainWindow->analyzer()->getLicense();

    UnitRequestDialog dlg(infoWeb);
    if (dlg.exec() == QDialog::Rejected) {
        emit canceled();
        return;
    }
    m_unitRequest.email = dlg.infoWeb().email;
    m_unitRequest.purchargeDate = dlg.infoWeb().purchargeDate;
    m_unitRequest.serialNumber = dlg.infoWeb().serialNumber;
    m_unitRequest.userName = dlg.infoWeb().userName;
    requestUnit();

}

void LicenseAgent::showModeless(QString title, QString text, QString buttonCancel, QString buttonOk)
{
    if (m_modelessPopup != nullptr) {
        m_modelessPopup->disconnect();
        m_modelessPopup->setVisible(false);
        m_modelessPopup->close();
        m_modelessPopup->deleteLater();
    }
    m_modelessPopup = new ModelessPopup(title, text, buttonCancel, buttonOk, MainWindow::m_mainWindow);
    connect(m_modelessPopup, &ModelessPopup::rejected, this, [=](){
        emit canceled();
    });
    connect(m_modelessPopup, &ModelessPopup::accepted, this, [=](){
        emit accepted();
    });
    m_modelessPopup->setModal(false);
    m_modelessPopup->show();
    m_modelessPopup->activateWindow();
    m_modelessPopup->setFocus();
}

void LicenseAgent::closeModeless()
{
    if (m_modelessPopup != nullptr) {
        m_modelessPopup->disconnect();
        m_modelessPopup->setVisible(false);
        m_modelessPopup->close();
        m_modelessPopup->deleteLater();
        m_modelessPopup = nullptr;
    }

}

void LicenseAgent::requestInfo()
{
    QString url = SERVER_NAME;
    QString strRaw = QString("dvName=%1&&&dvSN=%2&&&lcName=%3&&&")
                         .arg(m_infoRequest.deviceName, m_infoRequest.serialNumber, m_infoRequest.licenseName);
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=2&nRaw=1&raw=%1").arg(strData);
    qInfo() << "LicenseAgent::requestInfo()" << url;

    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(TRANSFER_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    m_state = WaitInfoWeb;
    m_mng.get(request);
}

void LicenseAgent::parseInfoWeb()
{
    QString infoWeb(m_arr);
    int pos = infoWeb.indexOf("&nRez=");
    int pos1 = infoWeb.indexOf("&", pos+1);
    QString nrez = infoWeb.mid(pos+6, pos1-pos+6);
    m_infoWeb.nRez = nrez.toInt();
    QString rez = EncodingHelpers::decodeString_nRaw1(infoWeb);
    qInfo() << "LicenseAgent::parseInfoWeb" << infoWeb;
    qInfo() << "LicenseAgent::parseInfoWeb decoded" << rez;
    QStringList list = rez.split("&&&");
    for (int var = 0; var < list.size(); ++var) {
        QString field = list.at(var);
        QStringList lst = field.split('=');
        if (lst.size() != 2) {
            continue;
        }
        if (lst.at(0) == "dvName") {
            m_infoWeb.deviceName = lst.at(1);
        } else  if (lst.at(0) == "dvName") {
            m_infoWeb.deviceName = lst.at(1);
        } else  if (lst.at(0) == "dvSN") {
            m_infoWeb.serialNumber = lst.at(1);
        } else  if (lst.at(0) == "lcName") {
            m_infoWeb.licenseName = lst.at(1);
        }  else  if (lst.at(0) == "dtPur") {
            m_infoWeb.purchargeDate = lst.at(1);
        } else  if (lst.at(0) == "dtLog") {
            m_infoWeb.loginDate = lst.at(1);
        }
    }
}

bool LicenseAgent::infoWebIsEmpty()
{
    return m_infoWeb.serialNumber.isEmpty();
}

void LicenseAgent::requestUnit()
{
    QString url = SERVER_NAME;
    QString strRaw = QString("dvSN=%1&&&Eml=%2&&&Name=%3&&&dtPur=%4&&&")
                         .arg(m_unitRequest.serialNumber, m_unitRequest.email, m_unitRequest.userName, m_unitRequest.purchargeDate);
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=3&nRaw=1&raw=%1").arg(strData);

    qInfo() << "LicenseAgent::requestUnit" << url;
    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(TRANSFER_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    m_state = WaitUnitWeb;
    m_mng.get(request);
    showModeless(tr("Register device"),tr("Registration..."), tr("Cancel"));
}


void LicenseAgent::parseUnitWeb()
{
    QString unitWeb(m_arr);
    qInfo() << "LicenseAgent::parseUnitWeb" << unitWeb;
    int pos = unitWeb.indexOf("&nRez=");
    int pos1 = unitWeb.indexOf("&", pos+1);
    QString nrez = unitWeb.mid(pos+6, pos1-pos+6);
    m_unitWeb.nRez = nrez.toInt();
    QString rez = EncodingHelpers::decodeString_nRaw1(unitWeb);
    qInfo() << "LicenseAgent::parseUnitWeb decoded" << rez;
    QStringList list = rez.split("&&&");
    for (int var = 0; var < list.size(); ++var) {
        QString field = list.at(var);
        QStringList lst = field.split('=');
        if (lst.size() != 2) {
            continue;
        }
        if (lst.at(0) == "dvName") {
            m_unitWeb.deviceName = lst.at(1);
        } else  if (lst.at(0) == "dvSN") {
            m_unitWeb.serialNumber = lst.at(1);
        } else  if (lst.at(0) == "Eml") {
            m_unitWeb.email = lst.at(1);
        } else  if (lst.at(0) == "EmlST") {
            m_unitWeb.emailStatus = lst.at(1) == "0";
        } else  if (lst.at(0) == "Name") {
            m_unitWeb.userName = lst.at(1);
        }  else  if (lst.at(0) == "dtPur") {
            m_unitWeb.purchargeDate = lst.at(1);
        } else  if (lst.at(0) == "dtLog") {
            m_unitWeb.loginDate = lst.at(1);
        }
    }
}

bool LicenseAgent::isUnitWebValid()
{
    return (m_unitWeb.nRez == 2 || m_unitWeb.nRez == 1);
}

void LicenseAgent::requestStatus_B16(QByteArray data)
{

    QString dataStr(data);
    if (dataStr.contains("Error")) {
        if (m_modelessPopup != nullptr)
            m_modelessPopup->close();
        showModeless(tr("Request status B16"), tr("Something went wrong")), tr("Ok");
        return;
    }
    QString url = SERVER_NAME;
    QString strData(data.toHex());
    url += QString("?nGet=21&nRaw=21&raw=073E%1").arg(strData.toUpper());
    url += "85";

    qInfo() << "LicenseAgent::requestStatus_B16" << url;

    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(TRANSFER_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    m_state = WaitProfileB16;
    m_mng.get(request);
}

void LicenseAgent::requestInfo_B16(QByteArray data)
{
    QString url = SERVER_NAME;
    QString strData(data.toHex());
    url += QString("?nGet=22&nRaw=21&raw=073E%1").arg(strData.toUpper());
    url += "85";

    qInfo() << "LicenseAgent::requestInfo_B16" << url;

    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(TRANSFER_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    m_state = WaitInfoB16;
    m_mng.get(request);
}

void LicenseAgent::onReplyFinished(QNetworkReply* reply)
{
    qInfo() << "LicenseAgent::onReplyFinished: " << reply->errorString();
    m_arr = reply->readAll();

    qInfo() << "    LicenseAgent::onReplyFinished: state=" << m_state << ", " << QString(m_arr);
    if (m_state == WaitEmailStatusWeb) {
        finishWaitEmailStatusWeb();
    } else if (m_state == WaitInfoWeb) {
        finishWaitInfoWeb();
    } else if(m_state == WaitUnitWeb) {
        finishWaitUnitWeb();
    } else if(m_state == WaitLicense) {
        finishWaitLicense();
    } else if (m_state == WaitInfoB16) {
        finishWaitInfoB16();
    } else if (m_state == WaitProfileB16) {
        finishWaitProfileB16();
    }
}

void LicenseAgent::finishWaitEmailStatusWeb()
{
    parseEmailStatus();
    if (m_emailStatusWeb) {
        // close notification
        if (m_popup != nullptr) {
            delete m_popup;
            m_popup = nullptr;
        }
        showModeless(tr("Register application"),tr("Registration was successful"), tr("Ok"));

        QSettings& set = *MainWindow::m_mainWindow->settings();
        set.beginGroup("Mainwindow");
        set.setValue("eMail", m_email);
        set.setValue("remind", false);
        set.setValue("userName", m_userName);
        set.endGroup();
        emit registered();
    } else {
        qint64 diff = QDateTime::currentMSecsSinceEpoch() - m_dtEmailStatus;
        if (diff <= EMAIL_TIMEOUT) {
                QTimer::singleShot(REQUEST_DELAY, this, [=]() {
                qInfo() << "finishWaitEmailStatusWeb m_dtEmailStatus <= EMAIL_TIMEOUT";
                requestEmailStatus();
            });
        } else {
            if (++m_helloAttempts > REQUEST_ATTEMPTS) {
                // close notification
                if (m_popup != nullptr) {
                    delete m_popup;
                    m_popup = nullptr;
                }
                QSettings& set = *MainWindow::m_mainWindow->settings();
                set.beginGroup("Mainwindow");
                set.setValue("remind", true);
                set.endGroup();
                // show network timeout
                showModeless(tr("Register application"),tr("Network timeout"), tr("Ok"));
                emit networkTimeout();
            } else {
                qInfo() << "finishWaitEmailStatusWeb REQUEST_ATTEMPTS: " << m_helloAttempts;
                QTimer::singleShot(REQUEST_DELAY, this, [=]() {
                    qInfo() << "finishWaitEmailStatusWeb m_dtEmailStatus <= EMAIL_TIMEOUT";
                    requestEmailStatus();
                });
            }
        }
    }
}

void LicenseAgent::finishWaitInfoWeb()
{
    parseInfoWeb();
    if (infoWebIsEmpty()) {
        // manual
        ManualInfoWeb info;
        QSettings& set = *MainWindow::m_mainWindow->settings();
        set.beginGroup("MaiWindow");
        if (m_email.isEmpty()) {
            m_email = set.value("eMail", "").toString();
        }
        if (m_userName.isEmpty()) {
            m_userName = set.value("userName", "").toString();
        }
        set.endGroup();
        info.email = m_email;
        info.userName = m_userName;
        info.licenseName = MainWindow::m_mainWindow->analyzer()->getLicense();
        info.userName = m_userName;
        info.serialNumber = MainWindow::m_mainWindow->analyzer()->getSerialNumber();
        info.purchargeDate = m_infoWeb.purchargeDate.isEmpty() ? QDate::currentDate().toString("dd.MM.yyyy") : m_infoWeb.purchargeDate;
        UnitRequestDialog dlg(info);
        if (dlg.exec() == QDialog::Rejected) {
            emit canceled();
            return;
        }
        m_unitRequest.email = dlg.infoWeb().email;
        m_unitRequest.purchargeDate = dlg.infoWeb().purchargeDate;
        m_unitRequest.serialNumber = dlg.infoWeb().serialNumber;
        m_unitRequest.userName = dlg.infoWeb().userName;
        requestUnit();
    } else {
        QString info;
        info += tr("Device name: ") + m_infoWeb.deviceName + "\n";
        info += tr("Serial number: ") + m_infoWeb.serialNumber + "\n";
        info += tr("License name: ") + m_infoWeb.licenseName + "\n";
        info += tr("Purchrge date: ") + m_infoWeb.purchargeDate + "\n";
        info += tr("Login date: ") + m_infoWeb.loginDate + "\n";
        showModeless(tr("Register device"), info, "Ok");
        // TODO
        // enable button "Update user data"
        // ...
        return;
    }
}

void LicenseAgent::finishWaitUnitWeb()
{
    parseUnitWeb();
    if (isUnitWebValid()) {
        QString info;
        info += tr("Registration was successful\n");
        info += tr("Device name: ") + m_infoWeb.deviceName + "\n";
        info += tr("Serial number: ") + m_infoWeb.serialNumber + "\n";
        info += tr("License name: ") + m_infoWeb.licenseName + "\n";
        info += tr("Purchrge date: ") + m_infoWeb.purchargeDate + "\n";
        info += tr("Login date: ") + m_infoWeb.loginDate + "\n";
        showModeless(tr("Register device"), info, "Ok");//, tr("Update User Data"));
        connect(m_modelessPopup, &ModelessPopup::accepted, this, [=] () {
            ManualInfoWeb info;
            QSettings& set = *MainWindow::m_mainWindow->settings();
            set.beginGroup("MaiWindow");
            if (m_email.isEmpty()) {
                m_email = set.value("eMail", "").toString();
            }
            if (m_userName.isEmpty()) {
                m_userName = set.value("userName", "").toString();
            }
            set.endGroup();
            info.email = m_email;
            info.userName = m_userName;
            info.licenseName = MainWindow::m_mainWindow->analyzer()->getLicense();
            info.userName = m_userName;
            info.serialNumber = MainWindow::m_mainWindow->analyzer()->getSerialNumber();
            info.purchargeDate = m_infoWeb.purchargeDate.isEmpty() ? QDate::currentDate().toString("dd.MM.yyyy") : m_infoWeb.purchargeDate;
            UnitRequestDialog dlg(info);
            if (dlg.exec() == QDialog::Rejected) {
                emit canceled();
                return;
            }
            m_unitRequest.email = dlg.infoWeb().email;
            m_unitRequest.purchargeDate = dlg.infoWeb().purchargeDate;
            m_unitRequest.serialNumber = dlg.infoWeb().serialNumber;
            m_unitRequest.userName = dlg.infoWeb().userName;
            requestUnit();
        });
    } else {
        if ((QDateTime::currentSecsSinceEpoch() - m_dtUnit) <= TRANSFER_TIMEOUT) {
            m_unitTimer.setSingleShot(true);
            QEventLoop loop;
            connect(&m_unitTimer, SIGNAL(timeout()), &loop, SLOT(quit()));
            connect(m_reply, SIGNAL(finished()), &loop, SLOT(quit()));
            m_unitTimer.start(REQUEST_DELAY);
            loop.exec();

            if(m_unitTimer.isActive()) {
                requestUnit();
            }
        } else {
            if (++m_UnitAttempts <= REQUEST_ATTEMPTS) {
                showModeless(tr("Register device"), tr("Registration failed"), tr("Cancel"), tr("Try again"));
                connect(this, &LicenseAgent::accepted, this, [=](){
                    requestInfo();
                });
            } else {
                showModeless(tr("Register device"), tr("email is not verified"), tr("Ok"));
                emit canceled();
            }
        }
    }
}

void LicenseAgent::finishWaitLicense()
{
    parseLicense();
    if ( ! licenseKeyBan()) {
        showModeless(tr("Renew license"), tr("License renewal..."), tr("Ok"));
        sendMatch_11();
    } else {
        if (++m_licenseAttempts <= REQUEST_ATTEMPTS) {
            updateLicense();
        } else {
            sendBlocked();
            showModeless(tr("Renew license"), tr("Something went wrong.\nContact the support service"), tr("Ok"));
            emit canceled();
        }
    }
}

void LicenseAgent::finishWaitProfileB16()
{
    QString data = m_arr;
    QByteArray decoded = EncodingHelpers::decodeString(data);
    qInfo() << "LicenseAgent::finishWaitProfileB16" << decoded.toHex();
    sendProfile_B16(decoded);
}

void LicenseAgent::finishWaitInfoB16()
{
    parseInfo_B16();
    if ( ! info_B16Failed()) {
        QString reboot = "REBOOT\r";
        MainWindow::m_mainWindow->analyzer()->sendCommand(reboot);
        showModeless(tr("License renewal"), tr("The license update was successful"), tr("Ok"));
        emit canceled();
    } else {
        showModeless(tr("License renewal"), tr("The license is not updated"), tr("Close"));
        emit canceled();
    }
}

bool LicenseAgent::info_B16Failed()
{
    return m_infoB16.status != "1";
}

void LicenseAgent::parseInfo_B16()
{
    QString infoB16(m_arr);
    QString decoded = EncodingHelpers::decodeString_nRaw1(infoB16);
    QStringList list = decoded.split("&&&");
    for (int var = 0; var < list.size(); ++var) {
        QString field = list.at(var);
        QStringList lst = field.split('=');
        if (lst.size() != 2) {
            continue;
        }
        if (lst.at(0) == "dvName") {
            m_infoB16.deviceName = lst.at(1);
        } else  if (lst.at(0) == "dvSN") {
            m_infoB16.serialNumber = lst.at(1);
        }  else  if (lst.at(0) == "dtPur") {
            m_infoB16.purchargeDate = lst.at(1);
        } else  if (lst.at(0) == "dtLog") {
            m_infoB16.loginDate = lst.at(1);
        } else if (lst.at(0) == "Status") {
            m_infoB16.status = lst.at(1);
        } else if (lst.at(0) == "lcName") {
            m_infoB16.licenseName = lst.at(1);
        }
    }
}

bool LicenseAgent::licenseKeyBan()
{
    return m_licenseWeb.status == "0";
}

void LicenseAgent::sendBlocked()
{
    // TODO

}

void LicenseAgent::sendMatch_11()
{
    m_arr = EncodingHelpers::sendToMatch(m_licenseWeb.serialNumber);
    QByteArray arr = m_arr.right(m_arr.length() - 2);
    MainWindow::m_mainWindow->analyzer()->setParseState(WAIT_MATCH_12);
    MainWindow::m_mainWindow->analyzer()->sendData(arr);
}

QByteArray string2bytes(QString& str)
{
    QByteArray arr;
    for (int i=0; i<str.length()-1; i+=2) {

    }
    return arr;
}

void LicenseAgent::sendProfile_B16(QByteArray data)
{
    m_arr = data;
    QByteArray arr = m_arr.mid(2);
    MainWindow::m_mainWindow->analyzer()->setParseState(WAIT_MATCH_PROFILE_B16);
    MainWindow::m_mainWindow->analyzer()->sendData(arr);
}


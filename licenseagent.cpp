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
        if (state() != Finished && state() != Error)
            showModeless(QString(tr("Error")), str, "Cancel");
    });
    connect(&m_timer, &QTimer::timeout, this, [=](){
        m_timer.stop();
        timeout();
        if (state() != Finished && state() != Error)
            showModeless(QString(tr("Timeout")), tr("Network timeout"), "Cancel");
    });
    m_timer.setSingleShot(true);
}

LicenseAgent::~LicenseAgent()
{

}

void LicenseAgent::registerApllication(QString user, QString email)
{
    if (state() == WaitEmailStatusWeb) {
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
    request.setTransferTimeout(REPLY_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    setState(WaitEmailStatusWeb);
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
    qInfo() << "LicenseAgent::parseEmailStatus " << m_emailStatusWeb;
    //requestEmailStatus();
}

void LicenseAgent::registerDevice(QString device_name, QString serial, QString license)
{
    m_infoRequest.deviceName = device_name;
    m_infoRequest.serialNumber = serial;
    m_infoRequest.licenseName = license;
    setState(Finished);
    requestInfo();
}

void LicenseAgent::updateLicense()
{
    QString key = QInputDialog::getText((QWidget*)MainWindow::m_mainWindow, tr("Renew license"), tr("Enter key"));
    if (key.isNull())
        return;
    setState(Finished);
    requestLicense(key);
}

void LicenseAgent::requestLicense(QString key)
{
    QString name = MainWindow::m_mainWindow->analyzer()->getModelString();
    QString serial = MainWindow::m_mainWindow->analyzer()->getSerialNumber();
    QString license = MainWindow::m_mainWindow->analyzer()->getLicense();
    QString url = SERVER_NAME;
    QString strRaw = QString("dvName=%1&&&dvSN=%2&&&lcCode=%3&&&lcName=%4&&&").arg(name, serial, key, license);
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=4&nRaw=1&raw=%1").arg(strData);

    qInfo() << "   LicenseAgent::requestLicense" << url;

    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(REPLY_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    setState(WaitLicense);
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
#if 0
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
        qInfo() << "LicenseAgent::updateUserData   Rejected";
        emit canceled();
        return;
    }
    m_unitRequest.email = dlg.infoWeb().email;
    m_unitRequest.purchargeDate = dlg.infoWeb().purchargeDate;
    m_unitRequest.serialNumber = dlg.infoWeb().serialNumber;
    m_unitRequest.userName = dlg.infoWeb().userName;

    qInfo() << "LicenseAgent::updateUserData: ";
    requestUnit();

}
#endif
void LicenseAgent::updateUserData()
{
    setState(Finished);
    requestUserInfo();
}

void LicenseAgent::showModeless(QString title, QString text, QString buttonCancel, QString buttonOk)
{
    if (m_modelessPopup != nullptr) {
        qInfo() << "showModeless OLD" << m_modelessPopup->title() << m_modelessPopup->text();
        if ((m_modelessPopup->title() == title) &&(m_modelessPopup->text() == text))
            return;
        m_modelessPopup->disconnect();
        m_modelessPopup->setVisible(false);
        m_modelessPopup->close();
        m_modelessPopup->deleteLater();
    }
    m_modelessPopup = new ModelessPopup(title, text, buttonCancel, buttonOk, MainWindow::m_mainWindow);
    connect(m_modelessPopup, &ModelessPopup::rejected, this, [=](){
        setState(Finished);
        m_canceled = true;
        emit canceled();
    });
    connect(m_modelessPopup, &ModelessPopup::accepted, this, [=](){
        emit accepted();
    });
    m_modelessPopup->setModal(false);
    m_modelessPopup->show();
    m_modelessPopup->activateWindow();
    m_modelessPopup->setFocus();
    qInfo() << "showModeless NEW" << m_modelessPopup->title() << m_modelessPopup->text() << state();
}

void LicenseAgent::closeModeless()
{
    if (m_modelessPopup != nullptr) {
        qInfo() << "showModeless CLOSE" << m_modelessPopup->title() << m_modelessPopup->text();
        m_modelessPopup->disconnect();
        m_modelessPopup->setVisible(false);
        m_modelessPopup->close();
        m_modelessPopup->deleteLater();
        m_modelessPopup = nullptr;
        setState(Finished);
    }

}

void LicenseAgent::requestUserInfo()
{
    setState(Finished);
    requestInfo();
    setState(WaitUserInfoWeb);
}

void LicenseAgent::requestInfo()
{
    QString url = SERVER_NAME;
//    QString strRaw = QString("dvName=%1&&&dvSN=%2&&&lcName=%3&&&")
//                         .arg(m_infoRequest.deviceName, m_infoRequest.serialNumber,
//                              m_infoRequest.licenseName);
    AnalyzerPro& analyzer = *MainWindow::m_mainWindow->analyzer();
    QString strRaw = QString("dvName=%1&&&dvSN=%2&&&lcName=%3&&&")
                         .arg(analyzer.getModelString(), analyzer.getSerialNumber(),
                              analyzer.getLicense());

    qInfo() << "LicenseAgent::requestInfo() strRaw" << strRaw;
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=2&nRaw=1&raw=%1").arg(strData);
    qInfo() << "LicenseAgent::requestInfo() url" << url;

    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(REPLY_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    setState(WaitInfoWeb);
    //showModeless(tr("Register device"),tr("Registration..."), tr("Cancel"));
    m_dtUnit = QDateTime::currentMSecsSinceEpoch();
    m_UnitAttempts = 0;
    m_mng.get(request);
}

void LicenseAgent::parseInfoWeb()
{
    QString infoWeb(m_arr);
    int pos = infoWeb.indexOf("&nRez=");
    int pos1 = infoWeb.indexOf("&", pos+1);
    QString nrez = infoWeb.mid(pos+6, pos1-(pos+6));
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
    //return m_infoWeb.serialNumber.isEmpty();
    return m_infoWeb.nRez != 1 ;
}

void LicenseAgent::requestUnit()
{
    QString url = SERVER_NAME;
    QString strRaw = QString("dvSN=%1&&&Eml=%2&&&Name=%3&&&dtPur=%4&&&dvName=%5&&&")
                         .arg(m_unitRequest.serialNumber, m_unitRequest.email,
                              m_unitRequest.userName, m_unitRequest.purchargeDate,
                              MainWindow::m_mainWindow->analyzer()->getModelString());
    qInfo() << "LicenseAgent::requestUnit" << strRaw;
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=3&nRaw=1&raw=%1").arg(strData);

    qInfo() << "LicenseAgent::requestUnit" << url;
    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(REPLY_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    setState(WaitUnitWeb);
    m_mng.get(request);
}


void LicenseAgent::parseUnitWeb()
{
    QString unitWeb(m_arr);
    qInfo() << "LicenseAgent::parseUnitWeb" << unitWeb;
    int pos = unitWeb.indexOf("&nRez=");
    int pos1 = unitWeb.indexOf("&", pos+1);
    QString nrez = unitWeb.mid(pos+6, pos1-(pos+6));
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
            m_unitWeb.emailStatus = lst.at(1) == "1";
        } else  if (lst.at(0) == "Name") {
            m_unitWeb.userName = lst.at(1);
        }  else  if (lst.at(0) == "dtPur") {
            m_unitWeb.purchargeDate = lst.at(1);
        } else  if (lst.at(0) == "dtLog") {
            m_unitWeb.loginDate = lst.at(1);
        }
    }
}

bool LicenseAgent::needWaitForEmail()
{
    return (m_unitWeb.nRez == 2);
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
    request.setTransferTimeout(REPLY_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    setState(WaitProfileB16);
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
    request.setTransferTimeout(REPLY_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    setState(WaitInfoB16);
    m_mng.get(request);
}

void LicenseAgent::onReplyFinished(QNetworkReply* reply)
{
    qInfo() << "LicenseAgent::onReplyFinished: " << reply->error() << reply->errorString();
    m_arr = reply->readAll();
    //closeModeless();

    if (reply->error() != QNetworkReply::NoError) {
        setState(Error);
        showModeless(tr("Network error."), reply->errorString(), tr("Close"));
        return;
    }
    if (m_canceled) {
        m_canceled = false;
        setState(Finished);
    }
    qInfo() << "    LicenseAgent::onReplyFinished: state=" << m_state << ", " << QString(m_arr);
    if (state() == WaitEmailStatusWeb) {
        finishWaitEmailStatusWeb();
    } else if (state() == WaitInfoWeb) {
        finishWaitInfoWeb();
    } else if (state() == WaitUserInfoWeb) {
        finishWaitUserInfoWeb();
    } else if(state() == WaitUnitWeb) {
        finishWaitUnitWeb();
    } else if(state() == WaitLicense) {
        finishWaitLicense();
    } else if (state() == WaitInfoB16) {
        finishWaitInfoB16();
    } else if (state() == WaitProfileB16) {
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

void LicenseAgent::finishWaitUserInfoWeb()
{
    qInfo() << "LicenseAgent::finishWaitUserInfoWeb";
    parseInfoWeb();
    bool empty = infoWebIsEmpty();
    if (empty) {
        // local data
        QString text;
        text += tr("Device name ") + MainWindow::m_mainWindow->analyzer()->getModelString();
        text += tr("\nSerial number ") + MainWindow::m_mainWindow->analyzer()->getSerialNumber();
        text += tr("\nLicense name ") + MainWindow::m_mainWindow->analyzer()->getLicense();
        showModeless(tr("Device info"), text, tr("Cancel"), tr("User Data update"));
        connect(m_modelessPopup, &ModelessPopup::accepted, this, [=](){
            closeModeless();
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
            closeModeless();
            UnitRequestDialog dlg(info);
            if (dlg.exec() == QDialog::Rejected) {
                emit canceled();
                return;
            }
            m_unitRequest.email = dlg.infoWeb().email;
            m_unitRequest.purchargeDate = dlg.infoWeb().purchargeDate;
            m_unitRequest.serialNumber = dlg.infoWeb().serialNumber;
            m_unitRequest.userName = dlg.infoWeb().userName;
            m_unitRequest.deviceName = MainWindow::m_mainWindow->analyzer()->getModelString();
            requestUnit();
            showModeless(tr("Register device"),tr("Registration..."), tr("Cancel"));
        });
    } else {
        // remote data
        QString text;
        text += tr("Device name ") + MainWindow::m_mainWindow->analyzer()->getModelString();
        text += tr("\nSerial number ") + MainWindow::m_mainWindow->analyzer()->getSerialNumber();
        text += tr("\nLicense name ") + MainWindow::m_mainWindow->analyzer()->getLicense();
        text += tr("\nPurchrge date: ") + m_infoWeb.purchargeDate;
        text += tr("\nRegistration date: ") + m_infoWeb.loginDate;
        showModeless(tr("Update user data"), text, tr("Cancel"), tr("User Data update"));
        connect(m_modelessPopup, &ModelessPopup::accepted, this, [=](){
            closeModeless();
            ManualInfoWeb manualInfo;
            QSettings& set = *MainWindow::m_mainWindow->settings();
            set.beginGroup("MaiWindow");
            if (m_email.isEmpty()) {
                m_email = set.value("eMail", "").toString();
            }
            if (m_userName.isEmpty()) {
                m_userName = set.value("userName", "").toString();
            }
            set.endGroup();
            manualInfo.email = m_email;
            manualInfo.userName = m_userName;
            manualInfo.licenseName = MainWindow::m_mainWindow->analyzer()->getLicense();
            manualInfo.userName = m_userName;
            manualInfo.serialNumber = MainWindow::m_mainWindow->analyzer()->getSerialNumber();
            manualInfo.purchargeDate = m_infoWeb.purchargeDate.isEmpty() ? QDate::currentDate().toString("dd.MM.yyyy") : m_infoWeb.purchargeDate;
            closeModeless();
            UnitRequestDialog dlg(manualInfo);
            if (dlg.exec() == QDialog::Rejected) {
                emit canceled();
                return;
            }
            m_unitRequest.email = dlg.infoWeb().email;
            m_unitRequest.purchargeDate = dlg.infoWeb().purchargeDate;
            m_unitRequest.serialNumber = dlg.infoWeb().serialNumber;
            m_unitRequest.userName = dlg.infoWeb().userName;
            m_unitRequest.deviceName = MainWindow::m_mainWindow->analyzer()->getModelString();
            m_userName = m_unitRequest.userName;
            m_email = m_unitRequest.email;
            requestUnit();
            showModeless(tr("Register device"),tr("Registration..."), tr("Cancel"));
        });
    }
}


void LicenseAgent::finishWaitInfoWeb()
{
    qInfo() << "LicenseAgent::finishWaitInfoWeb";
    parseInfoWeb();
    if (m_infoWeb.nRez != 1) {
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
        closeModeless();
        UnitRequestDialog dlg(info);
        if (dlg.exec() == QDialog::Rejected) {
            emit canceled();
            return;
        }
        m_unitRequest.email = dlg.infoWeb().email;
        m_unitRequest.purchargeDate = dlg.infoWeb().purchargeDate;
        m_unitRequest.serialNumber = dlg.infoWeb().serialNumber;
        m_unitRequest.userName = dlg.infoWeb().userName;
        m_unitRequest.deviceName = MainWindow::m_mainWindow->analyzer()->getModelString();
        m_userName = m_unitRequest.userName;
        m_email = m_unitRequest.email;
        requestUnit();
        showModeless(tr("Register device"),tr("Registration..."), tr("Cancel"));
    } else {
        QString info;
        info += tr("Device name: ") + m_infoWeb.deviceName + "\n";
        info += tr("Serial number: ") + m_infoWeb.serialNumber + "\n";
        info += tr("License name: ") + m_infoWeb.licenseName + "\n";
        info += tr("Purcharge date: ") + m_infoWeb.purchargeDate + "\n";
        info += tr("Registration date: ") + m_infoWeb.loginDate + "\n";
        showModeless(tr("Register device"), info, "Ok");
        return;
    }
}

void LicenseAgent::finishWaitUnitWeb()
{
    qInfo() << "        ******* m_dtUnit = " << m_dtUnit << ", m_UnitAttempts = " << m_UnitAttempts;
    parseUnitWeb();
    if (m_unitWeb.nRez == 2) {
        qInfo() << "LicenseAgent::finishWaitUnitWeb  VALID m_unitWeb.nRez == 2";
        QString info;
        info += tr("Registration was successful\n");
        info += tr("Device name: ") + m_unitWeb.deviceName + "\n";
        info += tr("Serial number: ") + m_unitWeb.serialNumber + "\n";
        QString license =(m_infoWeb.licenseName.isEmpty()
            ? MainWindow::m_mainWindow->analyzer()->getLicense()
            : m_infoWeb.licenseName);
        info += tr("License name: ") + license + "\n";
        info += tr("Purcharge date: ") + m_unitWeb.purchargeDate + "\n";
        info += tr("Registration date: ") + m_unitWeb.loginDate + "\n";
        showModeless(tr("Register device"), info, "Ok");
    } else if (!m_unitWeb.emailStatus) {
        if ((QDateTime::currentMSecsSinceEpoch() - m_dtUnit) <= TRANSFER_TIMEOUT) {
            qInfo() << "LicenseAgent::finishWaitUnitWeb  <= TRANSFER_TIMEOUT";
            m_unitTimer.setSingleShot(true);
            QEventLoop loop;
            connect(&m_unitTimer, SIGNAL(timeout()), &loop, SLOT(quit()));
            connect(m_reply, SIGNAL(finished()), &loop, SLOT(quit()));
            m_unitTimer.start(REQUEST_DELAY);
            loop.exec();

            //if(m_unitTimer.isActive()) {
                requestUnit();
                showModeless(tr("Register device"), tr("Wait for eMail confirmation..."), tr("Cancel"));
            //}
        } else {
            qInfo() << "LicenseAgent::finishWaitUnitWeb  >  TRANSFER_TIMEOUT";
            if (++m_UnitAttempts <= REQUEST_ATTEMPTS) {
                qInfo() << "LicenseAgent::finishWaitUnitWeb ++m_UnitAttempts <= REQUEST_ATTEMPTS" << m_UnitAttempts;
                showModeless(tr("Register device"), tr("Registration failed"), tr("Cancel"), tr("Try again"));
                connect(m_modelessPopup, &ModelessPopup::accepted, this, [=](){
                    //closeModeless();
                    ManualInfoWeb manualInfo;
                    QSettings& set = *MainWindow::m_mainWindow->settings();
                    set.beginGroup("MaiWindow");
                    if (m_email.isEmpty()) {
                        m_email = set.value("eMail", "").toString();
                    }
                    if (m_userName.isEmpty()) {
                        m_userName = set.value("userName", "").toString();
                    }
                    set.endGroup();
                    manualInfo.email = m_email;
                    manualInfo.userName = m_userName;
                    manualInfo.licenseName = MainWindow::m_mainWindow->analyzer()->getLicense();
                    manualInfo.userName = m_userName;
                    manualInfo.serialNumber = MainWindow::m_mainWindow->analyzer()->getSerialNumber();
                    manualInfo.purchargeDate = m_infoWeb.purchargeDate.isEmpty() ? QDate::currentDate().toString("dd.MM.yyyy") : m_infoWeb.purchargeDate;
                    closeModeless();
                    UnitRequestDialog dlg(manualInfo);
                    if (dlg.exec() == QDialog::Rejected) {
                        emit canceled();
                        return;
                    }
                    m_unitRequest.email = dlg.infoWeb().email;
                    m_unitRequest.purchargeDate = dlg.infoWeb().purchargeDate;
                    m_unitRequest.serialNumber = dlg.infoWeb().serialNumber;
                    m_unitRequest.userName = dlg.infoWeb().userName;
                    m_unitRequest.deviceName = MainWindow::m_mainWindow->analyzer()->getModelString();
                    m_userName = m_unitRequest.userName;
                    m_email = m_unitRequest.email;

                    m_dtUnit = QDateTime::currentMSecsSinceEpoch();
                    m_UnitAttempts = 0;
                    requestUnit();
                    showModeless(tr("Register device"),tr("Registration..."), tr("Cancel"));
                });
            } else {
                qInfo() << "LicenseAgent::++m_UnitAttempts > REQUEST_ATTEMPTS" << m_UnitAttempts;
                showModeless(tr("Register device"), tr("eMail is not confirmed"), tr("Ok"));
            }
        }
    } else {
        setState(Error);
        qInfo() << "LicenseAgent::finishWaitUnitWeb Registration failed";
        showModeless(tr("Register device"), tr("Registration failed"), tr("Cancel"));
        connect(this, &LicenseAgent::canceled, this, [=](){
            closeModeless();
        });
    }
}

void LicenseAgent::finishWaitLicense()
{
    parseLicense();
    if ( ! licenseKeyBan()) {
        showModeless(tr("Renew license"), tr("License renewal..."), tr("Ok"));
        sendMatch_11();
    } else {
        if (++m_licenseAttempts <= REQUEST_LICENSE_ATTEMPTS) {
            updateLicense();
        } else {
            showModeless(tr("Renew license"), tr("The update will not work until support is contacted"), tr("Ok"));
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
        MainWindow::m_mainWindow->analyzer()->on_disconnectDevice();
        showModeless(tr("License renewal"), tr("The license update was successful\n\nYou should reconnect analyzer."), tr("Ok"));
        emit registered();
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
    if (m_licenseWeb.status == "2") {
        m_licenseAttempts += 20;
        emit updateBlocked();
    }
    return m_licenseWeb.status != "1";
}

void LicenseAgent::sendBlocked()
{
    // obsolete
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


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
        qDebug() << "***       QNetworkReply::errorOccurred" << error;
    });
    connect(&m_timer, &QTimer::timeout, this, [=](){
        timeout();
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
    showModeless(tr("Waiting for email confirmation..."), tr("Cancel"));
    requestEmailStatus();
}

void LicenseAgent::requestEmailStatus()
{
    QString url = SERVER_NAME;
    QString strRaw = QString("Name=%1&&&Eml=%2&&&").arg(m_userName, m_email);
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=1&nRaw=1&raw=%1").arg(strData);

    qDebug() << "   requestEmailStatus: " << url;

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
    qDebug() << "   parseEmailStatus:" << status;
    QString rez = EncodingHelpers::decodeString(status);
    qDebug() << "   parseEmailStatus decoded:" << rez;
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
    requestEmailStatus();
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
    QString key = QInputDialog::getText(MainWindow::m_mainWindow, tr("Renew license"), tr("Enter key"));
    if (key.isNull())
        return;
    requestLicense(key);

}

void LicenseAgent::requestLicense(QString key)
{
    // TODO
    // send special command instead of HELLO
    QString name = MainWindow::m_mainWindow->analyzer()->getModelString();
    QString serial = MainWindow::m_mainWindow->analyzer()->getSerialNumber();
    QString url = SERVER_NAME;
    QString strRaw = QString("dvName=%1&&&dvSN=%2&&&lcCode=%3&&&").arg(name, serial, key);
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=4&nRaw=1&raw=%1").arg(strData);

    qDebug() << "   requestLicense: " << url;

    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(TRANSFER_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    m_state = WaitLicense;
    m_mng.get(request);

}

void LicenseAgent::parseLicense()
{
    QString licenseWeb(m_arr);
    qDebug() << "   parseLicense:" << licenseWeb;
    QString rez = EncodingHelpers::decodeString(licenseWeb);
    qDebug() << "   parseLicense decoded:" << rez;
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

void LicenseAgent::showModeless(QString text, QString buttonCancel, QString buttonOk)
{
    if (m_modelessPopup != nullptr) {
        m_modelessPopup->disconnect();
        m_modelessPopup->setVisible(false);
        m_modelessPopup->deleteLater();
    }
    m_modelessPopup = new ModelessPopup(text, buttonCancel, buttonOk, MainWindow::m_mainWindow);
    connect(m_modelessPopup, &ModelessPopup::rejected, this, [=](){
        emit canceled();
    });
    connect(m_modelessPopup, &ModelessPopup::accepted, this, [=](){
        emit accepted();
    });
    m_modelessPopup->setModal(false);
    m_modelessPopup->show();
    m_modelessPopup->setFocus();
}

void LicenseAgent::requestInfo()
{
    QString url = SERVER_NAME;
    QString strRaw = QString("dvName=%1&&&dvSN=%2&&&lcName=%3&&&")
                         .arg(m_infoRequest.deviceName, m_infoRequest.serialNumber, m_infoRequest.licenseName);
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=2&nRaw=1&raw=%1").arg(strData);

    qDebug() << "   requestInfo: " << url;

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
    qDebug() << "   parseInfoWeb:" << infoWeb;
    QString rez = EncodingHelpers::decodeString(infoWeb);
    qDebug() << "   parseInfoWeb decoded:" << rez;
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
    return m_infoWeb.licenseName.isEmpty();
}

void LicenseAgent::requestUnit()
{
    QString url = SERVER_NAME;
    QString strRaw = QString("dvSN=%1&&&Eml=%2&&&Name=%3&&&dtPur=%4&&&")
                         .arg(m_unitRequest.serialNumber, m_unitRequest.email, m_unitRequest.userName, m_unitRequest.purchargeDate);
    QString strData = EncodingHelpers::encodeString(strRaw);
    url += QString("?nGet=3&nRaw=1&raw=%1").arg(strData);

    qDebug() << "   requestUnit: " << strRaw;
    qDebug() << "   requestUnit: " << url;

    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(TRANSFER_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    m_state = WaitUnitWeb;
    m_mng.get(request);
    showModeless(tr("Registration..."), tr("Cancel"));
}


void LicenseAgent::parseUnitWeb()
{
    QString unitWeb(m_arr);
    qDebug() << "   parseUnitWeb:" << unitWeb;
    QString rez = EncodingHelpers::decodeString(unitWeb);
    qDebug() << "   parseUnitWeb decoded:" << rez;
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
    return m_unitWeb.emailStatus;
}

void LicenseAgent::requestStatus_B16(QByteArray data)
{

    QString dataStr(data);
    qDebug() << "requestStatus_B16:    " << data.toHex();
    if (dataStr.contains("Error")) {
        if (m_modelessPopup != nullptr)
            m_modelessPopup->close();
        QMessageBox::warning(MainWindow::m_mainWindow, tr("Update license"), tr("Something went wrong"));
        return;
    }
    QString url = SERVER_NAME;
    QString strData(data.toHex());
    url += QString("?nGet=21&nRaw=21&raw=073E%1").arg(strData.toUpper());
    url += "85";
    qDebug() << "requestStatus_B16" << url;

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

    qDebug() << "requestInfo_B16:   " << url;

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
    m_arr = reply->readAll();
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
    QString str(m_arr);
    parseEmailStatus();
    //qDebug() << str;
    if (m_emailStatusWeb) {
        // close notification
        if (m_popup != nullptr) {
            delete m_popup;
            m_popup = nullptr;
        }
        showModeless(tr("Registration was successful"), tr("Ok"));

        QSettings set;
        set.setValue("email", m_email);
        emit registered();
    } else {
        if (m_dtEmailStatus <= EMAIL_TIMEOUT) {
            QTimer::singleShot(REQUEST_DELAY, this, [=]() {
                requestEmailStatus();
            });
        } else {
            if (++m_helloAttempts > REQUEST_ATTEMPTS) {
                // close notification
                if (m_popup != nullptr) {
                    delete m_popup;
                    m_popup = nullptr;
                }
                // show network timeout
                showModeless(tr("Network timeout"), tr("Ok"));
                emit networkTimeout();
            } else {
                requestEmailStatus();
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
        showModeless(info, "Ok");
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
        QMessageBox::information(MainWindow::m_mainWindow, tr("Registration"), tr("Registration was successful"));
        QString info;
        info += tr("Device name: ") + m_infoWeb.deviceName + "\n";
        info += tr("Serial number: ") + m_infoWeb.serialNumber + "\n";
        info += tr("License name: ") + m_infoWeb.licenseName + "\n";
        info += tr("Purchrge date: ") + m_infoWeb.purchargeDate + "\n";
        info += tr("Login date: ") + m_infoWeb.loginDate + "\n";
        showModeless(info, "Ok");
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
                showModeless(tr("Registration failed"), tr("Cancel"), tr("Try again"));
                connect(this, &LicenseAgent::accepted, this, [=](){
                    requestInfo();
                });
            } else {
                showModeless(tr("email is not verified"), tr("Ok"));
                emit canceled();
            }
        }
    }
}

void LicenseAgent::finishWaitLicense()
{
    parseLicense();
    if ( ! licenseKeyBan()) {
        showModeless(tr("License renewal..."), tr("Ok"));
        sendMatch_11();
    } else {
        if (++m_licenseAttempts <= REQUEST_ATTEMPTS) {
            updateLicense();
        } else {
            sendBlocked();
            showModeless(tr("Contact the support service"), tr("Ok"));
            emit canceled();
        }
    }
}

void LicenseAgent::finishWaitProfileB16()
{
    QString data = m_arr;
    int pos = data.indexOf("&raw=");
    //data = data.mid(pos+5);
    QString decoded = EncodingHelpers::decodeString(data);
    sendProfile_B16(decoded.toLatin1());
}

void LicenseAgent::finishWaitInfoB16()
{
    parseInfo_B16();
    if ( ! info_B16Failed()) {
        showModeless(tr("The license update was successful"), tr("Ok"));
        emit canceled();
    } else {
        showModeless(tr("The license is not updated"), tr("Close"));
        emit canceled();
    }
}

bool LicenseAgent::info_B16Failed()
{
    return m_infoB16.status == "1";
}

void LicenseAgent::parseInfo_B16()
{
    QString infoB16(m_arr);
    qDebug() << "   parseInfo_B16:" << infoB16;
    QString decoded = EncodingHelpers::decodeString(infoB16);
    qDebug() << "   parseInfo_B16 decoded:" << decoded;
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
    qDebug() << "sendMatch_11: " << m_arr.toHex().data();
    QByteArray arr = m_arr.right(m_arr.length() - 2);
    MainWindow::m_mainWindow->analyzer()->setParseState(WAIT_MATCH_12);
    MainWindow::m_mainWindow->analyzer()->sendData(arr);
}

void LicenseAgent::sendProfile_B16(QByteArray data)
{
    m_arr = EncodingHelpers::sendToMatch(data.toHex());
    qDebug() << "sendProfile_B16:   " << m_arr;
    QByteArray arr = m_arr.right(m_arr.length() - 2);
    qDebug() << "" << arr.toHex();
    MainWindow::m_mainWindow->analyzer()->setParseState(WAIT_MATCH_PROFILE_B16);
    MainWindow::m_mainWindow->analyzer()->sendData(arr);
}


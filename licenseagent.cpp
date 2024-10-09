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

    QString url = SERVER_NAME;
    QString strRaw = QString("Name=%1&&&Eml=%2&&&").arg(user, email);
    QString strData = EncodingHelpers::A02_EnCode_strToBase16_v0(strRaw);
    url += QString("?nGet=1&nRaw=1&raw=%1").arg(strData);

    qDebug() << "   registerApllication: " << url;

    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(TRANSFER_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    showModeless(tr("Waiting for email confirmation..."), tr("Cancel"));

    m_state = WaitEmailStatusWeb;

    m_dtEmailStatus = QDateTime::currentSecsSinceEpoch();
    m_reply = m_mng.get(request);

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(m_reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(TRANSFER_TIMEOUT);
    loop.exec();

    if(timer.isActive()) {
        timer.stop();
        if(m_reply->error() > 0) {
            // handle error
            qDebug() << "    LicenseAgent::registerApllication error" << m_reply->error();
            m_modelessPopup->close();
            if (m_popup != nullptr)
                delete m_popup;
            m_popup = new PopUp();
            m_popup->setPopupText(m_reply->errorString());
            m_popup->show();
            emit networkTimeout();
        }
        else {
            int v = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (v >= 200 && v < 300) {  // Success
                qDebug() << "    LicenseAgent::registerApllication attribute" << v;
            }
        }
    } else {
        // timeout
        qDebug() << "    LicenseAgent::registerApllication timeout";
        disconnect(m_reply, SIGNAL(finished()), &loop, SLOT(quit()));

        m_reply->abort();
    }

    requestEmailStatus();
}

void LicenseAgent::requestEmailStatus()
{
    QString url = SERVER_NAME;
    QString strRaw = QString("Name=%1&&&Eml=%2&&&").arg(m_userName, m_email);
    QString strData = EncodingHelpers::A02_EnCode_strToBase16_v0(strRaw);
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
    QString reply(m_arr);
    QStringList list = reply.split("&");
    for (int i = 0; i <list.size(); ++i) {
        QString arg = list.at(i);
        if (arg.contains("nRez")) {
            QStringList lst = arg.split("=");
            m_emailStatusWeb = lst.at(1) == "1";
            m_state = Finished;
            return;
        }
    }
    requestEmailStatus();
}

void LicenseAgent::registerDevice(QString device_name, QString serial, QString license)
{
    m_infoRequest.deviceName = device_name;
    m_infoRequest.serialNumber = serial;
    m_infoRequest.licenseName = license;
    requestInfo();
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
    url += INFO;
    QNetworkRequest request((QUrl)url);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    m_mng.get(request);

    m_state = WaitInfoWeb;
}

void LicenseAgent::parseInfoWeb()
{
    // TODO
}

bool LicenseAgent::infoWebIsEmpty()
{
    return m_infoWeb.licenseName.isEmpty();
}

void LicenseAgent::requestUnit()
{
    QString url = SERVER_NAME;
    url += UNIT;
    // TODO url += m_unitRequest

    QNetworkRequest request((QUrl)url);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);
    m_dtUnit = QDateTime::currentSecsSinceEpoch();

    m_mng.get(request);

    m_state = WaitUnitWeb;
    showModeless(tr("Registration..."), tr("Cancel"));
}

void LicenseAgent::parseUnitWeb()
{
    //TODO
}

bool LicenseAgent::isUnitWebValid()
{
    // TODO
    return true;
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
        showModeless(tr("Registration is successful"), tr("Ok"));

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
        if (dlg.exec() == QDialogButtonBox::Cancel) {
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
        m_modelessPopup->close();
        showModeless(tr("Registration was successful"), tr("Ok"));
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



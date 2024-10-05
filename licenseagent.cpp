#include "licenseagent.h"
#include "popup.h"
#include "modelesspopup.h"
#include "mainwindow.h"

LicenseAgent::LicenseAgent(QObject *parent) :
    QObject(parent),
    m_state(Finished),
    m_reply(NULL)
{
    connect(&m_mng, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply){
        m_arr = reply->readAll();
        // TODO
        // parse m_emailStatusWeb
        if (m_emailStatusWeb) {
            // close notification
            if (m_popup != nullptr)
                delete m_popup;
            // show success
            m_popup = new PopUp();
            m_popup->setPopupText(tr("Registration is successful"));
            QSettings set;
            set.setValue("email", m_email);
            emit registered();
        } else {
            if (m_dtEmailStatus <= EMAIL_TIMEOUT) {
                QTimer::singleShot(REQUEST_DELAY, this, [=]() {
                    requestEmailStatus();
                });
            } else {
                if (++m_helloAttempts > 5) {
                    // close notification
                    if (m_popup != nullptr) {
                        delete m_popup;
                        m_popup = nullptr;
                    }
                    // show network timeout
                    m_popup = new PopUp();
                    m_popup->setPopupText(tr("Network timeout"));
                    m_popup->show();
                    emit networkTimeout();
                } else {
                    requestEmailStatus();
                }
            }
        }
    });
    connect(&m_timer, &QTimer::timeout, this, [=](){
        timeout();
    });
    m_timer.setSingleShot(true);
}

LicenseAgent::~LicenseAgent()
{

}

LicenseAgent::State LicenseAgent::registerApllication(QString user, QString email)
{
    if (m_state == InProgress) {
        return m_state;
    }

    m_userName = user;
    m_email = email;

//    QString url = "https://rigexpert.com/";
    QString url = "https://rigexpert.com.ua/";
    url += HELLO;
    url += QString("?name=%1&email=%2").arg(user, email);
    QNetworkRequest request((QUrl)url);
    request.setTransferTimeout(TRANSFER_TIMEOUT);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    //QMessageBox::information(0, tr("Register application"), tr("Waiting for email confirmation..."));
//    if (m_popup != nullptr)
//        delete m_popup;
//    m_popup = new PopUp(tr("Cancel"));
//    connect(m_popup, &PopUp::canceled, this, [=](){
//        emit canceled();
//    });
//    m_popup->setPopupText("Waiting for email confirmation...");
//    m_popup->setDurability(TRANSFER_TIMEOUT);
//    m_popup->show();
    showModeless(tr("Waiting for email confirmation..."), tr("Cancel"));

    m_dtEmailStatus = QDateTime::currentSecsSinceEpoch();
    m_reply = m_mng.get(request);

    //////////
    QTimer timer;
    timer.setSingleShot(TRANSFER_TIMEOUT);
    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(m_reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(30000);   // 30 secs. timeout
    loop.exec();

    if(timer.isActive()) {
        timer.stop();
        if(m_reply->error() > 0) {
            // handle error
            qDebug() << "    LicenseAgent::registerApllication error" << m_reply->error();
//            m_popup->hide();
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
    /////////

    //ReplyTimeout::set(m_reply, TRANSFER_TIMEOUT);
    //qCDebug(DOWNLOADER) << "start download info " << url;

    //delete popup;

    m_state = InProgress;
    return requestEmailStatus();
}

LicenseAgent::State LicenseAgent::requestEmailStatus()
{
    QString url = "https://rigexpert.com/";
    url += EMAIL_STATUS;
    QNetworkRequest request((QUrl)url);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);

    m_mng.get(request);

    //qCDebug(DOWNLOADER) << "start download info " << url;

    m_state = InProgress;
    return Started;
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

void LicenseAgent::showModeless(QString text, QString button)
{
    if (m_modelessPopup != nullptr) {
        m_modelessPopup->disconnect();
        m_modelessPopup->setVisible(false);
        m_modelessPopup->deleteLater();
    }
    m_modelessPopup = new ModelessPopup(text, button, MainWindow::m_mainWindow);
    connect(m_modelessPopup, &ModelessPopup::rejected, this, [=](){
        emit canceled();
    });
    m_modelessPopup->setModal(false);
    m_modelessPopup->show();
    m_modelessPopup->setFocus();
}

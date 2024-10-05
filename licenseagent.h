#ifndef LICENSEAGENT_H
#define LICENSEAGENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include "popup.h"
#include "modelesspopup.h"

#define HELLO "Hello"
#define REG_DEV "regdev"
#define EMAIL_STATUS "email_status"
#define EMAIL_TIMEOUT 30*1000
#define REQUEST_DELAY 5*1000
#define HELLO_ATTEMPTS 5
#define TRANSFER_TIMEOUT 30*1000

class LicenseAgent : public QObject
{
    Q_OBJECT
public:
    explicit LicenseAgent(QObject *parent = 0);
    ~LicenseAgent();

    enum State {Started, InProgress, Finished, Error};

    State registerApllication(QString user, QString email);
    void reset();
    void timeout();
    State requestEmailStatus();
    void showModeless(QString text, QString button);

signals:
    void registered();
    void networkTimeout();
    void canceled();

protected:
    State m_state;
    QNetworkAccessManager m_mng;
    QByteArray  m_arr;
    QNetworkReply *m_reply;
    QTimer m_timer;
    qint64 m_dtEmailStatus;
    bool m_emailStatusWeb = false;
    QString m_userName;
    QString m_email;
    int m_helloAttempts = 0;
    PopUp* m_popup = nullptr;
    ModelessPopup* m_modelessPopup = nullptr;
};

#endif // LICENSEAGENT_H

#ifndef LICENSEAGENT_H
#define LICENSEAGENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include "popup.h"
#include "modelesspopup.h"

#define SERVER_NAME "https://regdev.rigexpert.com/index.php"

#define HELLO "nGet=1"
#define INFO "info"
#define UNIT "unit"
#define EMAIL_STATUS "email_status"

#define EMAIL_TIMEOUT 30*1000
#define REQUEST_DELAY 5*1000
#define REQUEST_ATTEMPTS 5
#define TRANSFER_TIMEOUT 30*1000


struct InfoRequest
{
    InfoRequest () {}
    QString deviceName;
    QString serialNumber;
    QString licenseName;
};

struct InfoWeb
{
    InfoWeb() {}
    QString deviceName;
    QString serialNumber;
    QString licenseName;
    QString purchargeDate;
    QString loginDate;
};

struct UnitRequest
{
    UnitRequest() {}

    QString serialNumber;
    QString email;
    QString userName;
    QString purchargeDate;
};

struct UnitWeb
{
    UnitWeb() {}

    QString deviceName;
    QString serialNumber;
    QString email;
    bool emailStatus;
    QString userName;
    QString purchargeDate;
    QString loginDate;
};

struct ManualInfoWeb
{
    ManualInfoWeb() {}

    QString serialNumber;
    QString email;
    QString userName;
    QString purchargeDate;
    QString licenseName;
};

class LicenseAgent : public QObject
{
    Q_OBJECT
public:
    explicit LicenseAgent(QObject *parent = 0);
    ~LicenseAgent();

    enum State {WaitEmailStatusWeb, WaitInfoWeb, WaitUnitWeb, Finished, Error};

    void registerApllication(QString user, QString email);
    void registerDevice(QString device_name, QString serial, QString license);
    void reset();
    void timeout();
    void requestEmailStatus();
    void parseEmailStatus();

    void requestInfo();
    void parseInfoWeb();
    bool infoWebIsEmpty();

    void requestUnit();
    void parseUnitWeb();
    bool isUnitWebValid();

    void showModeless(QString text, QString buttonCancel, QString buttonOk=QString());

signals:
    void registered();
    void networkTimeout();
    void canceled();
    void accepted();

protected slots:
    void onReplyFinished(QNetworkReply* reply);

protected:
    void finishWaitEmailStatusWeb();
    void finishWaitInfoWeb();
    void finishWaitUnitWeb();

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
    QString m_serial;
    int m_helloAttempts = 0;
    PopUp* m_popup = nullptr;
    ModelessPopup* m_modelessPopup = nullptr;
    InfoRequest m_infoRequest;
    InfoWeb m_infoWeb;
    UnitRequest m_unitRequest;
    UnitWeb m_unitWeb;
    qint64 m_dtUnit;
    QTimer m_unitTimer;
    int m_UnitAttempts = 0;
};

#endif // LICENSEAGENT_H

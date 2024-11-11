#ifndef LICENSEAGENT_H
#define LICENSEAGENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include "popup.h"
#include "modelesspopup.h"

#define SERVER_NAME "http://regdev.rigexpert.com/index.php"

#define EMAIL_TIMEOUT 180*1000
#define REQUEST_DELAY 5*1000
#define REQUEST_ATTEMPTS 5
#define TRANSFER_TIMEOUT 180*1000
#define REPLY_TIMEOUT 60*1000


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

    int nRez;
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
    QString deviceName;
};

struct UnitWeb
{
    UnitWeb() {}

    int nRez;
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

struct LicenseWeb {
    QString deviceName;
    QString serialNumber;
    QString status;
};

struct InfoB16 {
    QString deviceName;
    QString serialNumber;
    QString status;
    QString licenseName;
    QString purchargeDate;
    QString loginDate;
};

class LicenseAgent : public QObject
{
    Q_OBJECT
public:
    explicit LicenseAgent(QObject *parent = 0);
    ~LicenseAgent();

    enum State {WaitEmailStatusWeb, WaitUserInfoWeb, WaitInfoWeb, WaitUnitWeb, WaitLicense,
                 WaitProfileB16, WaitInfoB16, Finished, Error};

    void registerApllication(QString user, QString email);
    void registerDevice(QString device_name, QString serial, QString license);
    void updateLicense();
    void reset();
    void timeout();

    void updateUserData();

    void requestEmailStatus();
    void parseEmailStatus();

    void requestUserInfo();
    void requestInfo();
    void parseInfoWeb();
    bool infoWebIsEmpty();

    void requestUnit();
    void parseUnitWeb();
    bool needWaitForEmail();

    void requestLicense(QString key);
    void parseLicense();
    bool licenseKeyBan();

    void requestStatus_B16(QByteArray data);
    void requestInfo_B16(QByteArray data);
    void parseProfile_B16();
    void parseInfo_B16();
    bool info_B16Failed();

    void showModeless(QString title, QString text, QString buttonCancel=QString(), QString buttonOk=QString());
    void closeModeless();

    void setEmail(QString mail) { m_email = mail; }
    void setUserName(QString name) { m_userName = name; }

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
    void finishWaitUserInfoWeb();
    void finishWaitUnitWeb();
    void finishWaitLicense();
    void finishWaitProfileB16();
    void finishWaitInfoB16();

    void sendMatch_11();
    void sendInfo_B16();
    void sendProfile_B16(QByteArray data);
    void sendBlocked();

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
    LicenseWeb m_licenseWeb;
    InfoB16 m_infoB16;
    qint64 m_dtUnit;
    QTimer m_unitTimer;
    int m_UnitAttempts = 0;
    int m_licenseAttempts = 0;

};

#endif // LICENSEAGENT_H

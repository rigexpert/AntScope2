#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDomDocument>
#include <QBuffer>
#include <QLoggingCategory>
#include <QTimer>

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = 0);
    ~Downloader();

    enum State {Started, InProgress, Finished, Error};

    State startDownloadInfo(QUrl url);
    State startDownloadFw();
    State state() const;
    bool hasDownloadLink() const;

    QString downloadLink() const;
    QString version() const;
    QString info() const;
    QString error() const;

    QByteArray file() const;


signals:
    void downloadFileComplete();
    void downloadInfoComplete();
    void progress(qint64 dowload, qint64 total);


public slots:
    void reset();

private slots:
    void fileDownloaded(QNetworkReply* reply);
    void timeout();
    void progressTmr(qint64 dowload, qint64 total);

private:
    State m_state;
    QNetworkAccessManager m_mng;
    QByteArray  m_arr;
    QNetworkReply *m_reply;

    QString m_link;
    QString m_info;
    QString m_ver;
    QString m_lastError;
    bool m_isInfo;
    QTimer m_timer;


    void parse(QByteArray *data);
    bool isHTML(const QByteArray &arr);
};

Q_DECLARE_LOGGING_CATEGORY(DOWNLOADER)

#endif // DOWNLOADER_H

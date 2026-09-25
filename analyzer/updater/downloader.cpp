#include "downloader.h"

Q_LOGGING_CATEGORY(DOWNLOADER, "downloader")

Downloader::Downloader(QObject *parent) :
    QObject(parent),
    m_state(Finished),
    m_reply(NULL),
    m_isInfo(false)
{
    connect(&m_mng, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(fileDownloaded(QNetworkReply*)));

    //20260925_vn : діагностика збоїв перевірки сертифіката.
    // ignoreSslErrors() НЕ викликаємо — інакше VerifyPeer втрачає сенс.
    connect(&m_mng, &QNetworkAccessManager::sslErrors, this,
            [this](QNetworkReply* reply, const QList<QSslError>& errors) {
                Q_UNUSED(reply)
                for (const QSslError& e : errors) {
                    qCWarning(DOWNLOADER) << "SSL error:" << int(e.error()) << e.errorString();
                    if (m_lastError.isEmpty())
                        m_lastError = tr("Certificate problem: ") + e.errorString();
                }
            });

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_timer.setSingleShot(true);
}

Downloader::~Downloader()
{

}

Downloader::State Downloader::startDownloadInfo(QUrl url)
{
    if (m_state == InProgress) {
        return m_state;
    }
    m_lastError.clear();   //20260925_vn : щоб деталі SSL не тягнулись із попереднього запиту

    QNetworkRequest request(url);

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    //conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setPeerVerifyMode(QSslSocket::VerifyPeer);
    request.setSslConfiguration(conf);

    m_mng.get(request);

    //qCDebug(DOWNLOADER) << "start download info " << url;

    m_state = InProgress;

    m_isInfo = true;
    m_sendStatisics = false;
    return Started;
}

Downloader::State Downloader::startDownloadFw()
{
    if (m_state == InProgress) {
        return m_state;
    }
    m_lastError.clear();   //20260925_vn : щоб деталі SSL не тягнулись із попереднього запиту

    m_isInfo = false;
    m_sendStatisics = false;

    QUrl url(m_link);

    qCDebug(DOWNLOADER) << "start download link " << url;

    QNetworkRequest request(url);
    QNetworkReply *reply;

    m_mng.clearAccessCache();
    QSslConfiguration conf = request.sslConfiguration();
    //conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setPeerVerifyMode(QSslSocket::VerifyPeer);
    request.setSslConfiguration(conf);

    reply = m_mng.get(request);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SIGNAL(progress(qint64,qint64)));

    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(progressTmr(qint64,qint64)));

    m_reply = reply;

    m_state = InProgress;

    return m_state;
}

Downloader::State Downloader::startSendStatistics(QUrl url)
{
    if (m_state == InProgress) {
        return m_state;
    }
    m_lastError.clear();   //20260925_vn : щоб деталі SSL не тягнулись із попереднього запиту

    QNetworkRequest request(url);
    m_mng.get(request);

    m_state = InProgress;

    m_isInfo = false;
    m_sendStatisics = true;

    return Started;
}

Downloader::State Downloader::state() const
{
    return m_state;
}


void Downloader::fileDownloaded(QNetworkReply *reply)
{
    m_state = Finished;
    m_timer.stop();

    m_arr = reply->readAll();

    //QString str_data(m_arr);
    //qDebug() << "Downloader::fileDownloaded" << str_data;


    if (reply->error() != QNetworkReply::NoError ) {
        //20260925_vn : не затирати деталі SSL, якщо вони вже записані
        if (m_lastError.isEmpty())
            m_lastError = reply->errorString();
        else
            m_lastError = reply->errorString() + "\n\n" + m_lastError;
        m_info.clear();
        m_link.clear();
    } else if(!m_isInfo && isHTML(m_arr)) {
        m_lastError = tr("Server does not have firmware file.");
    } else {
        m_lastError.clear();
        if (m_isInfo) {
            parse(&m_arr);
        }
    }

    reply->deleteLater();

    if (m_sendStatisics) {
        emit sendStatisicsComplete();
    } else {
        if (m_isInfo) {
            emit downloadInfoComplete();
        } else {
            emit downloadFileComplete();
        }
    }
    m_sendStatisics = false;
}


void Downloader::parse(QByteArray *data)
{
    QDomDocument xml;

    int line=0;
    int col=0;
    QString err;
    if (!xml.setContent(*data,&err, &line, &col)){
        qCWarning(DOWNLOADER) << "Wrong XML content: " << err << "[line " << line << " column " << col << "]";
        return;
    }

    QDomElement docElem = xml.documentElement();
    QDomNode n = docElem.firstChild();

    m_link.clear();
    m_info.clear();
    m_ver.clear();

    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if (e.tagName() == "INFO") {
                m_info = e.text();
            } else if (e.tagName() == "VERSION") {
                m_ver = e.text();
            } else if (e.tagName() == "LINK") {
                m_link = e.text();
            }
        }
        n = n.nextSibling();
    }
}

bool Downloader::isHTML(const QByteArray &arr)
{
    QString s(arr);
    if (s.indexOf("<html>") >= 0)
    {
        return true;
    }
    return false;
}

bool Downloader::hasDownloadLink() const
{
    if (m_link.isEmpty())
    {
        return false;
    }
    return true;
}

QString Downloader::downloadLink() const
{
    return m_link;
}


QString Downloader::version() const
{
    return m_ver;
}

QString Downloader::info() const
{
    return m_info;
}

QString Downloader::error() const
{
    return m_lastError;
}

QByteArray Downloader::file() const
{
    return m_arr;
}

void Downloader::reset()
{
    m_lastError.clear();
    m_info.clear();
    m_link.clear();
    m_arr.clear();
}

void Downloader::timeout()
{
    if (m_reply != NULL) {
        m_reply->abort();
        emit m_reply->finished();
        reset();
    }
}

void Downloader::progressTmr(qint64 dowload, qint64 total)
{
    Q_UNUSED(dowload);
    Q_UNUSED(total);
    m_timer.start(30000);
}



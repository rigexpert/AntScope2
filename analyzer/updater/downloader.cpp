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

    QNetworkRequest request(url);
    m_mng.get(request);

    //qCDebug(DOWNLOADER) << "start download " << url;

    m_state = InProgress;

    m_isInfo = true;
    return Started;
}

Downloader::State Downloader::startDownloadFw()
{
    if (m_state == InProgress) {
        return m_state;
    }

    m_isInfo = false;

    QUrl url(m_link);
    QNetworkRequest request(url);
    QNetworkReply *reply;

    reply = m_mng.get(request);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SIGNAL(progress(qint64,qint64)));

    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(progressTmr(qint64,qint64)));

    m_reply = reply;

    m_state = InProgress;

    return m_state;
}

Downloader::State Downloader::state() const
{
    return m_state;
}


void Downloader::fileDownloaded(QNetworkReply *reply)
{
    QByteArray arr;

    m_state = Finished;
    m_timer.stop();

    m_arr = reply->readAll();

    if (reply->error() != QNetworkReply::NoError ) {
        m_lastError = reply->errorString();
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

    if (m_isInfo) {
        emit downloadInfoComplete();
    } else {
        emit downloadFileComplete();
    }

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
        m_reply->finished();
        reset();
    }
}

void Downloader::progressTmr(qint64 dowload, qint64 total)
{
    Q_UNUSED(dowload);
    Q_UNUSED(total);
    m_timer.start(30000);
}



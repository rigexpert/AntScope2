#include "updater.h"

Updater::Updater(QObject *parent) : QObject(parent),
    m_downloader(NULL)
{

}

Updater::~Updater()
{
    if(m_downloader)
    {
        delete m_downloader;
    }
}

void Updater::on_checkUpdates()
{
    if(m_downloader == NULL)
    {
        m_downloader = new Downloader();
        connect(m_downloader, SIGNAL(downloadInfoComplete()),
                this, SLOT(on_downloadInfoComplete()));
        connect(m_downloader, SIGNAL(downloadFileComplete()),
                this, SLOT(on_downloadFileComplete()));
        connect(m_downloader, SIGNAL(progress(qint64,qint64)),
                this, SLOT(on_progress(qint64,qint64)));
    }
    QString url = "https://rigexpert.com/getsoftware?model=antscope2&revision=1";
    m_downloader->startDownloadInfo(QUrl(url));
}

void Updater::on_downloadInfoComplete()
{
    QString str = m_downloader->version();
    if(str.length() == 6)
    {
        str.insert(4,'.');
        str.insert(2,'.');
        if(str.at(6) == '0')
        {
            str.remove(6,1);
        }
        if(str.at(3) == '0')
        {
            str.remove(3,1);
        }
        if(str.at(0) == '0')
        {
            str.remove(0,1);
        }

        if(QString(ANTSCOPE2VER).remove('.').toInt() < str.remove('.').toInt())
        {
            emit newVersionAvailable();
        }
    }
}

void Updater::on_downloadFileComplete()
{
    openInstaller();
}

void Updater::on_progress(qint64 download, qint64 total)
{
    int percent = download * 100 / total;
    emit progress(percent);
}

void Updater::openInstaller()
{
    QByteArray arr = m_downloader->file();

    QString path = QDir::tempPath();
    path += "/RE.exe";

    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(arr);
    file.close();

    QString program = path;
    QStringList arguments;
    QProcess *myProcess = new QProcess();
    myProcess->start(program, arguments);
//    this->close();
    QApplication::quit();
}

void Updater::on_startDownload()
{
    m_downloader->startDownloadFw();
}

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

    QString os;
#ifdef Q_OS_WIN
    os = "&os=win";
#endif
#ifdef Q_OS_MAC
    os = "&os=mac";
#endif
#ifdef Q_OS_LINUX
    os = "&os=linux";
#endif

    QString url = "https://rigexpert.com/getsoftware?model=antscope2&revision=1" + os;
    m_downloader->startDownloadInfo(QUrl(url));
}

void Updater::on_downloadInfoComplete()
{
    QString str = m_downloader->version();

    QStringList list = QString(ANTSCOPE2VER).split('.');
    quint64 local_ver = list[2].toInt() + 1000 * list[1].toInt() + 1000000 * list[0].toInt();
    quint64 remote_ver = local_ver;
    list = QString(str).split('.');
    if (list.size() > 2)
        remote_ver = list[2].toInt() + 1000 * list[1].toInt() + 1000000 * list[0].toInt();

    // TODO DEBUG
    if (remote_ver > local_ver)
    {
        emit newVersionAvailable();
    }
    //}
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

    QDir dir = QDir::tempPath();
    QString path = dir.absoluteFilePath("RE.exe");

    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(arr);
    file.close();

    QString program = path;
    //QStringList arguments;
    //QProcess *myProcess = new QProcess();
    //myProcess->start(program, arguments);
    //myProcess->waitForStarted( -1 );
    //QProcess::ProcessError err = myProcess->error();
    //QString errString = myProcess->errorString();

    QString cmd = "cmd /c START " + program;
    system(cmd.toStdString().c_str());

    QApplication::quit();
}

void Updater::on_startDownload()
{
    m_downloader->startDownloadFw();
}

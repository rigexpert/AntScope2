#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QDialog>
#include <QSettings>
#include <QFileDialog>
#include <QTimer>
#include <QProcess>
#include <QApplication>
#include <analyzer/updater/downloader.h>

class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QObject *parent = 0);
    ~Updater();
    const Downloader* downloader() const { return m_downloader; }

private:
    Downloader *m_downloader;
    void openInstaller();

signals:
    void progress(int);
    void newVersionAvailable();

public slots:
    void on_checkUpdates();
    void on_downloadInfoComplete();
    void on_downloadFileComplete();
    void on_progress(qint64 download, qint64 total);
    void on_startDownload();
};

#endif // UPDATER_H

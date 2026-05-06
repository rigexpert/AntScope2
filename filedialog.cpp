#include "filedialog.h"
#include "style.h"

FileDialog::FileDialog(QObject *parent)
    : QObject{parent}
{}

QString FileDialog::getOpenFileName(QWidget *parent,
                               const QString &caption,
                               const QString &dir,
                               const QString &filter,
                               QString *selectedFilter)
{
    QString name;
    QFileDialog dlg(parent);
    dlg.setOption(QFileDialog::DontUseNativeDialog, true);
    dlg.setWindowTitle(caption);
    dlg.setDirectory(dir);
    dlg.setNameFilter(filter);
    QString style;
    style += Style::dialog();
    style += Style::pushButton();
    dlg.setStyleSheet(style);

    if (dlg.exec() == QDialog::Accepted) {
        name = dlg.selectedFiles().first();
    }
    return name;
}

QString FileDialog::getSaveFileName(QWidget *parent,
                               const QString &caption,
                               const QString &dir,
                               const QString &filter,
                               QString *selectedFilter)
{
    QString name;
    QFileDialog dlg(parent);
    dlg.setOption(QFileDialog::DontUseNativeDialog, true);
    dlg.setWindowTitle(caption);
    dlg.setDirectory(dir);
    dlg.setNameFilter(filter);
    QString style;
    style += Style::dialog();
    style += Style::pushButton();
    dlg.setStyleSheet(style);

    if (dlg.exec() == QDialog::Accepted) {
        name = dlg.selectedFiles().first();
    }
    return name;
}

QString FileDialog::getExistingDirectory(QWidget *parent,
                                    const QString &caption,
                                    const QString &dir,
                                    QFileDialog::Options options)
{
    QString name;
    QFileDialog dlg(parent);
    dlg.setOption(QFileDialog::DontUseNativeDialog, true);
    dlg.setWindowTitle(caption);
    dlg.setDirectory(dir);
    dlg.setOptions(options);

    QString style;
    style += Style::dialog();
    style += Style::pushButton();
    dlg.setStyleSheet(style);

    if (dlg.exec() == QDialog::Accepted) {
        name = dlg.selectedFiles().first();
    }

    return name;
}


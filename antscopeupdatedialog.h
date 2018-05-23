#ifndef ANTSCOPEUPDATEDIALOG_H
#define ANTSCOPEUPDATEDIALOG_H

#include <QDialog>

namespace Ui {
class AntScopeUpdateDialog;
}

class AntScopeUpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AntScopeUpdateDialog(QWidget *parent = 0);
    ~AntScopeUpdateDialog();

    void setAsNewVersion();
    void setAsAfterClosing();

signals:
    void downloadNow();
    void downloadAfterClosing();

private slots:
    void on_afterClosing_clicked();
    void on_dowloadNow_clicked();
    void on_close_clicked();
    void on_progress(int percent);

private:
    Ui::AntScopeUpdateDialog *ui;
};

#endif // ANTSCOPEUPDATEDIALOG_H

#ifndef APPREGISTRATIONDIALOG_H
#define APPREGISTRATIONDIALOG_H

#include <QDialog>
#include "licenseagent.h"

namespace Ui {
class AppRegistrationDialog;
}

class AppRegistrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppRegistrationDialog(LicenseAgent& agent, QWidget *parent = nullptr);
    explicit AppRegistrationDialog(QString user, QString email, LicenseAgent& agent, QWidget *parent = nullptr);
    ~AppRegistrationDialog();

    void init(QString user="", QString mail="");
    QString name();
    QString email();
    virtual void accept();

private:
    Ui::AppRegistrationDialog *ui;
    LicenseAgent& m_agent;
};

#endif // APPREGISTRATIONDIALOG_H

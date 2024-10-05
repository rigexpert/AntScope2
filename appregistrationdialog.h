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
    explicit AppRegistrationDialog(QWidget *parent = nullptr);
    ~AppRegistrationDialog();

    QString name();
    QString email();

private:
    Ui::AppRegistrationDialog *ui;
    LicenseAgent m_agent;
};

#endif // APPREGISTRATIONDIALOG_H

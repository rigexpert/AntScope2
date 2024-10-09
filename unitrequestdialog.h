#ifndef UNITREQUESTDIALOG_H
#define UNITREQUESTDIALOG_H

#include <QDialog>
#include "licenseagent.h"

namespace Ui {
class UnitRequestDialog;
}

class UnitRequestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UnitRequestDialog(ManualInfoWeb& infoWeb, QWidget *parent = nullptr);
    ~UnitRequestDialog();
    ManualInfoWeb infoWeb();

private:
    ManualInfoWeb m_infoWeb;
    Ui::UnitRequestDialog *ui;
};

#endif // UNITREQUESTDIALOG_H

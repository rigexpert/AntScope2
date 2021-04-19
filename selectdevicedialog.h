#ifndef SELECTDEVICEDIALOG_H
#define SELECTDEVICEDIALOG_H

#include <QDialog>
#include "devinfo/redeviceinfo.h"
#include "analyzer.h"
#include "analyzerparameters.h"

namespace Ui {
class SelectDeviceDialog;
}


class SelectDeviceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectDeviceDialog(QWidget *parent = 0);
    ~SelectDeviceDialog();
    int type();
    QString name();
    bool connectSilent(int _type, QString _device_name);

protected:
    void changeEvent(QEvent *e);

public slots:
    void onApply(ReDeviceInfo::InterfaceType type, QString name, QString port_or_serial);
    void onScan(int type);

private:
    Ui::SelectDeviceDialog *ui;
};

#endif // SELECTDEVICEDIALOG_H

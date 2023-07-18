#ifndef SELECTDEVICEDIALOG_H
#define SELECTDEVICEDIALOG_H

#include <QDialog>
#include "devinfo/redeviceinfo.h"
#include "analyzer.h"
#include "analyzerparameters.h"
#include "baseanalyzer.h"


namespace Ui {
class SelectDeviceDialog;
}


class SelectDeviceDialog : public QDialog
{
    Q_OBJECT
    BaseAnalyzer* m_analyzer = nullptr;
    bool m_isScanning = false;
    bool m_foundBle = false;

public:
    explicit SelectDeviceDialog(bool silent, QWidget *parent = 0);
    ~SelectDeviceDialog();
    ReDeviceInfo::InterfaceType type();
    QString name();
    bool connectSilent(int _type, QString _device_name);
    BaseAnalyzer* analyzer() { return m_analyzer; }

protected:
    void changeEvent(QEvent *e);
    void reset();
    QString scanSilent(QString& device_name);

public slots:
    void onApply(ReDeviceInfo::InterfaceType type, QString name, QString port_or_serial);
    void onScan(ReDeviceInfo::InterfaceType type);

private:
    Ui::SelectDeviceDialog *ui;
};

#endif // SELECTDEVICEDIALOG_H

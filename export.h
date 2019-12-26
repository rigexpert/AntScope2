#ifndef EXPORT_H
#define EXPORT_H

#include <QDialog>
#include <measurements.h>
#include <analyzer/analyzerparameters.h>
#include <QSettings>
#include <settings.h>

namespace Ui {
class Export;
}

class Export : public QDialog
{
    Q_OBJECT

public:
    explicit Export(QWidget *parent = 0);
    ~Export();

    void setMeasurements(Measurements * _measurements, quint32 number,
                         bool _applyCable=false, QString _description=QString());

private:
    Ui::Export *ui;
    Measurements * m_measurements;
    QSettings * m_settings;

    quint32 m_measureNumber;
    QString m_lastExportPath;
    bool m_bApplyCable = false;
    QString m_description;

private slots:
    void on_csvBtn_clicked();
    void on_zRiBtn_clicked();
    void on_sRiBtn_clicked();
    void on_sMaBtn_clicked();
    void on_nwlBtn_clicked();
};

#endif // EXPORT_H

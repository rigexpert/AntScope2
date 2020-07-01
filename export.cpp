#include "export.h"
#include "ui_export.h"

Export::Export(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Export)
{
    ui->setupUi(this);

    QString path = Settings::setIniFile();
    m_settings = new QSettings(path, QSettings::IniFormat);
    m_settings->beginGroup("Export");
    m_lastExportPath = m_settings->value("lastExportPath", "").toString();
    QRect rect = m_settings->value("geometry", 0).toRect();
    if(rect.x() != 0) {
        this->setGeometry(rect);
    }
    m_settings->endGroup();

    if (m_lastExportPath.isEmpty()) {
        m_settings->beginGroup("MainWindow");
        m_lastExportPath = m_settings->value("lastSavePath", "").toString();
        m_settings->endGroup();
    }
}

Export::~Export()
{
    m_settings->beginGroup("Export");
    m_settings->setValue("lastExportPath", m_lastExportPath);
    m_settings->setValue("geometry", this->geometry());
    m_settings->endGroup();

    delete ui;
}

void Export::setMeasurements(Measurements * _measurements, quint32 number, bool _applyCable, QString _description)
{
    m_measurements = _measurements;
    m_measureNumber = number;
    m_bApplyCable = _applyCable;
    m_description = _description;
}

void Export::on_csvBtn_clicked()
{
    if(m_measurements != NULL)
    {
        if(m_lastExportPath.indexOf('.') >= 0)
        {
            m_lastExportPath.remove(m_lastExportPath.indexOf('.'),4);
            m_lastExportPath.append(".csv");
        }
        QString path = QFileDialog::getSaveFileName(this, "Export", m_lastExportPath, "Comma Separated Values (*.csv)");

        if(!path.isEmpty())
        {
            m_lastExportPath = path;
            m_measurements->exportData(path, 0, m_measureNumber, m_bApplyCable);
        }
    }
}

void Export::on_nwlBtn_clicked()
{
    if(m_measurements != NULL)
    {
        if(m_lastExportPath.indexOf('.') >= 0)
        {
            m_lastExportPath.remove(m_lastExportPath.indexOf('.'),4);
            m_lastExportPath.append(".nwl");
        }
        QString path = QFileDialog::getSaveFileName(this, "Export", m_lastExportPath, "APAK-EL (*.nwl)");

        if(!path.isEmpty())
        {
            m_lastExportPath = path;
            m_measurements->exportData(path, 0, m_measureNumber, m_bApplyCable);
        }
    }
}

void Export::on_zRiBtn_clicked()
{
    if(m_measurements != NULL)
    {
        if(m_lastExportPath.indexOf('.') >= 0)
        {
            m_lastExportPath.remove(m_lastExportPath.indexOf('.'),4);
            m_lastExportPath.append(".s1p");
        }
        QString path = QFileDialog::getSaveFileName(this, "Export", m_lastExportPath, "Touchstone (*.s1p)");

        if(!path.isEmpty())
        {
            m_lastExportPath = path;
            m_measurements->exportData(path, 0, m_measureNumber, m_bApplyCable, m_description);
        }
    }
}

void Export::on_sRiBtn_clicked()
{
    if(m_measurements != NULL)
    {
        if(m_lastExportPath.indexOf('.') >= 0)
        {
            m_lastExportPath.remove(m_lastExportPath.indexOf('.'),4);
            m_lastExportPath.append(".s1p");
        }
        QString path = QFileDialog::getSaveFileName(this, "Export", m_lastExportPath, "Touchstone (*.s1p)");

        if(!path.isEmpty())
        {
            m_lastExportPath = path;
            m_measurements->exportData(path, 1, m_measureNumber, m_bApplyCable, m_description);
        }
    }
}

void Export::on_sMaBtn_clicked()
{
    if(m_measurements != NULL)
    {
        if(m_lastExportPath.indexOf('.') >= 0)
        {
            m_lastExportPath.remove(m_lastExportPath.indexOf('.'),4);
            m_lastExportPath.append(".s1p");
        }
        QString path = QFileDialog::getSaveFileName(this, "Export", m_lastExportPath, "Touchstone (*.s1p)");

        if(!path.isEmpty())
        {
            m_lastExportPath = path;
            m_measurements->exportData(path, 2, m_measureNumber, m_bApplyCable, m_description);
        }
    }
}

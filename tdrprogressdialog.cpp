#include "tdrprogressdialog.h"
#include "ui_tdrprogressdialog.h"
#include "settings.h"

TDRProgressDialog::TDRProgressDialog(Analyzer* _analyzer, QWidget *parent)
    : QDialog(parent), m_analyzer(_analyzer),
    ui(new Ui::TDRProgressDialog)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setStatusInfo(tr(""));
    setActionInfo(tr(""));
    setWindowOpacity(0.95);

    QString path = Settings::setIniFile();
    QSettings set(path, QSettings::IniFormat);
    set.beginGroup("MainWindow");
    m_measureSystemMetric = set.value("measureSystemMetric", true).toBool();
    set.endGroup();

    set.beginGroup("Cable");
    double cable = set.value("tdrCableLength", 20).toDouble();
    set.endGroup();

    QString units;
    if (!m_measureSystemMetric) {
        cable *= FEETINMETER;
        ui->sliderCable->setMaximum(ui->sliderCable->maximum() * FEETINMETER);
        ui->spinBoxCable->setMaximum(ui->spinBoxCable->maximum() * FEETINMETER);
        units = tr(", ft");
    } else {
        units = tr(", m");
    }
    ui->labelCable->setText(ui->labelCable->text() + units);

    ui->sliderCable->setValue(cable);
    ui->spinBoxCable->setValue(cable);

    connect(ui->startButton, &QPushButton::clicked, this, &TDRProgressDialog::on_startButton);
    connect(this, &TDRProgressDialog::startTDR, m_analyzer, &Analyzer::on_measure);
    connect(ui->stopButton, &QPushButton::clicked, this, &TDRProgressDialog::canceled);
    connect(ui->sliderCable, &QSlider::valueChanged, this, [=] (int value){
        ui->spinBoxCable->setValue(value);
    });
    connect(ui->spinBoxCable, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=] (int value){
        ui->sliderCable->setValue(value);
    });

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(1024);
    ui->progressBar->setValue(0);

}

TDRProgressDialog::~TDRProgressDialog()
{
    disconnect(this);
    delete ui;
}

void TDRProgressDialog::on_startButton()
{
    double cableLength = ui->sliderCable->value();
    if (!m_measureSystemMetric) {
        cableLength /= FEETINMETER;
    }
    const int dots = 1024;
    const int iTdrFftSize = 16384;
    const int iTdrFftSize2 = 8192;
    const double coeff = 1.0/4*SPEEDOFLIGHT;

    qint64 minFq_ = minFq[m_analyzer->getAnalyzerModel()].toULongLong()*1000;
    qint64 maxFq_ = maxFq[m_analyzer->getAnalyzerModel()].toULongLong()*1000;

    double resolution = cableLength * 1000000.0/iTdrFftSize;
    double fqMax = coeff * m_cableVelFactor / iTdrFftSize2 * (dots-1) / resolution;
    fqMax = minFq_ + fqMax*1000000;

    //double calc = fqMax;

    fqMax = qMin(maxFq_, (qint64)fqMax);

    QString path = Settings::setIniFile();
    QSettings set(path, QSettings::IniFormat);
    set.beginGroup("Cable");
    set.setValue("tdrCableLength", cableLength);
    set.endGroup();

    //qDebug() << "TDRProgressDialog" << dots << minFq_ << (double)fqMax << iTdrFftSize << resolution << cableLength << calc;
    emit startTDR(minFq_, fqMax, dots);
}

void TDRProgressDialog::setValue(int _value)
{
    ui->progressBar->setValue(_value);
}

void TDRProgressDialog::setStatusInfo(QString _strInfo)
{
    ui->lblStatusInfo->setText(_strInfo);
}

void TDRProgressDialog::setActionInfo(QString _strInfo)
{
    ui->lblActionInfo->setText(_strInfo);
}

void TDRProgressDialog::updateStatusInfo(QString _strInfo)
{
    ui->lblStatusInfo->setText(_strInfo);
}

void TDRProgressDialog::updateActionInfo(QString _strInfo)
{
    ui->lblActionInfo->setText(_strInfo);
}

void TDRProgressDialog::reject()
{
    // to skip ESC
}


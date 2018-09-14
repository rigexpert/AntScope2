#include "fqsettings.h"
#include "ui_fqsettings.h"

FqSettings::FqSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FqSettings)
{
    ui->setupUi(this);
    QTimer::singleShot(10,this, SLOT(setTitle()));
}

FqSettings::~FqSettings()
{
    delete ui;
}

void FqSettings::on_dotsSlider_valueChanged(int value)
{
    int number = (value/10)*10;
    if(number == 0)
    {
        number = 5;
    }
    ui->dotsLine->setText(QString::number(number));
}

void FqSettings::on_OKBtn_clicked()
{
    emit dotsNumber(ui->dotsLine->text().toInt());
    this->close();
}

void FqSettings::setDotsNumber(int dotsNumber)
{
    ui->dotsLine->setText(QString::number(dotsNumber));
    ui->dotsSlider->setValue(dotsNumber);
}

void FqSettings::setTitle()
{
    this->setWindowTitle(tr("Measurement points"));
}

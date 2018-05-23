#include "antscopeupdatedialog.h"
#include "ui_antscopeupdatedialog.h"

AntScopeUpdateDialog::AntScopeUpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AntScopeUpdateDialog)
{
    ui->setupUi(this);
}

AntScopeUpdateDialog::~AntScopeUpdateDialog()
{
    delete ui;
}

void AntScopeUpdateDialog::on_afterClosing_clicked()
{
    emit downloadAfterClosing();
    this->hide();
}

void AntScopeUpdateDialog::on_dowloadNow_clicked()
{
    emit downloadNow();
    ui->dowloadNow->setEnabled(false);
    ui->close->hide();
    ui->afterClosing->hide();
}

void AntScopeUpdateDialog::on_close_clicked()
{
    QApplication::exit(0);
}

void AntScopeUpdateDialog::setAsNewVersion()
{
    ui->downloadProgressBar->hide();
    ui->close->hide();
    ui->installUpdates->hide();

    ui->afterClosing->show();
    ui->newVersion->show();
}

void AntScopeUpdateDialog::setAsAfterClosing()
{
    ui->downloadProgressBar->hide();
    ui->afterClosing->hide();
    ui->newVersion->hide();

    ui->close->show();
    ui->installUpdates->show();
}

void AntScopeUpdateDialog::on_progress(int percent)
{
    ui->downloadProgressBar->show();
    ui->downloadProgressBar->setValue(percent);
}

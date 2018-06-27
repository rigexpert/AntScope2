#include "updatedialog.h"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
    ui->progressBar->hide();
    ui->statusLabel->hide();
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

void UpdateDialog::setStatusText(QString text)
{
    ui->statusLabel->setText(text);
}

void UpdateDialog::setMainText(QString text)
{
    ui->label->setText(text);
}

void UpdateDialog::on_updateBtn_clicked()
{
    if(ui->updateBtn->text() == tr("Finish"))
    {
        this->close();
    }else
    {
        emit update();
        ui->progressBar->show();
        ui->statusLabel->show();
//        setStatusText(tr("Updating, please wait..."));
        ui->updateBtn->setEnabled(false);
        ui->cancelBtn->setEnabled(false);
    }
}

void UpdateDialog::on_cancelBtn_clicked()
{
    this->close();
}

void UpdateDialog::on_percentChanged(qint32 percent)
{
    ui->progressBar->setValue(percent);
    if(percent == 100)
    {
        this->close();
    }
}

void UpdateDialog::on_errorTimerTick()
{
    m_errorTimer->stop();
    delete m_errorTimer;
    ui->progressBar->hide();

    ui->statusLabel->setText(tr("Error while updating."));
//    ui->statusLabel->setStyleSheet("QLabel { color : red; }");

    ui->updateBtn->setEnabled(true);
    ui->cancelBtn->setEnabled(true);
}

//void UpdateDialog::updateByFile()
//{
//    ui->cancelBtn->hide();//
//    ui->label->hide();
//    ui->progressBar->show();
//    ui->statusLabel->show();
//    ui->updateBtn->setText("Updating...");
//    ui->updateBtn->setEnabled(false);
//}

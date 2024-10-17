#include "inforequestdialog.h"
#include "ui_inforequestdialog.h"

InfoRequestDialog::InfoRequestDialog(QString name, QString serial, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoRequestDialog)
{
    ui->setupUi(this);
    ui->lineEditName->setText(name);
    ui->lineEditSerial->setText(serial);
    ui->lineEditLicense->setText("unlim_gold");
}

InfoRequestDialog::~InfoRequestDialog()
{
    delete ui;
}

QString InfoRequestDialog::license() { return ui->lineEditLicense->text(); }

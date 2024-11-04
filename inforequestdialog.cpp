#include "inforequestdialog.h"
#include "ui_inforequestdialog.h"

InfoRequestDialog::InfoRequestDialog(QString name, QString serial,
                                     QString license, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoRequestDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Register device"));
    ui->lineEditName->setText(name);
    ui->lineEditSerial->setText(serial);
    ui->lineEditLicense->setText(license);
}

InfoRequestDialog::~InfoRequestDialog()
{
    delete ui;
}

QString InfoRequestDialog::license() { return ui->lineEditLicense->text(); }

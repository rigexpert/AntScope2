#include "inforequestdialog.h"
#include "ui_inforequestdialog.h"
#include "style.h"

InfoRequestDialog::InfoRequestDialog(QString name, QString serial,
                                     QString license, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoRequestDialog)
{
    ui->setupUi(this);

    QString style;
    style = Style::dialog();
    style += Style::label();
    style += Style::lineEdit();
    setStyleSheet(style);

    style = "QDialogButtonBox " + Style::pushButton();
    ui->buttonBox->setStyleSheet(style);

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

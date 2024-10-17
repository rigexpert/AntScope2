#include "unitrequestdialog.h"
#include "ui_unitrequestdialog.h"

UnitRequestDialog::UnitRequestDialog(ManualInfoWeb& infoWeb, QWidget *parent) :
    QDialog(parent),
    m_infoWeb(infoWeb),
    ui(new Ui::UnitRequestDialog)
{
    ui->setupUi(this);

    ui->lineEditEmail->setText(m_infoWeb.email);
    ui->lineEditLicense->setText(m_infoWeb.licenseName);
    ui->lineEditPurcharge->setText(m_infoWeb.purchargeDate);
    ui->lineEditSerial->setText(m_infoWeb.serialNumber);
    ui->lineEditUser->setText(m_infoWeb.userName);
#ifdef _DEBUG
    ui->lineEditEmail->setText("vancom@bigmir.net");
    ui->lineEditPurcharge->setText("19.09.2024");
    ui->lineEditSerial->setText("123208137");
    ui->lineEditUser->setText("Ivan2");
#endif
}

UnitRequestDialog::~UnitRequestDialog()
{
    delete ui;
}

ManualInfoWeb UnitRequestDialog::infoWeb()
{
    m_infoWeb.licenseName = ui->lineEditLicense->text();
    m_infoWeb.purchargeDate = ui->lineEditPurcharge->text();
    m_infoWeb.serialNumber = ui->lineEditSerial->text();
    m_infoWeb.userName = ui->lineEditUser->text();
    m_infoWeb.email = ui->lineEditEmail->text();
    return m_infoWeb;
}

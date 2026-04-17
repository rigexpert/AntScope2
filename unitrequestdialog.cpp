#include "unitrequestdialog.h"
#include "ui_unitrequestdialog.h"
#include <QRegularExpression>
#include <QMessageBox>
#include "style.h"

extern int g_showMessageBox(QWidget* parent, QMessageBox::Icon icon,
                            QString title, QString text,
                            QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

UnitRequestDialog::UnitRequestDialog(ManualInfoWeb& infoWeb, QWidget *parent) :
    QDialog(parent),
    m_infoWeb(infoWeb),
    ui(new Ui::UnitRequestDialog)
{
    ui->setupUi(this);

    QString style = Style::dialog();
    style += Style::label();
    style += Style::lineEdit();
    style += Style::pushButton();
    setStyleSheet(style);

    style = Style::pushButton();
    ui->pushButtonCancel->setStyleSheet(style);
    ui->pushButtonOk->setStyleSheet(style);

    ui->lineEditEmail->setText(m_infoWeb.email);
    ui->lineEditLicense->setText(m_infoWeb.licenseName);
    ui->lineEditPurcharge->setText(m_infoWeb.purchargeDate);
    ui->lineEditSerial->setText(m_infoWeb.serialNumber);
    ui->lineEditUser->setText(m_infoWeb.userName);

    connect(ui->pushButtonCancel, &QPushButton::clicked, this, [=](){
        reject();
    });
    connect(ui->pushButtonOk, &QPushButton::clicked, this, [=](){
        if (ui->lineEditEmail->text().isEmpty()) {
            g_showMessageBox(this, QMessageBox::Warning, tr(""), tr("Email address should be valid"));
            return;
        }
        QRegularExpression mailREX("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
        if (!mailREX.match(ui->lineEditEmail->text().toUpper()).hasMatch()) {
            g_showMessageBox(this, QMessageBox::Warning, "", tr("Wrong email address"));
            return;
        }
        accept();
    });

//#ifdef _DEBUG
//    ui->lineEditEmail->setText("vancom@bigmir.net");
//    ui->lineEditPurcharge->setText("19.09.2024");
//    ui->lineEditSerial->setText("123208137");
//    ui->lineEditUser->setText("Ivan2");
//#endif
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

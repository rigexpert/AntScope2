#include "appregistrationdialog.h"
#include "ui_appregistrationdialog.h"
#include <QMessageBox>
#include <QRegularExpression>

AppRegistrationDialog::AppRegistrationDialog(LicenseAgent& agent, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AppRegistrationDialog),
    m_agent(agent)

{
    ui->setupUi(this);

    connect(&m_agent, &LicenseAgent::registered, this, [=](){
        accept();
    });
    connect(&m_agent, &LicenseAgent::networkTimeout, this, [=](){
        close();
    });
    connect(&m_agent, &LicenseAgent::canceled, this, [=](){
        reject();
    });
    connect(ui->pushButtonOk, &QPushButton::clicked, this, [=](){
        if (name().isEmpty() || email().isEmpty()) {
            QMessageBox::warning(this, tr(""), tr("All fields are mandatory"));
            return;
        }
//        QRegularExpression mailREX("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
//        if (!mailREX.match(email().toUpper()).hasMatch()) {
//            QMessageBox::warning(this, "", tr("Wrong email address"));
//            return;
//        }
        accept();
        m_agent.registerApllication(name(), email());
    });
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, [=](){
        reject();
    });

#ifdef _DEBUG
    ui->lineEditEmail->setText("vancom1@bigmir.net");
    ui->lineEditUserName->setText("Ivan1");
#endif
}

AppRegistrationDialog::~AppRegistrationDialog()
{
    delete ui;
}

QString AppRegistrationDialog::name() { return ui->lineEditUserName->text(); }
QString AppRegistrationDialog::email() { return ui->lineEditEmail->text(); }

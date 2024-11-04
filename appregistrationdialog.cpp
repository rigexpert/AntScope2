#include "appregistrationdialog.h"
#include "ui_appregistrationdialog.h"
#include <QMessageBox>
#include <QRegularExpression>

AppRegistrationDialog::AppRegistrationDialog(LicenseAgent& agent, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AppRegistrationDialog),
    m_agent(agent)

{
    init();
}

AppRegistrationDialog::AppRegistrationDialog(QString user, QString mail, LicenseAgent& agent, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AppRegistrationDialog),
    m_agent(agent)
{
    init(user, mail);
}

void AppRegistrationDialog::init(QString user, QString mail)
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
        if (email().isEmpty()) {
            QMessageBox::warning(this, tr(""), tr("Email address should be valid"));
            return;
        }
        QRegularExpression mailREX("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
        if (!mailREX.match(email().toUpper()).hasMatch()) {
            QMessageBox::warning(this, "", tr("Wrong email address"));
            return;
        }
        accept();
        m_agent.registerApllication(name(), email());
    });
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, [=](){
        reject();
    });

    if (!mail.isEmpty())
        ui->lineEditEmail->setText(mail);
    if (!user.isEmpty())
        ui->lineEditUserName->setText(user);
}

AppRegistrationDialog::~AppRegistrationDialog()
{
    delete ui;
}

void AppRegistrationDialog::accept()
{
    if (email().isEmpty()) {
        return;
    }
    QDialog::accept();
}

QString AppRegistrationDialog::name() { return ui->lineEditUserName->text(); }
QString AppRegistrationDialog::email() { return ui->lineEditEmail->text(); }

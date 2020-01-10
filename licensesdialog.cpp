#include "licensesdialog.h"
#include "ui_licensesdialog.h"

LicensesDialog::LicensesDialog(Analyzer* analyzer, QWidget *parent) :
    QDialog(parent),
    m_analyzer(analyzer),
    ui(new Ui::LicensesDialog)
{
    ui->setupUi(this);

    connect(ui->refreshButton, &QPushButton::pressed, m_analyzer, &Analyzer::on_getLicenses);
    connect(ui->generateButton, &QPushButton::pressed, m_analyzer, &Analyzer::on_generateLicence);
    connect(ui->applyButton, &QPushButton::pressed, this, &LicensesDialog::on_applyLicense);
    connect(this, &LicensesDialog::signalApplyLicense, m_analyzer, &Analyzer::on_applyLicense);

    connect(m_analyzer, &Analyzer::licensesList, this, &LicensesDialog::on_getList);
    connect(m_analyzer, &Analyzer::licenseRequest, this, &LicensesDialog::on_generateRequest);
    connect(m_analyzer, &Analyzer::licenseApplyResult, this, &LicensesDialog::on_applyResult);
}

LicensesDialog::~LicensesDialog()
{
    delete ui;
}

void LicensesDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void LicensesDialog::on_getList(QString& _license)
{
    ui->listWidget->addItem(_license);
}

void LicensesDialog::on_generateRequest(QString& _request)
{
    ui->requestEdit->setText(_request);
    ui->requestEdit->selectAll();
}

void LicensesDialog::on_applyLicense()
{
    QString license = ui->applyEdit->text();
    if (!license.isEmpty())
        emit signalApplyLicense(license);
}

void LicensesDialog::on_applyResult(QString& _result)
{
    ui->labelStatus->setText(_result);
}



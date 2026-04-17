#include "editbandsdialog.h"
#include "ui_editbandsdialog.h"
#include <QAbstractButton>
#include <QFile>
#include "settings.h"
#include "style.h"

extern int g_showMessageBox(QWidget* parent, QMessageBox::Icon icon,
                            QString title, QString text,
                            QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                            QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

EditBandsDialog::EditBandsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditBandsDialog)
{
    ui->setupUi(this);

    QString style;
    style = Style::dialog();
    style += "QDialogButtonBox QPushButton {background-color: rgb(26, 45, 98);color: white;}";
    setStyleSheet(style);

    QFont font = ui->textEdit->font();
    font.setPointSize(12);
    ui->textEdit->setFont(font);

    connect(ui->buttonBox, &QDialogButtonBox::clicked, [=](QAbstractButton* _button){
        QPushButton* button = qobject_cast<QPushButton*>(_button);
        if(button == ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)) {
            loadDefaults();
        } else if(button == ui->buttonBox->button(QDialogButtonBox::Save)) {
            save();
            QDialog::accept();
        } else if(button == ui->buttonBox->button(QDialogButtonBox::Cancel)) {
            QDialog::reject();
        }
    });
    load();
}

EditBandsDialog::~EditBandsDialog()
{
    delete ui;
}

void EditBandsDialog::changeEvent(QEvent *e)
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

bool EditBandsDialog::loadDefaults()
{
    QString ituPath = Settings::programDataPath("itu-regions-defaults.txt");

    QFile file(ituPath);
    bool res = file.open(QFile::ReadOnly);
    if(!res) {
        qDebug() << "load defaults" << file.errorString() << ituPath;
        g_showMessageBox(this, QMessageBox::Information, "loadDefaults", file.errorString() + ituPath);
        return false;
    }

    m_filePath = ituPath;

    ui->textEdit->clear();
    QTextStream stream(&file);
    ui->textEdit->setText(stream.readAll());
    file.close();

    return true;
}

bool EditBandsDialog::load()
{
    QString ituPath = Settings::programDataPath("itu-regions.txt");
    QFile file(ituPath);
    if (!file.exists()) {
        file.setFileName(Settings::programDataPath("itu-regions-defaults.txt"));
    }
    bool res = file.open(QFile::ReadOnly);
    if(!res) {
        qDebug() << "load" << file.errorString() << ituPath;
        g_showMessageBox(this, QMessageBox::Information, "load", file.errorString() + ituPath);
        return false;
    }

    m_filePath = ituPath;

    ui->textEdit->clear();
    QTextStream stream(&file);
    ui->textEdit->setText(stream.readAll());
    file.close();

    return true;
}

bool EditBandsDialog::save()
{
    QString ituPath = Settings::programDataPath("itu-regions.txt");
    QFile file(ituPath);
    bool res = file.open(QFile::Truncate|QFile::WriteOnly|QFile::Text);
    if(!res) {
        qDebug() << "save" << file.errorString() << ituPath;
        g_showMessageBox(this, QMessageBox::Information, "EditBandsDialog::save", file.errorString() + ituPath);
        return false;
    }
    QTextStream stream(&file);
    stream << ui->textEdit->toPlainText();

    file.flush();
    file.close();
    m_changed = true;

    return true;
}


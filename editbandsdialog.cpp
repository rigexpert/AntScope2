#include "editbandsdialog.h"
#include "ui_editbandsdialog.h"
#include <QAbstractButton>
#include <QFile>
#include "settings.h"

EditBandsDialog::EditBandsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditBandsDialog)
{
    ui->setupUi(this);
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
        //QMessageBox::information(this, "loadDefaults", file.errorString() + ituPath);
        return false;
    }
    ui->textEdit->clear();
    QTextStream stream(&file);
    ui->textEdit->setText(stream.readAll());
    file.close();

    return true;
}

bool EditBandsDialog::save()
{
    QString ituPath = Settings::localDataPath("itu-regions.txt");

    QFile file(ituPath);
    bool res = file.open(QFile::Truncate|QFile::WriteOnly|QFile::Text);
    if(!res) {
        qDebug() << "save" << file.errorString() << ituPath;
        return false;
    }
    QTextStream stream(&file);
    stream << ui->textEdit->toPlainText();

    file.flush();
    file.close();
    m_changed = true;

    return true;
}

bool EditBandsDialog::load()
{
    QString ituPath = Settings::localDataPath("itu-regions.txt");
    QFile file(ituPath);
    if (!file.exists()) {
        file.setFileName(Settings::programDataPath("itu-regions-defaults.txt"));
    }
    bool res = file.open(QFile::ReadOnly);
    if(!res) {
        qDebug() << "load" << file.errorString() << ituPath;
        //QMessageBox::information(this, "load", file.errorString() + ituPath);
        return false;
    }

    ui->textEdit->clear();
    QTextStream stream(&file);
    ui->textEdit->setText(stream.readAll());
    file.close();

    return true;
}

/*
void fillBands()
{
    static QString bands[] = {
        "[ITU Region 1 - Europe, Africa]"
        ,"135.7, 137.8, 2200m"
        ,"472, 479, 630m"
        ,"1810, 2000, 160m"
        ,"3500, 3800, 80m"
        ,"5250, 5450, 60m"
        ,"7000, 7200, 40m"
        ,"10100, 10150, 30m"
        ,"14000, 14350, 20m"
        ,"18068, 18168, 17m"
        ,"21000, 21450, 15m"
        ,"24890, 24990, 12m"
        ,"28000, 29700, 10m"
        ,"50000, 52000, 6m"
        ,"69000, 70500, 4m"
        ,"144000, 146000, 2m"
        ,"430000, 440000, 70sm"
        ,"1260000, 1300000, 23sm"
        ,"[ITU Region 2 - Americas]"
        ,"135.7, 137.8, 2200m"
        ,"472, 479, 630m"
        ,"1800, 2000, 160m"
        ,"3500, 4000, 80m"
        ,"5250, 5450, 60m"
        ,"7000, 7300, 40m"
        ,"10100, 10150, 30m"
        ,"14000, 14350, 20m"
        ,"18068, 18168, 17m"
        ,"21000, 21450, 15m"
        ,"24890, 24990, 12m"
        ,"28000, 29700, 10m"
        ,"50000, 54000, 6m"
        ,"144000, 148000, 2m"
        ,"420000, 450000, 70sm"
        ,"1240000, 1300000, 23sm"
        ,"[ITU Region 3- Asia, Oceania]"
        ,"135.7, 137.8, 2200m"
        ,"472, 479, 630m"
        ,"1800, 2000, 160m"
        ,"3500, 3900, 80m"
        ,"5351, 5366, 60m"
        ,"7000, 7300, 40m"
        ,"10100, 10150, 30m"
        ,"14000, 14350, 20m"
        ,"18068, 18168, 17m"
        ,"21000, 21450, 15m"
        ,"24890, 24990, 12m"
        ,"28000, 29700, 10m"
        ,"50000, 54000, 6m"
        ,"144000, 148000, 2m"
        ,"430000, 450000, 70sm"
        ,"1240000, 1300000, 23sm"
    };

}
*/

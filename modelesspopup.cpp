#include "modelesspopup.h"
#include "ui_modelesspopup.h"

ModelessPopup::ModelessPopup(QString text, QString buttonCancel, QString buttonOk, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModelessPopup)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    ui->label->setText(text);
    ui->pushButton->setText(buttonCancel);
    connect(ui->pushButton, &QPushButton::clicked, this, [=](){
        reject();
    });
    if (buttonOk.isEmpty()) {
        ui->pushButton_2->hide();
    } else {
        ui->pushButton_2->setText(buttonOk);
        connect(ui->pushButton_2, &QPushButton::clicked, this, [=](){
            accept();
        });
    }
}

ModelessPopup::~ModelessPopup()
{
    delete ui;
}

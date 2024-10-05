#include "modelesspopup.h"
#include "ui_modelesspopup.h"

ModelessPopup::ModelessPopup(QString text, QString button, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModelessPopup)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    ui->label->setText(text);
    ui->pushButton->setText(button);
    connect(ui->pushButton, &QPushButton::clicked, this, [=](){
        reject();
    });
}

ModelessPopup::~ModelessPopup()
{
    delete ui;
}

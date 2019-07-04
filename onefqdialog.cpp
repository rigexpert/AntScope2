#include "onefqdialog.h"
#include "ui_onefqdialog.h"

OneFqDialog::OneFqDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OneFqDialog)
{
    ui->setupUi(this);
    connect(ui->stopButton, &QPushButton::clicked, this, &OneFqDialog::canceled);
    QLabel* label = ui->label;
    label->setStyleSheet("QLabel { color : white;"
                        "margin-top: 6px;"
                        "margin-bottom: 6px;"
                        "margin-left: 10px;"
                        "margin-right: 10px; }");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
}

OneFqDialog::~OneFqDialog()
{
    delete ui;
}

void OneFqDialog::changeEvent(QEvent *e)
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

void OneFqDialog::reject()
{
    emit canceled();
}

void OneFqDialog::update(QString _msg)
{
    ui->label->setText(_msg);
    QWidget::update();
    QApplication::processEvents();
}

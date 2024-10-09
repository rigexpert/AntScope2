#ifndef INFOREQUESTDIALOG_H
#define INFOREQUESTDIALOG_H

#include <QDialog>

namespace Ui {
class InfoRequestDialog;
}

class InfoRequestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoRequestDialog(QString name, QString serial, QWidget *parent = nullptr);
    ~InfoRequestDialog();

    QString license();

private:
    Ui::InfoRequestDialog *ui;
};

#endif // INFOREQUESTDIALOG_H

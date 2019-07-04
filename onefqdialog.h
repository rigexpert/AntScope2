#ifndef ONEFQDIALOG_H
#define ONEFQDIALOG_H

#include <QDialog>

namespace Ui {
class OneFqDialog;
}

class OneFqDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OneFqDialog(QWidget *parent = 0);
    ~OneFqDialog();
    void update(QString _msg);

protected:
    void changeEvent(QEvent *e);

public slots:
    virtual void reject();

signals:
        void canceled();

private:
    Ui::OneFqDialog *ui;
};

#endif // ONEFQDIALOG_H

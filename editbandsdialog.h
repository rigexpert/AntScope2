#ifndef EDITBANDSDIALOG_H
#define EDITBANDSDIALOG_H

#include <QDialog>

namespace Ui {
class EditBandsDialog;
}

class EditBandsDialog : public QDialog
{
    Q_OBJECT
    bool m_changed = false;

public:
    explicit EditBandsDialog(QWidget *parent = 0);
    ~EditBandsDialog();
    bool changed() { return m_changed; }

protected:
    void changeEvent(QEvent *e);
    bool load();
    bool loadDefaults();
    bool save();

private:
    Ui::EditBandsDialog *ui;
};

#endif // EDITBANDSDIALOG_H

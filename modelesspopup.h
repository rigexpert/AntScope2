#ifndef MODELESSPOPUP_H
#define MODELESSPOPUP_H

#include <QDialog>

namespace Ui {
class ModelessPopup;
}

class ModelessPopup : public QDialog
{
    Q_OBJECT

public:
    explicit ModelessPopup(QString title, QString text, QString buttonCancel=QString(), QString buttonOk=QString(), QWidget *parent = nullptr);
    ~ModelessPopup();

    QString title() { return windowTitle(); }
    QString text();

private:
    Ui::ModelessPopup *ui;
};

#endif // MODELESSPOPUP_H

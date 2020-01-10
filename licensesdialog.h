#ifndef LICENSESDIALOG_H
#define LICENSESDIALOG_H

#include <QDialog>
#include "analyzer.h"

namespace Ui {
class LicensesDialog;
}

class LicensesDialog : public QDialog
{
    Q_OBJECT
    Analyzer* m_analyzer;

public:
    explicit LicensesDialog(Analyzer* analyzer, QWidget *parent = 0);
    ~LicensesDialog();

signals:
    void signalApplyLicense(QString& _license);

public slots:
    void on_getList(QString& _license);
    void on_generateRequest(QString& _request);
    void on_applyLicense();
    void on_applyResult(QString& _result);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::LicensesDialog *ui;
};

#endif // LICENSESDIALOG_H

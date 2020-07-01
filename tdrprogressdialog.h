#ifndef TDRPROGRESSDIALOG_H
#define TDRPROGRESSDIALOG_H

#include <QDialog>
#include "analyzer.h"


#ifndef FEETINMETER
#define FEETINMETER 3.2808399
#endif

#ifndef SPEEDOFLIGHT
#define SPEEDOFLIGHT 299792458.0
#endif


namespace Ui {
class TDRProgressDialog;
}

class TDRProgressDialog : public QDialog
{
    Q_OBJECT

    Analyzer* m_analyzer;
    int m_iMaxValue = 1024;
    double m_cableVelFactor=1;
    bool m_measureSystemMetric = true;

public:
    explicit TDRProgressDialog(Analyzer* _analyzer, QWidget *parent = 0);
    ~TDRProgressDialog();

    void setStatusInfo(QString _strInfo);
    void setActionInfo(QString _strInfo);
    void setCableVelFactor(double _cableVelFactor) { m_cableVelFactor = _cableVelFactor; }
    int maxValue() { return m_iMaxValue; }

public slots:
    void setValue(int _value);
    void updateStatusInfo(QString _strInfo);
    void updateActionInfo(QString _strInfo);
    virtual void reject();

protected slots:
    void on_startButton();

signals:
    void startTDR(qint64,qint64,int);
    void canceled();

private:
    Ui::TDRProgressDialog *ui;
};

#endif // TDRPROGRESSDIALOG_H

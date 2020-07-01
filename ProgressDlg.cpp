#include "ProgressDlg.h"

ProgressDlg::ProgressDlg(QWidget * parent) : QDialog(parent),
	m_iMinValue(0), 
	m_iMaxValue(100), 
	m_iStepValue(1)
{
	setupUi(this);
	setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
	setStatusInfo(tr(""));
	setActionInfo(tr(""));
	setWindowOpacity(0.95);
    stopButton->hide();
}

ProgressDlg::~ProgressDlg() {
	
}

void ProgressDlg::setProgressData(int _iMinValue, int _iMaxValue, int _iStepValue)
{
	m_iMinValue = _iMinValue;
	m_iMaxValue = _iMaxValue;
	m_iStepValue = _iStepValue;
	progressBar->setMinimum(m_iMinValue);
	progressBar->setMaximum(m_iMaxValue);
	progressBar->setValue(0);
}


void ProgressDlg::stepIt()
{
	progressBar->setValue(progressBar->value() + m_iStepValue);
}

void ProgressDlg::setValue(int _value)
{
	progressBar->setValue(_value);
}

void ProgressDlg::setStatusInfo(QString _strInfo)
{
	lblStatusInfo->setText(_strInfo);
}

void ProgressDlg::setActionInfo(QString _strInfo)
{
	lblActionInfo->setText(_strInfo);
}

void ProgressDlg::updateStatusInfo(QString _strInfo)
{
	lblStatusInfo->setText(_strInfo);
}

void ProgressDlg::updateActionInfo(QString _strInfo)
{
	lblActionInfo->setText(_strInfo);
}

void ProgressDlg::reject()
{
    // to skip ESC
}

void ProgressDlg::setCancelable(bool _cancelable)
{
    if (_cancelable) {
        stopButton->show();
        connect(stopButton, &QPushButton::clicked, this, &ProgressDlg::canceled);
    } else {
        stopButton->hide();
    }
}

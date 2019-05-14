#pragma once

#include <QDialog>
#include "ui_ProgressDlg.h"


class ProgressDlg : public QDialog, public Ui::dlgProgress
{
	Q_OBJECT

	public:
        explicit ProgressDlg(QWidget * parent = Q_NULLPTR);
        ~ProgressDlg();
		
		void setStatusInfo(QString _strInfo);
		void setActionInfo(QString _strInfo);

	public slots:
		void setProgressData(int _iMinValue, int _iMaxValue, int _iStepValue=1);
		void stepIt();
		void setValue(int _value);
		void updateStatusInfo(QString _strInfo);
		void updateActionInfo(QString _strInfo);

	private:
		int m_iMinValue;
		int m_iMaxValue;
		int m_iStepValue;

};

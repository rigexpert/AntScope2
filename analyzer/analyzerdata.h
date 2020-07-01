#ifndef ANALYZERDATA_H
#define ANALYZERDATA_H

#include <QDialog>
#include <qdebug.h>
#include <QListWidgetItem>
#include <QProgressDialog>

namespace Ui {
class AnalyzerData;
}

class AnalyzerData : public QDialog
{
    Q_OBJECT
    int m_model;

public:
    explicit AnalyzerData(int _model, QWidget *parent = 0);
    ~AnalyzerData();

private:
    Ui::AnalyzerData *ui;

    bool m_isSelected;
    QProgressDialog* progressDialog;
    int progressSteps;
    QString strSaveDir;
    QString strItemName;

signals:
    void itemDoubleClick(QString, QString, QString);
    void dialogClosed(void);
    void signalSaveFile(int row, QString path);
    void dataChanged(qint64 _center, qint64 _range, qint32 _dots);

public slots:
    void on_analyzerDataStringArrived(QString str);

private slots:
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_buttonBox_accepted();
    void on_btnReadAll_clicked();
    void on_complete();
    void on_finish();
    void nextStep();
};

#endif // ANALYZERDATA_H

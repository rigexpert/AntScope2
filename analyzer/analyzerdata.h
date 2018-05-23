#ifndef ANALYZERDATA_H
#define ANALYZERDATA_H

#include <QDialog>
#include <qdebug.h>
#include <QListWidgetItem>

namespace Ui {
class AnalyzerData;
}

class AnalyzerData : public QDialog
{
    Q_OBJECT

public:
    explicit AnalyzerData(QWidget *parent = 0);
    ~AnalyzerData();

private:
    Ui::AnalyzerData *ui;

    bool m_isSelected;
signals:
    void itemDoubleClick(QString, QString, QString);
    void dialogClosed(void);

public slots:
    void on_analyzerDataStringArrived(QString str);

private slots:
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_buttonBox_accepted();
};

#endif // ANALYZERDATA_H

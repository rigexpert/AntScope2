#include "analyzerdata.h"
#include "ui_analyzerdata.h"

AnalyzerData::AnalyzerData(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnalyzerData),
    m_isSelected(false)
{
    ui->setupUi(this);
}

AnalyzerData::~AnalyzerData()
{
    if(!m_isSelected)
    {
        emit dialogClosed();
    }
    delete ui;
}

void AnalyzerData::on_analyzerDataStringArrived(QString str)
{
    ui->listWidget->addItem(str);
}

void AnalyzerData::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    m_isSelected = true;
    QString str = item->text();
    QStringList list = str.split(",");
    QStringList list2 = list.at(3).split(":");

    emit itemDoubleClick(list.at(0), list2.at(0), list2.at(1));
    this->close();
}

void AnalyzerData::on_buttonBox_accepted()
{
    QList<QListWidgetItem*> list = ui->listWidget->selectedItems();
    if(list.length() != 0)
    {
        QString str = list.at(0)->data(0).toString();
        QStringList list1 = str.split(",");
        QStringList list2 = list1.at(3).split(":");

        emit itemDoubleClick(list1.at(0), list2.at(0), list2.at(1));
    }
}

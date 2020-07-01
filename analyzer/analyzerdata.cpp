#include "analyzerdata.h"
#include "ui_analyzerdata.h"
#include <QFileDialog>
#include "mainwindow.h"
#include <QCoreApplication>
#include <iostream>
#include "Notification.h"

AnalyzerData::AnalyzerData(int _model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnalyzerData),
    m_isSelected(false), m_model(_model),
    progressDialog(nullptr)
{
    ui->setupUi(this);
//    setWindowTitle(tr("Read analyzer data"));
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

    int div = 1;
    QString model = names[m_model];
    if (model == "AA-230 ZOOM" || model == "AA-55 ZOOM" || model == "AA-650 ZOOM")
        div = 1000;
    // center, range, dots
    emit dataChanged(list[1].toULongLong()/div, list[2].toULongLong()/div, list2[0].toInt());
    // index, dots, name
    emit itemDoubleClick(list.at(0), list2.at(0), list2.at(1));
    this->close();
}

void AnalyzerData::on_buttonBox_accepted()
{
    QList<QListWidgetItem*> list = ui->listWidget->selectedItems();
    if(list.length() != 0)
    {
        on_listWidget_itemDoubleClicked(list.at(0));
    }
}

void AnalyzerData::on_btnReadAll_clicked()
{
    QWidget* parent = parentWidget();
    MainWindow* mainWindow = qobject_cast<MainWindow*>(parent);
    strSaveDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 mainWindow->lastSavePath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if (strSaveDir.isEmpty())
        return;

    mainWindow->lastSavePath() = strSaveDir;

    connect(mainWindow->analyzer(), &Analyzer::measurementComplete, this, &AnalyzerData::on_complete);

    progressDialog = new QProgressDialog("Reading data...", "Abort", 0, ui->listWidget->count(), this);
    connect(progressDialog, &QProgressDialog::canceled, this, &AnalyzerData::on_finish);

    progressSteps = 0;
    progressDialog->setValue(progressSteps);
    progressDialog->show();
    QCoreApplication::processEvents();

    QListWidgetItem* item = ui->listWidget->item(progressSteps);
    QString str = item->text();
    QStringList list = str.split(",");
    QStringList list2 = list.at(3).split(":");
    strItemName = QString("%1-%2").arg(progressSteps, 2, 10, QLatin1Char('0')).arg(list2.at(1));

    emit itemDoubleClick(list.at(0), list2.at(0), strItemName);
    QCoreApplication::processEvents();
}

void AnalyzerData::on_complete()
{
    QDir dir = strSaveDir;
    QString path = dir.absoluteFilePath(strItemName+".asd");
    emit signalSaveFile(progressSteps, path);
    QCoreApplication::processEvents();

    //QTimer::singleShot(5000, this, SLOT(nextStep()));
    nextStep();
}

void AnalyzerData::nextStep()
{
    int end = ui->listWidget->count()-1;
    if (progressSteps < end)
    {
        progressDialog->setValue(progressSteps+1);
        QCoreApplication::processEvents();

        QListWidgetItem* item = ui->listWidget->item(++progressSteps);
        QString str = item->text();
        QStringList list = str.split(",");
        if (list.size() < 3)
        {
            on_finish();
            return;
        }
        QStringList list2 = list.at(3).split(":");
        strItemName = QString("%1-%2").arg(progressSteps, 2, 10, QLatin1Char('0')).arg(list2.at(1));

        emit itemDoubleClick(list.at(0), list2.at(0), strItemName);
        QCoreApplication::processEvents();
    }
    else
    {
        progressDialog->setValue(++progressSteps);
        QCoreApplication::processEvents();
        on_finish();
    }
}

void AnalyzerData::on_finish()
{
    MainWindow* mainWindow = qobject_cast<MainWindow*>(parentWidget());
    disconnect(mainWindow->analyzer(), &Analyzer::measurementComplete, this, &AnalyzerData::on_complete);
    delete progressDialog;
    progressDialog = nullptr;
    this->close();
}


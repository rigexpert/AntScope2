#include "presets.h"

Presets::Presets(QObject *parent) : QObject(parent),
    m_isRange(false)
{    
    QString path = Settings::setIniFile();
    m_settings = new QSettings(path, QSettings::IniFormat);
}

Presets::~Presets()
{
    m_settings->beginGroup("Presets");
    m_settings->setValue("Quantity", m_fqFromList.length());
    QString str;
    for(int i = 0; i < m_fqFromList.length(); ++i)
    {
        str = m_fqFromList.at(i);
        str += ",";
        str += m_fqToList.at(i);
        str += ",";
        str += m_fqPointsList.at(i);
        m_settings->setValue(QString("Preset%1").arg(i),str);
    }
    m_settings->setValue("isRange", m_isRange);
    m_settings->endGroup();
    delete m_settings;
}

void Presets::setTable (QTableWidget * table)
{
    m_tableWidget = table;    
    m_tableWidget->setColumnCount(3);
    m_tableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    m_settings->beginGroup("Presets");
    int count = m_settings->value("Quantity",0).toInt();
    QString str;
    QStringList list;
    for(int i = 0; i < count; ++i)
    {
        str = m_settings->value(QString("Preset%1").arg(i),QString()).toString();
        list = str.split(",");
        if(list.length() > 1)
        {
            m_fqFromList.append(list.at(0));
            m_fqToList.append(list.at(1));
            m_fqPointsList.append(list.length() > 2 ? list.at(2) : "50");
        }
    }
    m_isRange = m_settings->value("isRange", false).toBool();
    m_settings->endGroup();
    m_tableWidget->setRowCount(m_fqFromList.length());
    m_tableWidget->resizeColumnsToContents();
    refresh();    
}

QStringList Presets::getRow(int row)
{
    QStringList list;
    list.append(m_fqFromList.at(row));
    list.append(m_fqToList.at(row));
    list.append(m_fqPointsList.at(row));
    return list;
}

void Presets::addNewRow (QString fqFrom, QString fqTo, QString fqPoints)
{
    m_fqFromList.append(fqFrom);
    m_fqToList.append(fqTo);
    m_fqPointsList.append(fqPoints);
    m_tableWidget->setRowCount(m_fqFromList.length());
    refresh();
}

void Presets::deleteRow (int number)
{
    m_fqFromList.removeAt(number);
    m_fqToList.removeAt(number);
    m_fqPointsList.removeAt(number);
    m_tableWidget->removeRow(number);

    if(number == m_fqFromList.length())
    {
        number--;
    }

    QModelIndex myIndex = m_tableWidget->model()->index( number, 0, QModelIndex());
    m_tableWidget->selectionModel()->select(myIndex,QItemSelectionModel::Select);

    myIndex = m_tableWidget->model()->index( number, 1, QModelIndex());
    m_tableWidget->selectionModel()->select(myIndex,QItemSelectionModel::Select);
}

void Presets::moveRowUp (int number)
{
    if(number == 0)
    {
        return;
    }
    QString from = m_fqFromList.takeAt(number);
    QString to = m_fqToList.takeAt(number);
    QString points = m_fqPointsList.takeAt(number);

    m_fqFromList.insert(number-1,from);
    m_fqToList.insert(number-1,to);
    m_fqPointsList.insert(number-1,points);
    refresh();

    m_tableWidget->selectionModel()->clearSelection();
    QModelIndex myIndex = m_tableWidget->model()->index( number-1, 0, QModelIndex());
    m_tableWidget->selectionModel()->select(myIndex,QItemSelectionModel::Select);

    myIndex = m_tableWidget->model()->index( number-1, 1, QModelIndex());
    m_tableWidget->selectionModel()->select(myIndex,QItemSelectionModel::Select);
}

void Presets::refresh()
{
    for(int i = 0; i < m_fqFromList.length(); ++i)
    {
        if(!m_isRange)
        {
            m_tableWidget->setItem(i,0, new QTableWidgetItem(m_fqFromList.at(i)));
            m_tableWidget->setItem(i,1, new QTableWidgetItem(m_fqToList.at(i)));
        }else
        {
            qint64 center = (m_fqToList.at(i).toULongLong() + m_fqFromList.at(i).toULongLong())/2;
            qint64 band = (m_fqToList.at(i).toULongLong() - m_fqFromList.at(i).toULongLong())/2;
            m_tableWidget->setItem(i,0, new QTableWidgetItem(QString::number(center)));
            m_tableWidget->setItem(i,1, new QTableWidgetItem(QString::number(band)));
        }
        m_tableWidget->setItem(i,2, new QTableWidgetItem(m_fqPointsList.at(i)));
    }
}

void Presets::on_isRangeChanged (bool isRange)
{
    m_isRange = isRange;
    refresh();
}

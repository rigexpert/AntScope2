#include "printmarkers.h"


PrintMarkers::PrintMarkers(QWidget *parent)
    : QWidget(parent)
{
    QString path = Settings::setIniFile();
    m_settings = new QSettings(path,QSettings::IniFormat);
    initLayout();
}

PrintMarkers::~PrintMarkers()
{
}

void PrintMarkers::initLayout()
{
      createHeader();
      setLayout(&m_layout);
      updateTable();
      emit changeColumns();
}

void PrintMarkers::createHeader()
{
    QMap<int, QString>& mapHeader = MarkersHeaderColumn::headerMap();
    m_headerColumns.clear();

    QString style = "QLabel {color: black; font: 14pt}";
    m_settings->beginGroup("Markers");
    QString buttons = m_settings->value("header", "0,1,2,3,4,5,6,7,8,9").toString();
    m_settings->endGroup();
    QList<QString> list = buttons.split(',');
    list.removeAt(0);
    int column = 0;
    foreach (QString key, list) {
        if (key.isEmpty())
            continue;
        MarkersHeaderColumn data;
        int type = key.toInt();
        data.index = column;
        QLabel* button = new QLabel(this);
        button->setStyleSheet(style);
        button->setAlignment(Qt::AlignCenter);
        button->setProperty("field_type", type);
        button->setText(mapHeader[type]);
        m_layout.addWidget(button, 0 ,column++);
        data.button = button;
        m_headerColumns << data;
    }
}


void PrintMarkers::updateMarkers(int markers, int measurements)
{
    clearTable();

    if (markers == 0) {
        setVisible(false);
        return;
    }
    m_markers = markers;
    m_measurements = measurements;

    QString style = "QLabel {color: black; font: 14pt}";

    for (int i=0; i<m_headerColumns.size(); i++) {
        m_layout.addWidget(m_headerColumns[i].button, 0, i);
        m_headerColumns[i].button->show();
    }

    int rowCount = m_measurements==0 ? 1 : m_measurements;
    int rowIndex = 1;
    for (int i=0; i<m_markers; i++) {
        for (int j=0; j<rowCount; j++) {
            QList<QWidget*> row;
            for (int k=0; k<m_headerColumns.size(); k++) {
                QLabel* label = new QLabel(this);
                label->setStyleSheet(style);
                label->setAlignment(Qt::AlignCenter);

                row << qobject_cast<QLabel*>(label);
                m_layout.addWidget(label, rowIndex, k);
                label->show();
            }
            m_rows << row;
            rowIndex++;
        }
    }
}

void PrintMarkers::updateInfo(QList<QList<QVariant>>& info)
{
    if (info.size() != (m_markers*m_measurements))
        return;

    int rowIndex = 0;
    for (int i=0; i<m_markers; i++) {
        for (int j=0; j<m_measurements; j++) {
            QList<QVariant>& rowInfo = info[rowIndex];
            QList<QWidget*>& rowLabel = m_rows[rowIndex];
            for (int k=0; k<m_headerColumns.size(); k++) {
                if (j != 0 && k == 0)
                    continue;
                QVariant val = rowInfo[k+1];
                int type = m_headerColumns[k].button->property("field_type").toInt();
                QString str = formatText(type, val);
                QLabel* label = qobject_cast<QLabel*>(rowLabel[k]);
                label->setText(str);
            }
            rowIndex++;
        }
    }
    setVisible(true);
    updateTable();
}

QList<int> PrintMarkers::getColumns()
{
    QList<int> list;
    for (int i=0; i<m_headerColumns.size(); i++) {
        list << m_headerColumns[i].button->property("field_type").toInt();
    }
    return list;
}

void PrintMarkers::clearTable(void)
{
    QLayoutItem* item = nullptr;
    while((item=m_layout.takeAt(0)) != nullptr) {
        item->widget()->setVisible(false);
        m_layout.removeItem(item);
    }
    for(int i=0; i<m_rows.size(); i++) {
        QList<QWidget*> row = m_rows[i];
        for (int j=0; j<row.size(); j++) {
            delete row[j];
        }
        row.clear();
    }
    m_rows.clear();
}

void PrintMarkers::updateTable()
{
    adjustSize();
}

QString PrintMarkers::formatText(int type, QVariant v)
{
    if (v.type() == QVariant::Invalid || v.toDouble() == DBL_MAX)
        return "";

    QString str;
    switch (type) {
    case MarkersHeaderColumn::fieldDelete:
        break;
    case MarkersHeaderColumn::fieldNum:
        str = QString::number(v.toInt());
        break;
    case MarkersHeaderColumn::fieldSerie:
        str = QString::number(v.toInt());
        break;
    case MarkersHeaderColumn::fieldZ:
        str = v.toString();
        break;
    case MarkersHeaderColumn::fieldZpar:
        str = v.toString();
        break;
    default:
        str = QString::number(v.toDouble(),'f', 2);
        break;
    }
    return str;
}

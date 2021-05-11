#include "markerspopup.h"
#include "mainwindow.h"

QMap<int, QString> MarkersHeaderColumn::m_mapHeader;

MarkersPopUp::MarkersPopUp(QWidget *parent) : QWidget(parent),
      m_durability(2000),
      m_hiding(true),
      m_x(0),
      m_y(0),
      m_biasX(0),
      m_biasY(0),
      m_mainX(0),
      m_mainY(0),
      m_mainBiasX(0),
      m_mainBiasY(0),
      m_bgColor(0,0,0,180),
      m_penColor(255,255,255,180),
      m_textColor("white")
{
      setWindowFlags(Qt::FramelessWindowHint |        // Отключаем оформление окна
                     Qt::Tool |                       // Отменяем показ в качестве отдельного окна
                     Qt::WindowStaysOnTopHint);       // Устанавливаем поверх всех окон
      setAttribute(Qt::WA_TranslucentBackground);     // Указываем, что фон будет прозрачным
      setAttribute(Qt::WA_ShowWithoutActivating);     // При показе, виджет не получается фокуса автоматически

      animation.setTargetObject(this);                // Устанавливаем целевой объект анимации
      animation.setPropertyName("popupOpacity");      // Устанавливаем анимируемое свойство
      connect(&animation, &QAbstractAnimation::finished, this, &MarkersPopUp::hide); /* Подключаем сигнал окончания
                                                                               * анимации к слоту скрытия                                                                               * */
      QString path = Settings::setIniFile();
      m_settings = new QSettings(path,QSettings::IniFormat);

      m_timer = new QTimer();
      connect(m_timer, &QTimer::timeout, this, &MarkersPopUp::hideAnimation);

      initLayout();
}

void MarkersPopUp::setName(QString name)
{
    m_name = name;
    m_settings->beginGroup(m_name);
    if(m_name == "Markers")
    {
        m_x = m_settings->value("x",861).toInt();
        m_y = m_settings->value("y",127).toInt();
        m_mainX = m_settings->value("mainX",169).toInt();
        m_mainY = m_settings->value("mainY",101).toInt();
        m_mainBiasX = m_settings->value("mainBiasX",692).toInt();
        m_mainBiasY = m_settings->value("mainBiasY",26).toInt();
    }
    int widthDesc = qApp->desktop()->width();
    int heightDesc = qApp->desktop()->height();
    if((m_x > widthDesc - width()) || (m_x < 0))
    {
        m_x = 500;
    }
    if( (m_y > heightDesc - height()) || (m_y < 0))
    {
        m_y = 500;
    }

    m_settings->endGroup();

    setGeometry(m_x,m_y,width(),height());
}

MarkersPopUp::~MarkersPopUp()
{
    m_settings->beginGroup(m_name);
    m_settings->setValue("x",m_x);
    m_settings->setValue("y",m_y);
    m_settings->setValue("mainX",m_mainX);
    m_settings->setValue("mainY",m_mainY);
    m_settings->setValue("mainBiasX",m_mainBiasX);
    m_settings->setValue("mainBiasY",m_mainBiasY);
    QList<int> buttons = getColumns();
    QString header;
    for (int i=0; i<buttons.size(); i++)
        header += QString::number(buttons[i]) + ",";
    header.remove(header.length()-1, 1);
    m_settings->setValue("header", header);
    m_settings->endGroup();

    delete m_settings;
}

void MarkersPopUp::initLayout()
{
      //fillHeaderMap();
      createHeader();
      setLayout(&m_layout);
      updateTable();
}

void MarkersPopUp::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect roundedRect;
    roundedRect.setX(rect().x() + 5);
    roundedRect.setY(rect().y() + 5);
    roundedRect.setWidth(rect().width() - 10);
    roundedRect.setHeight(rect().height() - 10);

    painter.setBrush(QBrush(m_bgColor));
    painter.setPen(m_penColor);

    painter.drawRoundedRect(roundedRect, 5, 5);
}


void MarkersPopUp::on_remove()
{
    QString str = sender()->objectName();
    str.remove(0,2);
    int markerIndex = str.toInt();
    emit removeMarker(markerIndex);
}

QList <QStringList> MarkersPopUp::getPopupList()
{ // print support

    QList <QStringList> retList;
    QStringList tempList;

    // TODO
//    for(int i = 0; i < m_measurementsList.length(); ++i)
//    {
//        tempList.append(m_markersList.at(i));
//        tempList.append(m_measurementsList.at(i));
//        tempList.append(m_fqList.at(i));
//        tempList.append(m_swrList.at(i));
//        tempList.append(m_rlList.at(i));
//        tempList.append(m_zList.at(i));
//        tempList.append(m_phaseList.at(i));
//        retList.append(tempList);
//        tempList.clear();
//    }
    return retList;
}

void MarkersPopUp::show()
{
    if (m_markers == 0 || m_measurements == 0)
        return;

    setWindowOpacity(0.0);

    animation.setDuration(150);
    animation.setStartValue(0.0);
    animation.setEndValue(1.0);

    setGeometry(QCursor::pos().x() - 400,
                QCursor::pos().y() - 100,
                width(),
                height());

    QWidget::show();

    animation.start();
    if(m_hiding)
    {
        m_timer->start(m_durability);
    }
}

void MarkersPopUp::focusShow()
{
    //qDebug() << "MarkersPopUp::focusShow()" << m_menuVisible;
    QWidget::show();
}

void MarkersPopUp::focusHide()
{
    //qDebug() << "MarkersPopUp::focusHide()" << m_menuVisible;
    if (m_menuVisible) {
        setVisible(true);
        return;
    }
    QWidget::hide();
}

void MarkersPopUp::hideAnimation()
{
    m_timer->stop();
    animation.setDuration(1000);
    animation.setStartValue(1.0);
    animation.setEndValue(0.0);
    animation.start();
}

void MarkersPopUp::hide()
{
    if(getPopupOpacity() == 0.0)
    {
        QWidget::hide();
    }
}

void MarkersPopUp::setPopupOpacity(float opacity)
{
    popupOpacity = opacity;

    setWindowOpacity(opacity);
}

float MarkersPopUp::getPopupOpacity() const
{
    return popupOpacity;
}

void MarkersPopUp::mousePressEvent(QMouseEvent * event)
{
    m_biasX = event->pos().x();
    m_biasY = event->pos().y();
}

void MarkersPopUp::mouseMoveEvent(QMouseEvent * )
{
    m_x = QCursor::pos().x() - m_biasX;
    m_y = QCursor::pos().y() - m_biasY;
    setGeometry(m_x,
                m_y,
                width(),
                height());
    m_mainBiasX = m_x - m_mainX;
    m_mainBiasY = m_y - m_mainY;
}

void MarkersPopUp::MainWindowPos(int x, int y)
{
    m_mainX = x;
    m_mainY = y;

    m_x = x + m_mainBiasX;
    m_y = y + m_mainBiasY;
    setGeometry(m_x,
                m_y,
                width(),
                height());
}

void MarkersPopUp::setPosition(int x, int y)
{
    m_x = x;
    m_y = y;
    setGeometry(m_x,
                m_y,
                width(),
                height());
}

void MarkersPopUp::setTextColor(QString color)
{
    m_textColor = color;
}

void MarkersPopUp::on_translate()
{
//    m_removeLabel.setText(tr("Del"));
//    m_numberLabel.setText(tr("Marker"));
//    m_measurementLabel.setText("#");
//    m_fqLabel.setText(tr("Fq"));
//    m_swrLabel.setText(tr("SWR"));
//    m_rlLabel.setText(tr("RL"));
//    m_zLabel.setText(tr("Z"));
//    m_phaseLabel.setText(tr("Phase"));
}


void MarkersPopUp::createMenu(MarkersHeaderColumn &column)
{
    column.menu = new QMenu(column.button);
    QMapIterator<int, QString> it(MarkersHeaderColumn::headerMap());
    while (it.hasNext()) {
        it.next();
        if (it.key() > MarkersHeaderColumn::fieldFQ) {
            QAction* action = column.menu->addAction(it.value());
            action->setData(it.key());
        }
    }
    connect(column.menu, &QMenu::aboutToShow, this, [=](){
        m_menuVisible = true;
        setVisible(true);
    });
    connect(column.menu, &QMenu::aboutToHide, this, [=](){
        m_menuVisible = false;
        extern MainWindow* g_mainWindow;
        QApplication::setActiveWindow(g_mainWindow);
    });
    connect(column.menu, &QMenu::triggered, this, [this, column](QAction* act) {
       m_menuVisible = true;
       int index = act->data().toInt();
       if (index == MarkersHeaderColumn::fieldInsert) {
            // inset column
           if (m_headerColumns.size() <= MAX_BUTTONS_NUM) {
               QList<int> buttons = getColumns();
               buttons.insert(column.index+1, MarkersHeaderColumn::fieldSWR);
               QString header;
               for (int i=0; i<buttons.size(); i++)
                   header += QString::number(buttons[i]) + ",";
               m_settings->beginGroup("Markers");
               m_settings->setValue("header", header);
               m_settings->endGroup();
               m_settings->sync();
               clearTable();
               createHeader();
               updateMarkers(m_markers, m_measurements);
           }
       } else  if (index == MarkersHeaderColumn::fieldRemove) {
            // remove column
           if (m_headerColumns.size() > MIN_BUTTONS_NUM) {
               QList<int> buttons = getColumns();
               buttons.removeAt(column.index);
               QString header;
               for (int i=0; i<buttons.size(); i++)
                   header += QString::number(buttons[i]) + ",";
               m_settings->beginGroup("Markers");
               m_settings->setValue("header", header);
               m_settings->endGroup();
               m_settings->sync();
               clearTable();
               createHeader();
               updateMarkers(m_markers, m_measurements);
           }
       } else {
           QString title = act->text();
           ((QToolButton*)(column.button))->setText(title);
           column.button->setProperty("field_type", index);
           QList<int> buttons = getColumns();
           QString header;
           for (int i=0; i<buttons.size(); i++)
               header += QString::number(buttons[i]) + ",";
           m_settings->beginGroup("Markers");
           m_settings->setValue("header", header);
           m_settings->endGroup();
           m_settings->sync();
        }
       setVisible(true);
       m_menuVisible = false;
       emit changeColumns();
    });
}

void MarkersPopUp::createHeader()
{
    QMap<int, QString>& mapHeader = MarkersHeaderColumn::headerMap();
    m_headerColumns.clear();

    m_settings->beginGroup("Markers");
    QString buttons = m_settings->value("header", "0,1,2,3,4,5,6,7,8,9").toString();
    m_settings->endGroup();
    QList<QString> list = buttons.split(',');
    int column = 0;
    foreach (QString key, list) {
        if (key.isEmpty())
            continue;
        MarkersHeaderColumn data;
        int type = key.toInt();
        data.index = column;
        QToolButton* button = new QToolButton(this);
        data.button = button;
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        button->setProperty("field_type", type);
        if (type > MarkersHeaderColumn::fieldFQ) {
            button->setPopupMode(QToolButton::MenuButtonPopup);
            createMenu(data);
            button->setMenu(data.menu);
        }
        button->setText(mapHeader[type]);
        m_layout.addWidget(data.button, 0 ,column++, Qt::AlignHCenter);
        m_headerColumns << data;
    }
}


QList<int> MarkersPopUp::getColumns()
{
    QList<int> list;
    for (int i=0; i<m_headerColumns.size(); i++) {
        list << m_headerColumns[i].button->property("field_type").toInt();
    }
    return list;
}

void MarkersPopUp::updateMarkers(int markers, int measurements)
{
    clearTable();

    if (markers == 0) {
        hide();
        return;
    }
    m_markers = markers;
    m_measurements = measurements;

    for (int i=0; i<m_headerColumns.size(); i++) {
        m_layout.addWidget(m_headerColumns[i].button, 0, i);
        m_headerColumns[i].button->show();
    }

    int rowCount = m_measurements==0 ? 1 : m_measurements;
    int rowIndex = 1;
    for (int i=0; i<m_markers; i++) {
        QToolButton* button = new QToolButton(this);
        QString str = "RM" + QString::number(i);
        button->setObjectName(str);
        button->setMaximumWidth(20);
        button->setText("X");
        connect(button, &QToolButton::clicked, this, &MarkersPopUp::on_remove);
        m_layout.addWidget(button, rowIndex, 0);
        for (int j=0; j<rowCount; j++) {
            QList<QWidget*> row;
            row << qobject_cast<QLabel*>(button);
            for (int k=1; k<m_headerColumns.size(); k++) {
                QLabel* label = new QLabel(this);
                label->setAlignment(Qt::AlignCenter);
                row << qobject_cast<QLabel*>(label);
                m_layout.addWidget(label, rowIndex, k);
                label->show();
            }
            m_rows << row;
            rowIndex++;
        }
    }

    updateTable();
}

void MarkersPopUp::updateInfo(QList<QList<QVariant>>& info)
{
    if (info.size() != (m_markers*m_measurements))
        return;

    int rowIndex = 0;
    for (int i=0; i<m_markers; i++) {
        for (int j=0; j<m_measurements; j++) {
            QList<QVariant>& rowInfo = info[rowIndex];
            QList<QWidget*>& rowLabel = m_rows[rowIndex];
            for (int k=1; k<m_headerColumns.size(); k++) { // ignore fieldDelete
                if (j != 0 && k == MarkersHeaderColumn::fieldNum)
                    continue;
                QVariant val = rowInfo[k];
                int type = m_headerColumns[k].button->property("field_type").toInt();
                QString str = formatText(type, val);
                QLabel* label = qobject_cast<QLabel*>(rowLabel[k]);
                label->setText(str);
            }
            rowIndex++;
        }
    }
}

void MarkersPopUp::clearTable(void)
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

void MarkersPopUp::updateTable()
{
    adjustSize();
}

QString MarkersPopUp::formatText(int type, QVariant v)
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

///////////////////////////////////////////////////////
QMap<int, QString>& MarkersHeaderColumn::headerMap()
{
    if (m_mapHeader.isEmpty()) {
        int i = MarkersHeaderColumn::fieldDelete;
        m_mapHeader.insert(i++, "Del");
        m_mapHeader.insert(i++, "Marker");
        m_mapHeader.insert(i++, " # ");
        m_mapHeader.insert(i++, "FQ, kHz");
        m_mapHeader.insert(i++, "SWR");        // SWR   - коэф. стоячей волны
        m_mapHeader.insert(i++, "RL, dB");     // RL   - возвратные потери
        m_mapHeader.insert(i++, "Phase°");     // phase - фаза
        m_mapHeader.insert(i++, "R, Ohm");     // R    - активное сопротивление (последовательная модель)
        m_mapHeader.insert(i++, "X, Ohm");     // X    - реактивное сопротивление (последовательная модель)
        m_mapHeader.insert(i++, "Z, Ohm");     // Z     - импеданс
        m_mapHeader.insert(i++, "L, nH");      // L    - индуктивность (последовательная модель)
        m_mapHeader.insert(i++, "C, pF");      // C    - ёмкость (последовательная модель)
        m_mapHeader.insert(i++, "rho");        // rho   - магнитуда
        m_mapHeader.insert(i++, "|Z|, Ohm");   // |Z|  - модуль импеданса
        m_mapHeader.insert(i++, "R||, Ohm");   // R||  - активное сопротивление (параллельная модель)
        m_mapHeader.insert(i++, "X||, Ohm");   // X||  - реактивное сопротивление (параллельная модель)
        m_mapHeader.insert(i++, "Z||, Ohm");   // Z||   - импеданс  (параллельная модель)
        m_mapHeader.insert(i++, "L||, nH");    // L||  - индуктивность (параллельная модель)
        m_mapHeader.insert(i++, "C||, pF");    // C||  - ёмкость (параллельная модель)

        m_mapHeader.insert(MarkersHeaderColumn::fieldInsert, "Insert column");
        m_mapHeader.insert(MarkersHeaderColumn::fieldRemove, "Remove column");
    }
    return m_mapHeader;
}

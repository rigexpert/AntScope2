#include "markerspopup.h"

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
                                                                               * анимации к слоту скрытия
                                                                               * */


      m_layout.addWidget(&m_removeLabel,0,0);
      m_layout.addWidget(&m_numberLabel,0,1);
      m_layout.addWidget(&m_measurementLabel,0,2);
      m_layout.addWidget(&m_fqLabel,0,3);
      m_layout.addWidget(&m_swrLabel,0,4);
      m_layout.addWidget(&m_rlLabel,0,5);
      m_layout.addWidget(&m_zLabel,0,6);
      m_layout.addWidget(&m_phaseLabel,0,7);
      setLayout(&m_layout);

      on_translate();

      m_timer = new QTimer();
      connect(m_timer, &QTimer::timeout, this, &MarkersPopUp::hideAnimation);

      QString path = Settings::setIniFile();
      m_settings = new QSettings(path,QSettings::IniFormat);
      updateTable();
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
    m_settings->endGroup();
    delete m_settings;
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

void MarkersPopUp::addRowText( int markerNumber,
                               QVector<int> *measurement,
                               QVector<double> *fq,
                               QVector<double> *swr,
                               QVector<double> *rl,
                               QVector<QString> *z,
                               QVector<double> *phase)
{
    QPushButton* button = new QPushButton();
    int row = fq->length() == 0 ? 1 : fq->length();
    m_layout.addWidget(button, row * m_buttonsObjList.length() + 1, 0);
    //m_layout.addWidget(button, fq->length()*(m_buttonsObjList.length()) + 1, 0);
    //m_layout.addWidget(button, markerNumber + 1, 0);
    QString str = "RM" + QString::number(markerNumber);
    button->setObjectName(str);
    button->setText("X");
    button->setMaximumWidth(20);
    connect(button, SIGNAL(clicked()), this, SLOT(on_remove()));
    m_buttonsObjList.append(button);

    QLabel* label = new QLabel();
    label->setText(QString::number(markerNumber+1));
    m_layout.addWidget(label, row * m_markersObjList.length() + 1, 1);
    //m_layout.addWidget(label, fq->length()*(m_markersObjList.length()) + 1, 1);
    //m_layout.addWidget(label, markerNumber + 1, 1);
    m_markersObjList.append(label);

    for(int i = 0; i < measurement->length(); ++i)
    {
        QLabel* label = new QLabel();
        label->setText(QString::number(measurement->at(measurement->length()- i-1)));
        int t = measurement->length()*(markerNumber) + i + 1;
        m_layout.addWidget(label, t, 2);
        m_measurementsObjList.append(label);
        m_measurementsList.append(QString::number(measurement->at(measurement->length()- i-1)));
        m_markersList.append(QString::number(markerNumber+1));
    }

    for(int i = 0; i < fq->length(); ++i)
    {
        QLabel* label = new QLabel();
        label->setText(QString::number(fq->at(i)));
        int t = fq->length()*(markerNumber) + i + 1;
        m_layout.addWidget(label, t, 3);
        m_fqObjList.append(label);
        m_fqList.append(QString::number(fq->at(i)));
    }

    for(int i = 0; i < swr->length(); ++i)
    {
        QLabel* label = new QLabel();
        label->setText(QString::number(swr->at(i),'f', 2));
        m_layout.addWidget(label, swr->length()*(markerNumber) + i + 1, 4);
        m_swrObjList.append(label);
        m_swrList.append(QString::number(swr->at(i),'f', 2));
    }

    for(int i = 0; i < rl->length(); ++i)
    {
        QLabel* label = new QLabel();
        label->setText(QString::number(rl->at(i),'f', 2));
        m_layout.addWidget(label, rl->length()*(markerNumber) + i + 1, 5);
        m_rlObjList.append(label);
        m_rlList.append(QString::number(rl->at(i),'f', 2));
    }

    for(int i = 0; i < z->length(); ++i)
    {
        QLabel* label = new QLabel();
        label->setText(z->at(i));
        m_layout.addWidget(label, z->length()*(markerNumber) + i + 1, 6);
        m_zObjList.append(label);
        m_zList.append(z->at(i));
    }

    for(int i = 0; i < phase->length(); ++i)
    {
        QLabel* label = new QLabel();
        label->setText(QString::number(phase->at(i),'f', 2));
        m_layout.addWidget(label, phase->length()*(markerNumber) + i + 1, 7);
        m_phaseObjList.append(label);
        m_phaseList.append(QString::number(phase->at(i),'f', 2));
    }

    updateTable();
}

void MarkersPopUp::on_remove()
{
    QPushButton *button = (QPushButton*) sender();
    QString str = button->objectName();
    str.remove(0,2);
    emit removeMarker(str.toInt());
}

void MarkersPopUp::clearTable(void)
{
    m_markersList.clear();
    m_measurementsList.clear();
    m_fqList.clear();
    m_swrList.clear();
    m_rlList.clear();
    m_zList.clear();
    m_phaseList.clear();

    for(int i = 0; i < m_buttonsObjList.length(); ++i)
    {
        delete m_buttonsObjList[i];
    }
    for(int i = 0; i < m_markersObjList.length(); ++i)
    {
        delete m_markersObjList[i];
    }
    for(int i = 0; i < m_measurementsObjList.length(); ++i)
    {
        delete m_measurementsObjList[i];
    }
    for(int i = 0; i < m_fqObjList.length(); ++i)
    {
        delete m_fqObjList[i];
    }
    for(int i = 0; i < m_swrObjList.length(); ++i)
    {
        delete m_swrObjList[i];
    }
    for(int i = 0; i < m_rlObjList.length(); ++i)
    {
        delete m_rlObjList[i];
    }
    for(int i = 0; i < m_zObjList.length(); ++i)
    {
        delete m_zObjList[i];
    }
    for(int i = 0; i < m_phaseObjList.length(); ++i)
    {
        delete m_phaseObjList[i];
    }

    m_buttonsObjList.clear();
    m_markersObjList.clear();
    m_measurementsObjList.clear();
    m_fqObjList.clear();
    m_swrObjList.clear();
    m_rlObjList.clear();
    m_zObjList.clear();
    m_phaseObjList.clear();
}

void MarkersPopUp::updateTable()
{
    adjustSize();
}

QString MarkersPopUp::getPopupText()
{
    return m_numberLabel.text();
}

QList <QStringList> MarkersPopUp::getPopupList()
{
    QList <QStringList> retList;
    QStringList tempList;

    for(int i = 0; i < m_measurementsList.length(); ++i)
    {
        tempList.append(m_markersList.at(i));
        tempList.append(m_measurementsList.at(i));
        tempList.append(m_fqList.at(i));
        tempList.append(m_swrList.at(i));
        tempList.append(m_rlList.at(i));
        tempList.append(m_zList.at(i));
        tempList.append(m_phaseList.at(i));
        retList.append(tempList);
        tempList.clear();
    }
    return retList;
}

void MarkersPopUp::show()
{
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
    QWidget::show();
}

void MarkersPopUp::focusHide()
{
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
    m_removeLabel.setText(tr("Del"));
    m_numberLabel.setText(tr("Marker"));
    m_measurementLabel.setText("#");
    m_fqLabel.setText(tr("Fq"));
    m_swrLabel.setText(tr("SWR"));
    m_rlLabel.setText(tr("RL"));
    m_zLabel.setText(tr("Z"));
    m_phaseLabel.setText(tr("Phase"));
}

#include "popup.h"
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <mainwindow.h>

PopUp::PopUp(QWidget *parent) : QWidget(parent),
    m_bgColor(0,0,0,180),
    m_penColor(255,155,255,180),
    m_textColor("white"),
    m_popupOpacity(0.1f),
    m_durability(2000),
    m_hiding(true),
    m_x(850),
    m_y(130),
    m_biasX(0),
    m_biasY(0),
    m_mainX(177),
    m_mainY(131),
    m_mainBiasX(0),
    m_mainBiasY(0)
{
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::Tool |
                   Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    animation.setTargetObject(this);
    animation.setPropertyName("popupOpacity");
    connect(&animation, &QAbstractAnimation::finished, this, &PopUp::hide);

    label.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label.setStyleSheet("QLabel { color : " + m_textColor +
                        "margin-top: 6px;"
                        "margin-bottom: 6px;"
                        "margin-left: 10px;"
                        "margin-right: 10px; }");

    layout.addWidget(&label);
    setLayout(&layout);

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &PopUp::hideAnimation);

    QString path = Settings::setIniFile();
    m_settings = new QSettings(path,QSettings::IniFormat);
}

void PopUp::setName(QString name)
{
    m_name = name;
    m_settings->beginGroup(m_name);
    if(m_name == "Hint")
    {
        m_x = m_settings->value("x",185).toInt();
        m_y = m_settings->value("y",642).toInt();
        m_mainX = m_settings->value("mainX",169).toInt();
        m_mainY = m_settings->value("mainY",101).toInt();
        m_mainBiasX = m_settings->value("mainBiasX",16).toInt();
        m_mainBiasY = m_settings->value("mainBiasY",541).toInt();
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

PopUp::~PopUp()
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

void PopUp::paintEvent(QPaintEvent *event)
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

void PopUp::setPopupText(const QString &text)
{
    label.setText(text);
    adjustSize();
}

QString PopUp::getPopupText()
{
    return label.text();
}

void PopUp::show()
{
    setWindowOpacity(0.0);

    animation.setDuration(150);
    animation.setStartValue(0.0);
    animation.setEndValue(1.0);

    QWidget::show();

    animation.start();
    if(m_hiding)
    {
        timer->start(m_durability);
    }
}

void PopUp::focusShow()
{
    QWidget::show();
}

void PopUp::focusHide()
{
    QWidget::hide();
}

void PopUp::hideAnimation()
{
    timer->stop();
    animation.setDuration(1000);
    animation.setStartValue(1.0);
    animation.setEndValue(0.0);
    animation.start();
}

void PopUp::hide()
{
    if(getPopupOpacity() == 0.0)
    {
        QWidget::hide();
    }
}

void PopUp::setPopupOpacity(float opacity)
{
    m_popupOpacity = opacity;

    setWindowOpacity(opacity);
}

float PopUp::getPopupOpacity() const
{
    return m_popupOpacity;
}

void PopUp::mousePressEvent(QMouseEvent * event)
{
    m_biasX = event->pos().x();
    m_biasY = event->pos().y();
}

void PopUp::mouseMoveEvent(QMouseEvent * )
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

void PopUp::MainWindowPos(int x, int y)
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

void PopUp::setPosition(int x, int y)
{
    m_x = x;
    m_y = y;
    setGeometry(m_x,
                m_y,
                width(),
                height());
}



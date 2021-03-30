#include "marqueelabel.h"
#include <QPainter>
#include <QObject>
#include <QFile>
#include <QJsonParseError>
#include <QJsonArray>
#include <QDebug>

MarqueeLabel::MarqueeLabel(QWidget *parent) 
    : QLabel(parent), m_px(0), m_py(10), m_speed(1), m_direction(RightToLeft)
{	
    connect(&m_timer, &QTimer::timeout, this, [=](){ repaint(); });
    //m_timer.start(10);
}

MarqueeLabel::~MarqueeLabel()
{
	
}

void MarqueeLabel::show()
{	
	QLabel::show();
}

void MarqueeLabel::setAlignment(Qt::Alignment _align)
{
	m_align = _align;
	updateCoordinates();
	QLabel::setAlignment(_align);
}

void MarqueeLabel::paintEvent(QPaintEvent *event)
{
    if (m_strings.isEmpty() || m_waitForDelay) {
        QLabel::paintEvent(event);
        return;
    }
    if (m_current >= m_strings.size())
        m_current = 0;

	QPainter paint(this);
    QColor color = m_strings[m_current].color();
    if (color.isValid() && color != QColor(Qt::transparent))
        paint.setPen(m_strings[m_current].color());
	if(m_direction==RightToLeft)
	{
		m_px -= m_speed;
        if(m_px <= (-m_textLength)) {
           //m_px = (-m_textLength);
           m_waitForDelay = true;
           m_timer.stop();
           QTimer::singleShot(m_strings[m_current].delay()*1000, this, [=]() { next(); });
        }
	}
	else
	{
		m_px += m_speed;
        if(m_px >= width()) {
            //m_px = width();
            m_waitForDelay = true;
            m_timer.stop();
            QTimer::singleShot(m_strings[m_current].delay()*1000, this, [=]() { next(); });
        }
	}
    paint.drawText(m_px, 0, width()-m_px, height(), Qt::AlignLeft|Qt::AlignVCenter ,text());

//    QRectF rf(m_px/2.0, 0, width()-m_px/2.0, height());
//    paint.drawText(rf, Qt::AlignLeft|Qt::AlignVCenter ,text());

    paint.translate(m_px,0);
}

void MarqueeLabel::next()
{
    m_current++;
    if (m_current >= m_strings.size()) {
        m_timer.stop();
        repeate(m_repeateDelaySec);
        m_speed = m_strings[m_current].speed();
        return;
    } else {
        setText(m_strings[m_current].text());
        m_speed = m_strings[m_current].speed();
        updateCoordinates();
        setDirection(m_direction);
        m_px = (m_direction==RightToLeft) ? width() : (- m_textLength);
    }
    if (!m_strings[m_current].link().isEmpty()) {
        setCursor(Qt::PointingHandCursor);
    } else{
        setCursor(Qt::ArrowCursor);
    }
    m_timer.start(10);
    m_waitForDelay = false;
}

void MarqueeLabel::repeate(int delay)
{
    if (m_strings.isEmpty()) {
        m_timer.stop();
        return;
    }
    //QTimer::singleShot(delay*1000, this, [=](){
        m_current = 0;
        setText(m_strings[m_current].text());
        m_speed = m_strings[m_current].speed();
        updateCoordinates();
        m_px = width();
        if (!m_strings[m_current].link().isEmpty())
            setCursor(Qt::PointingHandCursor);
        else
            setCursor(Qt::ArrowCursor);
        m_timer.start(10);
        m_waitForDelay = false;
    //});
}

void MarqueeLabel::resizeEvent(QResizeEvent *evt)
{	
	updateCoordinates();
	QLabel::resizeEvent(evt);
}

void MarqueeLabel::updateCoordinates()
{
	switch(m_align)
	{
		case Qt::AlignTop:
			m_py = 10;
			break;
		case Qt::AlignBottom:
			m_py = height()-10;
			break;
		case Qt::AlignVCenter:
			m_py = height()/2;
			break;
	}
    m_fontSize = font().pointSize()/2;
	m_textLength = fontMetrics().width(text());
}

void MarqueeLabel::setSpeed(int _speed)
{
	m_speed = _speed;
}

int MarqueeLabel::speed()
{
	return m_speed;
}

void MarqueeLabel::setDirection(int _direction)
{
	m_direction = _direction;
	if (m_direction == RightToLeft)
		m_px = width() - m_textLength;
	else
		m_px = 0;
    //repaint();
}

void MarqueeLabel::setStrings(QList<MarqueeString>& list)
{
    m_strings = list;

    m_timer.stop();
    clear();
    m_current = 0;
    setText(m_strings[m_current].text());
    updateCoordinates();
    setDirection(m_direction);
    m_timer.start(10);
}

void MarqueeLabel::mousePressEvent ( QMouseEvent *  )
{
    if (m_waitForDelay || m_strings.isEmpty() || m_current >= m_strings.size())
        return;
    if (!m_strings[m_current].link().isEmpty())
        emit clicked(m_strings[m_current].link());
}

bool MarqueeLabel::load(QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();
    return load(data);
}

bool MarqueeLabel::load(QByteArray& data)
{
    QJsonParseError parseError;
    QJsonDocument doc(QJsonDocument::fromJson(data, &parseError));
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "MarqueeLabel::load()" << parseError.errorString() << parseError.offset;
        return false;
    }

    QJsonObject mainObj = doc.object();
    QJsonArray array = mainObj["messages"].toArray();
    for(int i=0; i<array.size(); ++i)
    {
        QJsonObject dataObject = array[i].toObject();
        MarqueeString string(dataObject);
        m_strings << string;
    }
    repeate(0);
    return true;
}


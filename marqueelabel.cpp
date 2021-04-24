#include "marqueelabel.h"
#include <QPainter>
#include <QObject>
#include <QFile>
#include <QJsonParseError>
#include <QJsonArray>
#include <QDebug>
#include "analyzerparameters.h"

MarqueeLabel::MarqueeLabel(QWidget *parent) 
    : QLabel(parent), m_downloader(nullptr), m_px(0), m_py(10), m_speed(1), m_direction(RightToLeft)
{	
    connect(&m_timer, &QTimer::timeout, this, [=](){ repaint(); });
}

MarqueeLabel::~MarqueeLabel()
{
    delete m_downloader;
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
        setText("");
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
        setText("");
        repeate(m_repeateDelaySec);
        return;
    } else {
        setText(m_strings[m_current].text());
        m_speed = m_strings[m_current].speed();
        updateCoordinates();
        setDirection(m_direction);
        m_px = (m_direction==RightToLeft) ? width() : 0;//(- m_textLength);
    }
    if (!m_strings[m_current].link().isEmpty()) {
        setCursor(Qt::PointingHandCursor);
    } else{
        setCursor(Qt::ArrowCursor);
    }
    m_timer.start(SPEED_TIMER);
    m_waitForDelay = false;
}

void MarqueeLabel::repeate(int delay)
{
    if (m_strings.isEmpty()) {
        m_timer.stop();
        return;
    }
    QTimer::singleShot(delay*1000, this, [=](){
        m_current = 0;
        setText(m_strings[m_current].text());
        m_speed = m_strings[m_current].speed();
        updateCoordinates();
        m_px = width();
        if (!m_strings[m_current].link().isEmpty())
            setCursor(Qt::PointingHandCursor);
        else
            setCursor(Qt::ArrowCursor);
        m_timer.start(SPEED_TIMER);
        m_waitForDelay = false;
    });
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
        m_px = width();// - m_textLength;
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
    m_timer.start(SPEED_TIMER);
}

void MarqueeLabel::addStrings(QList<MarqueeString>& list)
{
    for (int i=0; i<list.size(); i++) {
        m_strings.append(list.at(i));
    }
    m_timer.stop();
    clear();
    m_current = 0;
    setText(m_strings[m_current].text());
    updateCoordinates();
    setDirection(m_direction);
    m_timer.start(SPEED_TIMER);
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
        MarqueeString mstring(dataObject);
        if (mstring.enddate().isValid() && QDate::currentDate() > mstring.enddate()) {
            qDebug() << "JSON expired";
            continue;
        }
        if (mstring.keywords().isEmpty()) {
            m_strings << mstring;
        } else {
            AnalyzerParameters* analyzer = AnalyzerParameters::current();
            if (analyzer != nullptr) {
                QMapIterator<QString, QString> i(mstring.keywords());
                while (i.hasNext()) {
                    i.next();
                    if (i.key()=="type" && analyzer->name().contains(i.value())) {
                        m_strings << mstring;
                        break;
                    }
                    if (i.key()=="sn" && analyzer->serilal().contains(i.value())) {
                        m_strings << mstring;
                        break;
                    }
                }
            } else {
                m_strings << mstring;
            }
        }
    }
    repeate(0);
    return true;
}

void MarqueeLabel::request()
{
    if(m_downloader == nullptr)
    {
        m_downloader = new Downloader(this);
        connect(m_downloader, &Downloader::downloadInfoComplete, this, &MarqueeLabel::on_downloadInfoComplete);
        connect(m_downloader, &Downloader::downloadFileComplete, this, &MarqueeLabel::on_downloadFileComplete);
    }

    QString url = "https://www.rigexpert.com/get.php?part=asscroll&model=";
#ifndef NEW_ANALYZER
    url += names[m_analyzerModel].toLower().remove(" ").remove("-");
#else
    QString name = AnalyzerParameters::getName();
    url += name.toLower().remove(" ").remove("-");
#endif
    url += "&sn=" + AnalyzerParameters::getSerial();
    url += "&os=" + QSysInfo::prettyProductName().replace(" ", "-").toLower();
    url += "&cpu=" + QSysInfo::currentCpuArchitecture();
    url += "&lang=" + QLocale::languageToString(QLocale::system().language());
    url += "&sw=" + QString(ANTSCOPE2VER);

    m_downloader->startDownloadInfo(QUrl(url));
}

void MarqueeLabel::on_downloadInfoComplete()
{
    m_downloader->startDownloadFw();
}

void MarqueeLabel::on_downloadFileComplete()
{
    QByteArray arr = m_downloader->file();
    if (load(arr)) {
        show();
    }
}


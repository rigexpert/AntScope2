#include "marqueelabel.h"
#include <QPainter>
#include <QObject>
#include <QFile>
#include <QJsonParseError>
#include <QJsonArray>
#include <QDebug>
#include "analyzerparameters.h"
#include "settings.h"

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
        //setText("");
        QLabel::paintEvent(event);
        return;
    }
    if (m_current >= m_strings.size()) {
        m_current = 0;
    }
	QPainter paint(this);

// Huck. High DPI Scaling 125%
//    QFont font = paint.font();
//    font.setPointSizeF(font.pointSizeF()*0.75);
//    paint.setFont(font);

    QColor color = m_strings[m_current].color();
    if (color.isValid() && color != QColor(Qt::transparent))
        paint.setPen(m_strings[m_current].color());
	if(m_direction==RightToLeft)
	{
		m_px -= m_speed;
        if ((m_px >= 0 && m_px < m_speed) || (m_px <= 0 && m_px > m_speed)) {
            QRectF boundingRect = paint.boundingRect(0, 0, 3080, 20, Qt::AlignLeft|Qt::AlignHCenter, text());
            m_textLength = boundingRect.width();

            m_px = 0;
            m_waitForDelay = true;
            m_timer.stop();
            int ww = m_strings[m_current].waitTime()*1000;
            QTimer::singleShot(ww, this, [=]() {
                m_waitForDelay = false;
                m_timer.start(m_speedTimerMs);
            });
        } else  if(m_px <= (-m_textLength)) {
           m_waitForDelay = true;
           int delay_ms = m_strings[m_current].delay()*1000;
           m_timer.stop();
           setText("");
           QTimer::singleShot(delay_ms, this, [=]() {
               next();
           });
        }
	}
	else
	{
		m_px += m_speed;
        if(m_px >= width()) {
            m_waitForDelay = true;
            m_timer.stop();
            QTimer::singleShot(m_strings[m_current].delay()*1000, this, [=]() { next(); });
        }
    }
    paint.drawText(m_px, -3, width()-m_px, height()+3, Qt::AlignLeft|Qt::AlignVCenter ,text());
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
        m_px = (m_direction==RightToLeft) ? width() : 0;
    }
    if (!m_strings[m_current].link().isEmpty()) {
        setCursor(Qt::PointingHandCursor);
    } else{
        setCursor(Qt::ArrowCursor);
    }
    m_timer.start(m_speedTimerMs);
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
        m_timer.start(m_speedTimerMs);
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
        m_px = width();
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
    m_timer.start(m_speedTimerMs);
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
    m_timer.start(m_speedTimerMs);
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
        QString str_data(data);
        qDebug() << "MarqueeLabel::load" << str_data;
        return false;
    }
//    QString str_data(data);
//    qInfo() << "MarqueeLabel::load" << str_data;

    reset();

    QJsonObject mainObj = doc.object();
    QJsonArray array = mainObj["messages"].toArray();
    for(int i=0; i<array.size(); ++i)
    {
        QJsonObject dataObject = array[i].toObject();
        MarqueeString mstring(dataObject);
        if (mstring.startdate().isValid() && (QDate::currentDate() < mstring.startdate())) {
            //qDebug() << "JSON wait for start date";
            continue;
        }
        if (mstring.enddate().isValid() && (QDate::currentDate() > mstring.enddate())) {
            //qDebug() << "JSON expired";
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
                    if (i.key()=="type" && analyzer->name()==i.value()) {
                        m_strings << mstring;
                        break;
                    }
                    if (i.key()=="sn" && analyzer->serilal()==i.value()) {
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

    qDebug() << "----> MarqueeLabel::request():" << url;

    m_downloader->startDownloadInfo(QUrl(url));
}

void MarqueeLabel::on_downloadInfoComplete()
{
    m_downloader->startDownloadFw();
}

void MarqueeLabel::on_downloadFileComplete()
{
//    if (!m_downloader->error().isEmpty()) {
//        QString msg = tr("Network error: ")+m_downloader->error();
//        QMessageBox::information(nullptr, "QpenSSL", msg);
//    }
    QByteArray arr = m_downloader->file();
    if (load(arr)) {
        show();
    }
}

void MarqueeLabel::reset()
{
    m_timer.stop();
    m_strings.clear();
    m_px = 0;
    m_py = 10;
    m_speed = 1;
    m_direction = RightToLeft;
}

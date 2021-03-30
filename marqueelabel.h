#ifndef _MARQUEELABEL_H_
#define _MARQUEELABEL_H_

#include <QLabel>
#include <QTimer>
#include <QJsonObject>

class MarqueeString
{
    QString m_text;
    QString m_link;
    int m_timeAfterSec = 0;
    QColor m_color = Qt::transparent;
    double m_width = 100; // ??????
    QList<QString> m_keywords;
    int m_speed = 1;

public:
    MarqueeString() {}
    MarqueeString(QJsonObject& obj) { read(obj); }
    MarqueeString(const QString& text) { m_text = text; }
    ~MarqueeString() {}

    QString text() { return m_text; }
    QString link() { return m_link; }
    int delay() { return m_timeAfterSec ; }
    QColor color() { return m_color; }
    double width() { return m_width ; }
    int speed() { return m_speed; }
    QList<QString> keywords() { return m_keywords; }

    void read(QJsonObject& obj)
    {
        m_text = obj["message"].toString("");
        m_link = obj["linkto"].toString("");
        m_timeAfterSec = obj["timeoutafter"].toInt(0);
        m_speed = obj["speed"].toInt(1);
        QString tmp = obj["textcolor"].toString("");
        if (tmp.isEmpty()) {
            m_color = QColor(Qt::transparent);
        } else if (tmp[0] != '#')
            tmp = "#" + tmp;
        QColor color(tmp);
        m_color = color.isValid() ? color : (QColor(Qt::transparent));
        m_width = obj["size"].toDouble(100.0);
        tmp = obj["keywords"].toString("");
        if (!tmp.isEmpty()) {
            m_keywords = tmp.split(",");
        }
    }
};

class MarqueeLabel : public QLabel
{
	Q_OBJECT

public: 
    enum Direction{LeftToRight,RightToLeft};

    MarqueeLabel(QWidget *parent = 0);
    ~MarqueeLabel();
    void show();
    void setAlignment(Qt::Alignment);
    int getSpeed();
    void setStrings(QList<MarqueeString>& list);
    bool load(QString& path);
    bool load(QByteArray& data);

signals:
    void clicked(const QString& link);

public slots: 
    void setSpeed(int s);
    void setDirection(int d);
    int speed();

protected: 
    void paintEvent(QPaintEvent *evt);
    void resizeEvent(QResizeEvent *evt);
    void updateCoordinates();
    void mousePressEvent ( QMouseEvent * event ) ;
    void repeate(int delay);
    void next();

private: 
    int m_px;
    int m_py;
    QTimer m_timer;
    Qt::Alignment m_align;
    int m_speed;
    int m_direction;
    int m_fontSize;
    int m_textLength;
    QList<MarqueeString> m_strings;
    int m_current = 0;
    int m_repeateDelaySec = 10;
    bool m_waitForDelay = false;
};

#endif /*_MARQUEELABEL_H_*/

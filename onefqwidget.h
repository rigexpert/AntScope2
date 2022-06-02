#ifndef ONEFQWIDGET_H
#define ONEFQWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtNetwork>
#include "analyzerparameters.h"
#include "settings.h"

#define UDP_SEND_VERSION "AA1"
#define UDP_RECEIVE_VERSION "AA2"
#define UDP_PORT_SEND 6050
#define UDP_PORT_RECEIVE 6051

class OneFqWidget : public QWidget
{
    Q_OBJECT

    QSettings *m_settings;
    QLabel m_label;
    QColor m_bgColor;
    QColor m_penColor;
    QColor m_textColor;
    int m_points;
    QPair<bool, bool> m_hints; // 1-hintEnabled, 2-shortHintEnabled
    bool m_hintEnabled = true;
    bool m_shortHintEnabled = true;
    GraphData m_data;
    int m_added=0;
    int m_x;
    int m_y;
    int m_biasX;
    int m_biasY;
    int m_mainX;
    int m_mainY;
    int m_mainBiasX;
    int m_mainBiasY;
    int m_parentX;
    int m_parentY;
    QUdpSocket* m_udpSender=nullptr;
    QUdpSocket* m_udpReceiver=nullptr;
    QHostAddress m_udpAddress;
    bool m_needBroadcast = false;
    qreal m_broadcastFq = 0;

public:
    explicit OneFqWidget(int _points, QWidget *parent = 0);
    ~OneFqWidget();

    void setBackgroundColor(QColor color){ m_bgColor = color;}
    void setPenColor(QColor color){ m_penColor = color;}
    void setTextColor(QString _name)
    { // _name = #RRGGBB
        m_textColor.setNamedColor(_name);
        m_label.setStyleSheet("QLabel { color : " + _name + ";"
                            "margin-top: 6px;"
                            "margin-bottom: 6px;"
                            "margin-left: 10px;"
                            "margin-right: 10px; }");}
    void addData(GraphData _data);
    GraphData& getData() { return m_data; }
    void reset() { m_added=0;  }
    int points() { return m_points; }
    bool needUpdate() { return (m_added >= m_points); }
    void saveHintFlags(QPair<bool, bool> _hints) { m_hints=_hints; }
    QPair<bool, bool> resoreHintFlags() { return m_hints; }
    void needBroadcast(qreal fq) { m_needBroadcast=true; m_broadcastFq=fq; }
    void MainWindowPos(int x, int y);
    void setX(int x){m_x = x;}
    void setY(int y){m_y = y;}
    void setPosition(int x, int y);
    void setParentPosition(int x, int y)
    {
        m_parentX = x;
        m_parentY = y;
    }

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent *);
    void addValue(double src, double& dst);
    void updateText();
    void broadcastDatagram();

signals:
    void canceled(bool);
    void udpReceived(QString cmd, qreal data);

public slots:
    void processPendingDatagrams();
    void setText(const QString& text);
    QString getText();
};

#endif // ONEFQWIDGET_H

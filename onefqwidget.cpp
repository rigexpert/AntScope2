#include "onefqwidget.h"
#include "mainwindow.h"

static QString msgFormat = "FQ: %1\nSWR: %2\nRhoPhase: %3\nRhoMod: %4\nR: %5\nX: %6\nZ: %7\nRpar: %8\nXpar: %9\nZpar: %10\nRL: %11";
extern bool g_developerMode;
extern MainWindow* g_mainWindow;

OneFqWidget::OneFqWidget(int _points, QWidget *parent) :
    QWidget(parent),
    m_points(_points),
    m_hints(true,true),
    m_x(850),
    m_y(130),
    m_biasX(0),
    m_biasY(0),
    m_mainX(177),
    m_mainY(131),
    m_mainBiasX(0),
    m_mainBiasY(0),
    m_bgColor(0,0,0,180),
    m_penColor(255,155,255,180),
    m_textColor(Qt::white)
{
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::Tool |
                   Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    m_label.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_label.setStyleSheet("QLabel { color : " + m_textColor.name() +
                        "margin-top: 6px;"
                        "margin-bottom: 6px;"
                        "margin-left: 10px;"
                        "margin-right: 10px; }");

    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->addWidget(&m_label);
//    vlayout->addLayout(hlayout);

    setLayout(vlayout);

    QString path = Settings::setIniFile();
    m_settings = new QSettings(path,QSettings::IniFormat);
    m_settings->beginGroup("OneFqWidget");
    m_x = m_settings->value("x",185).toInt();
    m_y = m_settings->value("y",642).toInt();
    m_mainX = m_settings->value("mainX",169).toInt();
    m_mainY = m_settings->value("mainY",101).toInt();
    m_mainBiasX = m_settings->value("mainBiasX",16).toInt();
    m_mainBiasY = m_settings->value("mainBiasY",541).toInt();

//    QWidget* widget = parent != nullptr ? parent : qApp->activeWindow();
//    QPoint pt = widget->mapToGlobal(widget->rect().center());
//    QScreen* pScreen = QGuiApplication::screenAt(pt);
//    QRect availableScreenSize = pScreen->availableGeometry();
    int widthDesc = MainWindow::m_mainWindow->width();
    int heightDesc = MainWindow::m_mainWindow->height();
    if((m_x > widthDesc - width()) || (m_x < 0))
    {
        m_x = 500;
    }
    if( (m_y > heightDesc - height()) || (m_y < 0))
    {
        m_y = 500;
    }
    setGeometry(m_x,m_y,width(),height());
    m_settings->endGroup();

    setText(msgFormat);
    if (g_developerMode) {
        m_udpSender = new QUdpSocket(this);
        m_udpReceiver = new QUdpSocket(this);
        m_udpReceiver->bind(UDP_PORT_RECEIVE, QUdpSocket::ShareAddress);
        connect(m_udpReceiver, &QUdpSocket::readyRead, this, &OneFqWidget::processPendingDatagrams);
    }
    qDebug() << "OneFqWidget::OneFqWidget()";
}

OneFqWidget::~OneFqWidget()
{
    m_settings->beginGroup("OneFqWidget");
    m_settings->setValue("x",m_x);
    m_settings->setValue("y",m_y);
    m_settings->setValue("mainX",m_mainX);
    m_settings->setValue("mainY",m_mainY);
    m_settings->setValue("mainBiasX",m_mainBiasX);
    m_settings->setValue("mainBiasY",m_mainBiasY);
    m_settings->endGroup();
    delete m_settings;
    if (g_developerMode) {
        m_udpSender->close();
        m_udpSender->deleteLater();
        m_udpSender = nullptr;
        m_udpReceiver->close();
        m_udpReceiver->deleteLater();
        m_udpReceiver = nullptr;
    }
    qDebug() << "OneFqWidget::~OneFqWidget()";
}

void OneFqWidget::paintEvent(QPaintEvent *event)
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

void OneFqWidget::setText(const QString &text)
{
    m_label.setText(text);
    adjustSize();
}

QString OneFqWidget::getText()
{
    return m_label.text();
}

void OneFqWidget::mousePressEvent(QMouseEvent * event)
{
    m_biasX = event->pos().x();
    m_biasY = event->pos().y();
}

void OneFqWidget::mouseMoveEvent(QMouseEvent * )
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

void OneFqWidget::MainWindowPos(int x, int y)
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

void OneFqWidget::setPosition(int x, int y)
{
    m_x = x;
    m_y = y;
    setGeometry(m_x,
                m_y,
                width(),
                height());
}

void OneFqWidget::addData(GraphData _data)
{
    m_added++;
    addValue(_data.FQ, m_data.FQ);
    addValue(_data.SWR, m_data.SWR);
    addValue(_data.RhoPhase, m_data.RhoPhase);
    addValue(_data.RhoMod, m_data.RhoMod);
    addValue(_data.R, m_data.R);
    addValue(_data.X, m_data.X);
    addValue(_data.Z, m_data.Z);
    addValue(_data.Rpar, m_data.Rpar);
    addValue(_data.Xpar, m_data.Xpar);
    addValue(_data.Zpar, m_data.Zpar);
    addValue(_data.RL, m_data.RL);

    if (g_developerMode) {
        if (m_needBroadcast && _data.FQ == m_broadcastFq) {
            broadcastDatagram();
            m_needBroadcast = false;
        }
    }

    if (m_added == 1)
        updateText();

    if (m_added >= m_points) {
        updateText();
        reset();
        m_data = GraphData();
    }
}

void OneFqWidget::addValue(double src, double &dst)
{
    if (src != DBL_MAX)
        dst = (dst==DBL_MAX ? src : (dst+src)/2.0);
}

void OneFqWidget::updateText()
{
    QString msg = QString(msgFormat)
            .arg(m_data.FQ)
            .arg(m_data.SWR)
            .arg(m_data.RhoPhase)
            .arg(m_data.RhoMod)
            .arg(m_data.R)
            .arg(m_data.X)
            .arg(m_data.Z)
            .arg(m_data.Rpar)
            .arg(m_data.Xpar)
            .arg(m_data.Zpar)
            .arg(m_data.RL);
    setText(msg);
}

void OneFqWidget::broadcastDatagram()
{
    if (g_developerMode) {
        QString msg = QString("%1, %2, %3")
                .arg(UDP_SEND_VERSION)
                .arg(m_data.FQ)
                .arg(m_data.SWR);

        QByteArray datagram = msg.toUtf8();
        m_udpSender->writeDatagram(datagram.data(), datagram.size(), m_udpAddress, UDP_PORT_SEND);
                                 //QHostAddress::Broadcast, UDP_PORT_SEND);
    }
}

void OneFqWidget::processPendingDatagrams()
{
    if (g_developerMode) {
        while (m_udpReceiver->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(m_udpReceiver->pendingDatagramSize());
            m_udpReceiver->readDatagram(datagram.data(), datagram.size(), &m_udpAddress);
            QString msg(datagram.data());
            qDebug() << "OneFqWidget::processPendingDatagrams()" << msg;
            if (!msg.isEmpty()) {
                QList<QString> list = msg.split(',');
                if (list.size() > 1) {
                    if (list.at(0).trimmed().compare("AA2", Qt::CaseInsensitive) == 0) {
                        if (list.at(1).trimmed().compare("SETFQ", Qt::CaseInsensitive) == 0) {
                            bool ok;
                            QString strfq = list.at(2).trimmed();
                            qreal fq = strfq.toDouble(&ok) *1000;
                            if (ok) {
                                emit udpReceived(list.at(1), fq);
                            }
                        }
                    }
                }
            }
        }
    }
}

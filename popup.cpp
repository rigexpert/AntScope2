#include "popup.h"
#include <QPainter>
#include <QApplication>
//#include <QDesktopWidget>
#include <QDebug>
#include <mainwindow.h>

PopUp::PopUp(QWidget *parent, bool embedded) : QWidget(parent),
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
    m_mainBiasY(0),
    m_embedded(embedded)
{
    init();
}

PopUp::PopUp(QString buttonName, QWidget *parent, bool embedded) : QWidget(parent),
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
    m_mainBiasY(0),
    m_buttonName(buttonName),
    m_showButton(true),
    m_embedded(embedded)
{
    init();
}

void PopUp::init()
{
    if (m_embedded) {
        // A plain child widget of its parent, not a separate top-level
        // window: there is no window for a window manager/compositor to
        // ever see, so this cannot perturb the parent's activation state
        // (see focusShow()/focusHide()) and it cannot show up in any
        // window/task list either.
    } else {
        setWindowFlags(Qt::FramelessWindowHint |
                       Qt::Tool |
                       Qt::WindowStaysOnTopHint |
                       Qt::WindowDoesNotAcceptFocus);
        setAttribute(Qt::WA_ShowWithoutActivating);
    }
    setAttribute(Qt::WA_TranslucentBackground);

    animation.setTargetObject(this);
    animation.setPropertyName("popupOpacity");
    connect(&animation, &QAbstractAnimation::finished, this, &PopUp::hide);

    label.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label.setStyleSheet("QLabel { color : " + m_textColor + ";"
                        "margin-top: 6px;"
                        "margin-bottom: 6px;"
                        "margin-left: 10px;"
                        "margin-right: 10px; }");

    layout.addWidget(&label);
    button.setVisible(false);
    if (m_showButton) {
        button.setText(m_buttonName);
        button.setVisible(true);
        button.setEnabled(true);
        connect(&button, &QPushButton::clicked, this, [=](){
            hide();
            emit canceled();
        });
        connect(&button, &QPushButton::released, this, [=](){
            hide();
            emit canceled();
        });
        layout.addWidget(&button);
    }
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

//    QWidget* widget = parentWidget() != nullptr ? parentWidget() : qApp->activeWindow();
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

    m_settings->endGroup();

    applyGeometry();
}

void PopUp::applyGeometry()
{
    // m_x/m_y are tracked as global screen coordinates throughout this
    // class (MainWindowPos(), mouseMoveEvent() and every caller of
    // setPosition() all compute them that way, matching the original
    // Qt::Tool top-level design). When embedded as a child widget,
    // translate to parent-relative coordinates right here instead of
    // touching that positioning math anywhere else.
    if (m_embedded && parentWidget()) {
        QPoint local = parentWidget()->mapFromGlobal(QPoint(m_x, m_y));
        setGeometry(local.x(), local.y(), width(), height());
    } else {
        setGeometry(m_x, m_y, width(), height());
    }
}

PopUp::~PopUp()
{
    disconnect();
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
    applyGeometry();
    if (!isVisible()) {
        QWidget::show();
    }
    if (m_embedded) {
        raise();
    }
}

void PopUp::focusHide()
{
    if (m_embedded) {
        // A plain child widget: hiding it does not touch any top-level
        // window, so there is no compositor feedback-loop risk here -
        // see the Qt::Tool branch below for what that risk was.
        QWidget::hide();
        return;
    }
    // Park off-screen instead of QWidget::hide(): unmapping this
    // Qt::Tool top-level surface causes some Wayland compositors
    // (observed on COSMIC/cosmic-comp) to send a spurious
    // WindowActivate/WindowDeactivate event to the parent window. Since
    // that event drives this same show/hide logic (via MainWindow::event
    // -> focus() -> showHideHints()), unmapping creates a self-sustaining
    // feedback loop ("flickering popups"). Moving off-screen is visually
    // equivalent while keeping the surface continuously mapped.
    move(-32000, -32000);
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



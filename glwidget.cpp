#include "glwidget.h"
#include <QGLFunctions>
#include <QPainter>
#include <QOpenGLTexture>
#include <QTransform>
#include <QOpenGLBuffer>
#include <QVector2D>
#include <QApplication>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QOpenGLTexture>
#include <QTextOption>

GLWidget::GLWidget(QCustomPlot* _plotter, QWidget *parent) :
    QWidget(parent)
{
    setObjectName("GLWidget");
    setPlotter(_plotter);
}

GLWidget::GLWidget(QWidget *parent) :
    QWidget(parent)
{
    setObjectName("GLWidget");
}

GLWidget::~GLWidget()
{
}

void GLWidget::setPlotter(QCustomPlot* _plotter)
{
    if (m_plotter != nullptr)
    {
        m_plotter->disconnect();
        m_plotter->hide();
        m_plotter->setEnabled(false);
    }
    m_plotter = _plotter;
    connect(m_plotter, &QCustomPlot::mouseWheel, this, &GLWidget::onMouseWheelEvent);
    connect(m_plotter, &QCustomPlot::mouseMove, this, &GLWidget::onMouseMoveEvent);
    m_plotter->setEnabled(true);
    m_plotter->show();
    repaint();
}

void GLWidget::replot()
{
    repaint();
}

void GLWidget::onMouseWheelEvent(QWheelEvent*)
{
    //qDebug() << "GLWidget::onMouseWheelEvent(QWheelEvent*)";
    repaint();
}

void GLWidget::onMouseMoveEvent(QMouseEvent*)
{
    //qDebug() << "GLWidget::onMouseMoveEvent(QMouseEvent*)";
    repaint();
}


void GLWidget::paintEvent(QPaintEvent *event)
{
        QWidget::paintEvent(event);
}


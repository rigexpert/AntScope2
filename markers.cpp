#include "markers.h"

Markers::Markers(QObject *parent) : QObject(parent),
    m_swrWidget(NULL),
    m_phaseWidget(NULL),
    m_rsWidget(NULL),
    m_rpWidget(NULL),
    m_rlWidget(NULL),
    m_tdrWidget(NULL),
    m_smithWidget(NULL),
    m_markersHint(NULL),
    m_markersHintEnabled(true),
    m_measurements(NULL)
{
    QString path = Settings::setIniFile();
    m_settings = new QSettings(path, QSettings::IniFormat);
    m_settings->beginGroup("Markers");

    m_markersHintEnabled = m_settings->value("markersHintEnabled", true).toBool();

    m_settings->endGroup();

    if(m_markersHint == NULL)
    {
        m_markersHint = new MarkersPopUp();
        m_markersHint->setHiding(false);
        if(m_markersHintEnabled && !m_markersList.isEmpty())
            m_markersHint->focusShow();
        m_markersHint->setName(tr("Markers"));
        connect(m_markersHint, SIGNAL(removeMarker(int)), SLOT(on_removeMarker(int)));
        repaint();
    }
}

Markers::~Markers()
{
    m_settings->beginGroup("Markers");
    m_settings->setValue("markersHintEnabled", m_markersHintEnabled);
    m_settings->endGroup();

    if(m_markersHint)
    {
        delete m_markersHint;
    }
}

void Markers::setWidgets(QCustomPlot * swr, QCustomPlot * phase, QCustomPlot * rs, QCustomPlot * rp,
                         QCustomPlot * rl, QCustomPlot * tdr, QCustomPlot * smith)
{
    m_swrWidget = swr;
    m_phaseWidget = phase;
    m_rsWidget = rs;
    m_rpWidget = rp;
    m_rlWidget = rl;
    m_tdrWidget = tdr;
    m_smithWidget = smith;
}

void Markers::setMeasurements(Measurements *m)
{
    m_measurements = m;
}

void Markers::create(double fq)
{
    marker *m = new marker();
    m->frequency = fq;

    m->swrLine = new QCPItemStraightLine(m_swrWidget);
    m->swrLineText = new QCPItemText(m_swrWidget);
    m->swrLine->setAntialiased(false);
    m->swrLine->setPen(QPen(QColor(255,0,0,150)));
    m->swrLineText->setColor(QColor(255, 0, 0, 150));

    m->phaseLine = new QCPItemStraightLine(m_phaseWidget);
    m->phaseLineText = new QCPItemText(m_phaseWidget);
    m->phaseLine->setAntialiased(false);
    m->phaseLine->setPen(QPen(QColor(255,0,0,150)));
    m->phaseLineText->setColor(QColor(255, 0, 0, 150));

    m->rsLine = new QCPItemStraightLine(m_rsWidget);
    m->rsLineText = new QCPItemText(m_rsWidget);
    m->rsLine->setAntialiased(false);
    m->rsLine->setPen(QPen(QColor(255,0,0,150)));
    m->rsLineText->setColor(QColor(255, 0, 0, 150));

    m->rpLine = new QCPItemStraightLine(m_rpWidget);
    m->rpLineText = new QCPItemText(m_rpWidget);
    m->rpLine->setAntialiased(false);
    m->rpLine->setPen(QPen(QColor(255,0,0,150)));
    m->rpLineText->setColor(QColor(255, 0, 0, 150));

    m->rlLine = new QCPItemStraightLine(m_rlWidget);
    m->rlLineText = new QCPItemText(m_rlWidget);
    m->rlLine->setAntialiased(false);
    m->rlLine->setPen(QPen(QColor(255,0,0,150)));
    m->rlLineText->setColor(QColor(255, 0, 0, 150));

    m_markersList.append(m);
}

void Markers::setFq(double fq)
{
    if(m_markersList.length() == 0)
    {
        return;
    }

    m_markersList.last()->frequency = fq;

    m_markersList.last()->swrLine->point1->setCoords(fq, MIN_SWR);
    m_markersList.last()->swrLine->point2->setCoords(fq, MAX_SWR);

    double offsetX = (m_swrWidget->xAxis->range().upper - m_swrWidget->xAxis->range().lower)/40;
    double offsetY = (m_swrWidget->yAxis->range().upper - m_swrWidget->yAxis->range().lower)/10;
    m_markersList.last()->swrLineText->position->setCoords(fq + offsetX, m_swrWidget->yAxis->range().center()-offsetY);
    m_markersList.last()->swrLineText->setText(QString::number(fq));

    //==========================================================================
    m_markersList.last()->phaseLine->point1->setCoords(fq, -180);
    m_markersList.last()->phaseLine->point2->setCoords(fq, 180);

    offsetX = (m_phaseWidget->xAxis->range().upper - m_phaseWidget->xAxis->range().lower)/40;
    offsetY = (m_phaseWidget->yAxis->range().upper - m_phaseWidget->yAxis->range().lower)/10;
    m_markersList.last()->phaseLineText->position->setCoords(fq + offsetX, m_phaseWidget->yAxis->range().center()-offsetY);
    m_markersList.last()->phaseLineText->setText(QString::number(fq));

    //==========================================================================
    m_markersList.last()->rsLine->point1->setCoords(fq, -1600);
    m_markersList.last()->rsLine->point2->setCoords(fq, 1600);

    offsetX = (m_rsWidget->xAxis->range().upper - m_rsWidget->xAxis->range().lower)/40;
    offsetY = (m_rsWidget->yAxis->range().upper - m_rsWidget->yAxis->range().lower)/10;
    m_markersList.last()->rsLineText->position->setCoords(fq + offsetX, m_rsWidget->yAxis->range().center()-offsetY);
    m_markersList.last()->rsLineText->setText(QString::number(fq));

    //==========================================================================
    m_markersList.last()->rpLine->point1->setCoords(fq, -1600);
    m_markersList.last()->rpLine->point2->setCoords(fq, 1600);

    offsetX = (m_rpWidget->xAxis->range().upper - m_rpWidget->xAxis->range().lower)/40;
    offsetY = (m_rpWidget->yAxis->range().upper - m_rpWidget->yAxis->range().lower)/10;
    m_markersList.last()->rpLineText->position->setCoords(fq + offsetX, m_rpWidget->yAxis->range().center()-offsetY);
    m_markersList.last()->rpLineText->setText(QString::number(fq));

    //==========================================================================
    m_markersList.last()->rlLine->point1->setCoords(fq, 0);
    m_markersList.last()->rlLine->point2->setCoords(fq, 60);

    offsetX = (m_rlWidget->xAxis->range().upper - m_rlWidget->xAxis->range().lower)/40;
    offsetY = (m_rlWidget->yAxis->range().upper - m_rlWidget->yAxis->range().lower)/10;
    m_markersList.last()->rlLineText->position->setCoords(fq + offsetX, m_rlWidget->yAxis->range().center()-offsetY);
    m_markersList.last()->rlLineText->setText(QString::number(fq));

    redraw();
}

void Markers::rescale()
{
    for(int i = 0; i < m_markersList.length(); ++i)
    {
        double fq = m_markersList.at(i)->frequency;
        double offsetX;
        double offsetY;

        if(m_currentTab == "tab_1")
        {
            offsetX = (m_swrWidget->xAxis->range().upper - m_swrWidget->xAxis->range().lower)/40;
            offsetY = (m_swrWidget->yAxis->range().upper - m_swrWidget->yAxis->range().lower)/10;
            m_markersList.at(i)->swrLineText->position->setCoords(fq + offsetX/2, m_swrWidget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_2")
        {
            offsetX = (m_phaseWidget->xAxis->range().upper - m_phaseWidget->xAxis->range().lower)/40;
            offsetY = (m_phaseWidget->yAxis->range().upper - m_phaseWidget->yAxis->range().lower)/10;
            m_markersList.at(i)->phaseLineText->position->setCoords(fq + offsetX/2, m_phaseWidget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_3")
        {
            offsetX = (m_rsWidget->xAxis->range().upper - m_rsWidget->xAxis->range().lower)/40;
            offsetY = (m_rsWidget->yAxis->range().upper - m_rsWidget->yAxis->range().lower)/10;
            m_markersList.at(i)->rsLineText->position->setCoords(fq + offsetX/2, m_rsWidget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_4")
        {
            offsetX = (m_rpWidget->xAxis->range().upper - m_rpWidget->xAxis->range().lower)/40;
            offsetY = (m_rpWidget->yAxis->range().upper - m_rpWidget->yAxis->range().lower)/10;
            m_markersList.at(i)->rpLineText->position->setCoords(fq + offsetX/2, m_rpWidget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_5")
        {
            offsetX = (m_rlWidget->xAxis->range().upper - m_rlWidget->xAxis->range().lower)/40;
            offsetY = (m_rlWidget->yAxis->range().upper - m_rlWidget->yAxis->range().lower)/10;
            m_markersList.at(i)->rlLineText->position->setCoords(fq + offsetX/2, m_rlWidget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_6")
        {
        }else if(m_currentTab == "tab_7")
        {
        }
    }
}

void Markers::add()
{
    if(m_markersHint == NULL)
    {
        return;
    }
    for(int i = 0; i < m_markersList.length(); ++i)
    {
        m_markersList.at(i)->swrLineText->setText(QString::number(i+1));
        m_markersList.at(i)->phaseLineText->setText(QString::number(i+1));
        m_markersList.at(i)->rsLineText->setText(QString::number(i+1));
        m_markersList.at(i)->rpLineText->setText(QString::number(i+1));
        m_markersList.at(i)->rlLineText->setText(QString::number(i+1));
    }
    repaint();
    redraw();
    if(m_markersHintEnabled && !m_markersList.isEmpty()) {
       m_markersHint->focusShow();
    }
}

void Markers::on_focus(bool focus)
{    
    m_focus = focus;

    if(m_markersHint)
    {
        if(m_markersHintEnabled && m_focus && !m_markersList.isEmpty())
        {
            m_markersHint->focusShow();
        }else
        {
            m_markersHint->focusHide();
        }
    }
}

void Markers::repaint()
{
    m_markersHint->clearTable();

    if(!m_measurements)
    {
        return;
    }
    for(int n = 0; n < m_markersList.length(); ++n)
    {
        QVector<double> fq;
        QVector<double> swr;
        QVector<double> rl;
        QVector<QString> z;
        QVector<double> phase;
        QVector<int> measurement;

        int count = m_measurements->getMeasurementLength();
        for(int i = 0; i < count; ++i)
        {
            measurement.append(i+1);
            fq.append(m_markersList.at(n)->frequency);

            double dSwr;
            double dRl;
            double dR;
            double dX;
            double dPhase;

            QCPDataMap *swrMap;
            if(m_measurements->getCalibrationEnabled())
            {
                swrMap = &m_measurements->getMeasurement(i)->swrGraphCalib;
            }else
            {
                swrMap = &m_measurements->getMeasurement(i)->swrGraph;
            }
            QList <double> swrKeys = swrMap->keys();

            for(int ii = 0; ii < swrKeys.length()-1; ++ii)
            {
                if((swrKeys.at(ii) <= m_markersList.at(n)->frequency) && (swrKeys.at(ii+1) >= m_markersList.at(n)->frequency))
                {
                    double frequency1 = swrKeys.at(ii);
                    double frequency2 = swrKeys.at(ii+1);

                    if(m_measurements->getCalibrationEnabled())
                    {
                        if(m_measurements->getFarEndMeasurement() == 1)
                        {
                            dPhase = interpolate(frequency1,
                                                 m_markersList.at(n)->frequency,
                                                 frequency2,
                                                 m_measurements->getMeasurementSub(i)->phaseGraphCalib.value(frequency1).value,
                                                 m_measurements->getMeasurementSub(i)->phaseGraphCalib.value(frequency2).value);

                            dR = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurementSub(i)->rsrGraphCalib.value(frequency1).value,
                                             m_measurements->getMeasurementSub(i)->rsrGraphCalib.value(frequency2).value);

                            dX = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurementSub(i)->rsxGraphCalib.value(frequency1).value,
                                             m_measurements->getMeasurementSub(i)->rsxGraphCalib.value(frequency2).value);

                        }else if(m_measurements->getFarEndMeasurement() == 2)
                        {
                            dPhase = interpolate(frequency1,
                                                 m_markersList.at(n)->frequency,
                                                 frequency2,
                                                 m_measurements->getMeasurementAdd(i)->phaseGraphCalib.value(frequency1).value,
                                                 m_measurements->getMeasurementAdd(i)->phaseGraphCalib.value(frequency2).value);

                            dR = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurementAdd(i)->rsrGraphCalib.value(frequency1).value,
                                             m_measurements->getMeasurementAdd(i)->rsrGraphCalib.value(frequency2).value);

                            dX = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurementAdd(i)->rsxGraphCalib.value(frequency1).value,
                                             m_measurements->getMeasurementAdd(i)->rsxGraphCalib.value(frequency2).value);
                        }else
                        {
                            dPhase = interpolate(frequency1,
                                                 m_markersList.at(n)->frequency,
                                                 frequency2,
                                                 m_measurements->getMeasurement(i)->phaseGraphCalib.value(frequency1).value,
                                                 m_measurements->getMeasurement(i)->phaseGraphCalib.value(frequency2).value);

                            dR = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurement(i)->dataRXCalib.at(ii).r,
                                             m_measurements->getMeasurement(i)->dataRXCalib.at(ii+1).r);

                            dX = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurement(i)->dataRXCalib.at(ii).x,
                                             m_measurements->getMeasurement(i)->dataRXCalib.at(ii+1).x);
                        }
                        double m1 = m_measurements->getMeasurement(i)->swrGraphCalib.value(frequency1).value;
                        double m2 = m_measurements->getMeasurement(i)->swrGraphCalib.value(frequency2).value;
                        if(m1 > 10)
                        {
                            m1 = 10;
                        }
                        if(m2 > 10)
                        {
                            m2 = 10;
                        }
                        dSwr = interpolate(frequency1,
                                           m_markersList.at(n)->frequency,
                                           frequency2,
                                           m1,
                                           m2);

                        dRl = interpolate(frequency1,
                                          m_markersList.at(n)->frequency,
                                          frequency2,
                                          m_measurements->getMeasurement(i)->rlGraphCalib.value(frequency1).value,
                                          m_measurements->getMeasurement(i)->rlGraphCalib.value(frequency2).value);

                    }else
                    {
                        if(m_measurements->getFarEndMeasurement() == 1)
                        {
                            dPhase = interpolate(frequency1,
                                                 m_markersList.at(n)->frequency,
                                                 frequency2,
                                                 m_measurements->getMeasurementSub(i)->phaseGraph.value(frequency1).value,
                                                 m_measurements->getMeasurementSub(i)->phaseGraph.value(frequency2).value);

                            dR = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurementSub(i)->rsrGraph.value(frequency1).value,
                                             m_measurements->getMeasurementSub(i)->rsrGraph.value(frequency2).value);

                            dX = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurementSub(i)->rsxGraph.value(frequency1).value,
                                             m_measurements->getMeasurementSub(i)->rsxGraph.value(frequency2).value);

                        }else if(m_measurements->getFarEndMeasurement() == 2)
                        {
                            dPhase = interpolate(frequency1,
                                                 m_markersList.at(n)->frequency,
                                                 frequency2,
                                                 m_measurements->getMeasurementAdd(i)->phaseGraph.value(frequency1).value,
                                                 m_measurements->getMeasurementAdd(i)->phaseGraph.value(frequency2).value);

                            dR = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurementAdd(i)->rsrGraph.value(frequency1).value,
                                             m_measurements->getMeasurementAdd(i)->rsrGraph.value(frequency2).value);

                            dX = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurementAdd(i)->rsxGraph.value(frequency1).value,
                                             m_measurements->getMeasurementAdd(i)->rsxGraph.value(frequency2).value);

                        }else
                        {
                            dPhase = interpolate(frequency1,
                                                 m_markersList.at(n)->frequency,
                                                 frequency2,
                                                 m_measurements->getMeasurement(i)->phaseGraph.value(frequency1).value,
                                                 m_measurements->getMeasurement(i)->phaseGraph.value(frequency2).value);

                            dR = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurement(i)->dataRX.at(ii).r,
                                             m_measurements->getMeasurement(i)->dataRX.at(ii+1).r);

                            dX = interpolate(frequency1,
                                             m_markersList.at(n)->frequency,
                                             frequency2,
                                             m_measurements->getMeasurement(i)->dataRX.at(ii).x,
                                             m_measurements->getMeasurement(i)->dataRX.at(ii+1).x);

                        }
                        double m1 = m_measurements->getMeasurement(i)->swrGraph.value(frequency1).value;
                        double m2 = m_measurements->getMeasurement(i)->swrGraph.value(frequency2).value;
                        if(m1 > 10)
                        {
                            m1 = 10;
                        }
                        if(m2 > 10)
                        {
                            m2 = 10;
                        }
                        dSwr = interpolate(frequency1,
                                           m_markersList.at(n)->frequency,
                                           frequency2,
                                           m1,
                                           m2);

                        dRl = interpolate(frequency1,
                                          m_markersList.at(n)->frequency,
                                          frequency2,
                                          m_measurements->getMeasurement(i)->rlGraph.value(frequency1).value,
                                          m_measurements->getMeasurement(i)->rlGraph.value(frequency2).value);

                    }

                    swr.append(dSwr);
                    rl.append(dRl);
                    phase.append(dPhase);
                    QString zString = QString::number(dR,'f', 2);
                    if(dX >= 0)
                    {
                        zString+= " + j";
                        zString+= QString::number(dX,'f', 2);
                    }else
                    {
                        zString+= " - j";
                        zString+= QString::number((dX * (-1)),'f', 2);
                    }
                    z.append(zString);
                }
            }
        }
        m_markersHint->addRowText( n, &measurement, &fq, &swr, &rl, &z, &phase);
    }
}

double Markers::interpolate(double fq1, double fq2, double fq3, double param1, double param2)
{
    return param1 + (fq2-fq1)/(fq3-fq1) *(param2-param1);
}

void Markers::on_mainWindowPos(int x, int y)
{
    if(m_markersHint)
    {
        m_markersHint->MainWindowPos(x, y);
    }
}

void Markers::on_currentTab(QString name)
{
    m_currentTab = name;
    rescale();
}

void Markers::on_newMeasurement(QString )
{
}

void Markers::on_measurementComplete()
{
    repaint();
}

void Markers::setMarkersHintEnabled(bool enabled)
{
    m_markersHintEnabled = enabled;
    if(m_markersHint)
    {
        if(m_markersHintEnabled && !m_markersList.isEmpty())
        {
            m_markersHint->focusShow();
        }else
        {
            m_markersHint->focusHide();
        }
    }
}

bool Markers::getMarkersHintEnabled(void)
{
    return m_markersHintEnabled;
}

void Markers::saveBmp(QString path)
{
    if(m_markersHint)
    {
        QPixmap map = m_markersHint->grab();
        QPixmap mapScaled = map.scaled(5000,3000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        mapScaled.save(path,"BMP",100);
    }
}

QString Markers::getMarkersHintText()
{
    if(m_markersHint != NULL)
    {
        return m_markersHint->getPopupText();
    }
    return QString();
}

QList <QStringList> Markers::getMarkersHintList()
{
    if(m_markersHint != NULL)
    {
        return m_markersHint->getPopupList();
    }
    return QList <QStringList> ();
}

qint32 Markers::getMarkersCount()
{
    return m_markersList.length();
}
marker Markers::getMarker( quint32 number)
{
    return *m_markersList.at(number);
}

void Markers::redraw(void)
{
    rescale();
    if(m_currentTab == "tab_1")
    {
        m_swrWidget->replot();
    }else if(m_currentTab == "tab_2")
    {
        m_phaseWidget->replot();
    }else if(m_currentTab == "tab_3")
    {
        m_rsWidget->replot();
    }else if(m_currentTab == "tab_4")
    {
        m_rpWidget->replot();
    }else if(m_currentTab == "tab_5")
    {
        m_rlWidget->replot();
    }else if(m_currentTab == "tab_6")
    {
        m_tdrWidget->replot();
    }else if(m_currentTab == "tab_7")
    {
        m_smithWidget->replot();
    }
}

void Markers::on_removeMarker(int number)
{
    m_markersList.at(number)->clear();
    m_markersList.remove(number,1);
    for(int i = 0; i < m_markersList.length(); ++i)
    {
        m_markersList.at(i)->swrLineText->setText(QString::number(i+1));
        m_markersList.at(i)->phaseLineText->setText(QString::number(i+1));
        m_markersList.at(i)->rsLineText->setText(QString::number(i+1));
        m_markersList.at(i)->rpLineText->setText(QString::number(i+1));
        m_markersList.at(i)->rlLineText->setText(QString::number(i+1));
    }
    repaint();
    redraw();
    if (m_markersList.isEmpty()) {
       m_markersHint->focusHide();
    }
}

void Markers::on_translate()
{
    if (m_markersHint != nullptr)
    {
        m_markersHint->setName(tr("Markers"));
        m_markersHint->on_translate();
    }
}

void Markers::changeColorTheme(bool _dark)
{
    if (!_dark) {
        m_markersHint->setBackgroundColor(QColor(127,127,127,64 ));
        m_markersHint->setTextColor("#010101");
    } else {
        m_markersHint->setBackgroundColor(QColor(0,0,0,180 ));
        m_markersHint->setTextColor("#01b2ff");
    }
}

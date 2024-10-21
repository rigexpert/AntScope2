#include "markers.h"
#include "mainwindow.h"

Markers::Markers(QObject *parent) : QObject(parent),
    m_swrWidget(NULL),
    m_phaseWidget(NULL),
    m_rsWidget(NULL),
    m_rpWidget(NULL),
    m_rlWidget(NULL),
    m_s21Widget(NULL),
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
        connect(m_markersHint, &MarkersPopUp::changeColumns, this, [&](){ repaint(); });
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
                         QCustomPlot * rl, QCustomPlot * tdr, QCustomPlot * s21, QCustomPlot * smith)
{
    m_swrWidget = swr;
    m_phaseWidget = phase;
    m_rsWidget = rs;
    m_rpWidget = rp;
    m_rlWidget = rl;
    m_s21Widget = s21;
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

    m->s21Line = new QCPItemStraightLine(m_s21Widget);
    m->s21LineText = new QCPItemText(m_s21Widget);
    m->s21Line->setAntialiased(false);
    m->s21Line->setPen(QPen(QColor(255,0,0,150)));
    m->s21LineText->setColor(QColor(255, 0, 0, 150));

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

    //==========================================================================
    m_markersList.last()->s21Line->point1->setCoords(fq, 0);
    m_markersList.last()->s21Line->point2->setCoords(fq, 60);

    offsetX = (m_s21Widget->xAxis->range().upper - m_s21Widget->xAxis->range().lower)/40;
    offsetY = (m_s21Widget->yAxis->range().upper - m_s21Widget->yAxis->range().lower)/10;
    m_markersList.last()->s21LineText->position->setCoords(fq + offsetX, m_s21Widget->yAxis->range().center()-offsetY);
    m_markersList.last()->s21LineText->setText(QString::number(fq));

    redraw();
}

void Markers::rescale()
{
    for(int i = 0; i < m_markersList.length(); ++i)
    {
        double fq = m_markersList.at(i)->frequency;
        double offsetX;
        double offsetY;

        if(m_currentTab == "tab_swr")
        {
            offsetX = (m_swrWidget->xAxis->range().upper - m_swrWidget->xAxis->range().lower)/40;
            offsetY = (m_swrWidget->yAxis->range().upper - m_swrWidget->yAxis->range().lower)/10;
            m_markersList.at(i)->swrLineText->position->setCoords(fq + offsetX/2, m_swrWidget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_phase")
        {
            offsetX = (m_phaseWidget->xAxis->range().upper - m_phaseWidget->xAxis->range().lower)/40;
            offsetY = (m_phaseWidget->yAxis->range().upper - m_phaseWidget->yAxis->range().lower)/10;
            m_markersList.at(i)->phaseLineText->position->setCoords(fq + offsetX/2, m_phaseWidget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_rs")
        {
            offsetX = (m_rsWidget->xAxis->range().upper - m_rsWidget->xAxis->range().lower)/40;
            offsetY = (m_rsWidget->yAxis->range().upper - m_rsWidget->yAxis->range().lower)/10;
            m_markersList.at(i)->rsLineText->position->setCoords(fq + offsetX/2, m_rsWidget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_rp")
        {
            offsetX = (m_rpWidget->xAxis->range().upper - m_rpWidget->xAxis->range().lower)/40;
            offsetY = (m_rpWidget->yAxis->range().upper - m_rpWidget->yAxis->range().lower)/10;
            m_markersList.at(i)->rpLineText->position->setCoords(fq + offsetX/2, m_rpWidget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_rl")
        {
            offsetX = (m_rlWidget->xAxis->range().upper - m_rlWidget->xAxis->range().lower)/40;
            offsetY = (m_rlWidget->yAxis->range().upper - m_rlWidget->yAxis->range().lower)/10;
            m_markersList.at(i)->rlLineText->position->setCoords(fq + offsetX/2, m_rlWidget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_s21")
        {
            offsetX = (m_s21Widget->xAxis->range().upper - m_s21Widget->xAxis->range().lower)/40;
            offsetY = (m_s21Widget->yAxis->range().upper - m_s21Widget->yAxis->range().lower)/10;
            m_markersList.at(i)->s21LineText->position->setCoords(fq + offsetX/2, m_s21Widget->yAxis->range().center()-offsetY);
        }else if(m_currentTab == "tab_tdr")
        {
        }else if(m_currentTab == "tab_smith")
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
        QString index = QString::number(i+1);
        m_markersList.at(i)->swrLineText->setText(index);
        m_markersList.at(i)->phaseLineText->setText(index);
        m_markersList.at(i)->rsLineText->setText(index);
        m_markersList.at(i)->rpLineText->setText(index);
        m_markersList.at(i)->rlLineText->setText(index);
    }

    changeMarkersHint();
    redraw();
    if(m_markersHintEnabled && !m_markersList.isEmpty()) {
       m_markersHint->focusShow();
    } else {
        m_markersHint->setVisible(false);
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
    if(!m_measurements)
    {
        return;
    }
    QList<int> types = m_markersHint->getColumns();
    QList<QList<QVariant>> info = updateInfo(types);
    m_markersHint->updateInfo(info);
}

QList<QList<QVariant>> Markers::updateInfo(QList<int> _columnTypes)
{
    QList<QList<QVariant>> info;

    for(int n = 0; n < m_markersList.length(); ++n)
    {
        double fq0 = m_markersList.at(n)->frequency;

        int count = m_measurements->getMeasurementLength();
        for(int i=count-1; i>=0; i--)
        {
            QList<QVariant> row;
            int index = i;
            QString name = m_measurements->getMeasurement(i)->name;
            int pos = name.indexOf('>');
            if (pos != -1)
                index = name.left(2).toInt();
            row << QVariant(); // fieldDelete
            row << QVariant(n+1); // fieldMarker
            row << QVariant(index); // fieldSerie
            row << QVariant(fq0); // fieldFQ

            double dSwr=DBL_MAX;
            double dRl=DBL_MAX;
            double dR=DBL_MAX;
            double dX=DBL_MAX;
            double dZmod=DBL_MAX;
            double dL=DBL_MAX;
            double dC=DBL_MAX;
            double dPhase=DBL_MAX;
            double dRho=DBL_MAX;
            double dRpar=DBL_MAX;
            double dXpar=DBL_MAX;
            double dLpar=DBL_MAX;
            double dCpar=DBL_MAX;

            QString zString;
            QString zparString;

            QCPDataMap *swrMap;
            if(m_measurements->getCalibrationEnabled())
            {
                swrMap = &m_measurements->getMeasurement(i)->swrGraphCalib;
            }else
            {
                swrMap = &m_measurements->getMeasurement(i)->swrGraph;
            }
            QList <double> swrKeys = swrMap->keys();
            measurement* mm;
            bool calib = m_measurements->getCalibrationEnabled();
            switch (m_measurements->getFarEndMeasurement()) {
            case 1:
                mm = m_measurements->getMeasurementSub(i);
                break;
            case 2:
                mm = m_measurements->getMeasurementAdd(i);
                break;
            default:
                mm = m_measurements->getMeasurement(i);
                break;
            }

            for(int ii = 0; ii < swrKeys.length()-1; ++ii)
            {
                if((swrKeys.at(ii) <= fq0) && (swrKeys.at(ii+1) >= fq0))
                {
                    double fq1 = swrKeys.at(ii);
                    double fq2 = swrKeys.at(ii+1);

                    if(calib)
                    {
                        dPhase = interpolate(fq1, fq0, fq2, mm->phaseGraphCalib.value(fq1).value, mm->phaseGraphCalib.value(fq2).value);
                        dR = interpolate(fq1, fq0, fq2, mm->rsrGraphCalib.value(fq1).value, mm->rsrGraphCalib.value(fq2).value);
                        dX = interpolate(fq1, fq0, fq2, mm->rsxGraphCalib.value(fq1).value, mm->rsxGraphCalib.value(fq2).value);
                        dRho = interpolate(fq1, fq0, fq2, mm->rhoGraphCalib.value(fq1).value, mm->rhoGraphCalib.value(fq2).value);
                        dRpar = interpolate(fq1, fq0, fq2, mm->rprGraphCalib.value(fq1).value, mm->rprGraphCalib.value(fq2).value);
                        dXpar = interpolate(fq1, fq0, fq2, mm->rpxGraphCalib.value(fq1).value, mm->rpxGraphCalib.value(fq2).value);
//                        double m1 = m_measurements->getMeasurement(i)->swrGraphCalib.value(fq1).value;
//                        double m2 = m_measurements->getMeasurement(i)->swrGraphCalib.value(fq2).value;
//                        if(m1 > 10)
//                            m1 = 10;
//                        if(m2 > 10)
//                            m2 = 10;
//                        dSwr = interpolate(fq1, fq0, fq2, m1, m2);
                        dSwr = interpolate(fq1, fq0, fq2, mm->swrGraphCalib.value(fq1).value, mm->swrGraphCalib.value(fq2).value);

//                        m1 = m_measurements->getMeasurement(i)->rlGraphCalib.value(fq1).value;
//                        m2 = m_measurements->getMeasurement(i)->rlGraphCalib.value(fq2).value;
//                        dRl = interpolate(fq1, fq0, fq2, m1, m2);
                        dRl = interpolate(fq1, fq0, fq2, mm->rlGraphCalib.value(fq1).value, mm->rlGraphCalib.value(fq2).value);
                    }else
                    {
                        dPhase = interpolate(fq1, fq0, fq2, mm->phaseGraph.value(fq1).value, mm->phaseGraph.value(fq2).value);
                        dR = interpolate(fq1, fq0, fq2, mm->rsrGraph.value(fq1).value, mm->rsrGraph.value(fq2).value);
                        dX = interpolate(fq1, fq0, fq2, mm->rsxGraph.value(fq1).value, mm->rsxGraph.value(fq2).value);
                        dRho = interpolate(fq1, fq0, fq2, mm->rhoGraph.value(fq1).value, mm->rhoGraph.value(fq2).value);
                        dRpar = interpolate(fq1, fq0, fq2, mm->rprGraph.value(fq1).value, mm->rprGraph.value(fq2).value);
                        dXpar = interpolate(fq1, fq0, fq2, mm->rpxGraph.value(fq1).value, mm->rpxGraph.value(fq2).value);
//                        double m1 = m_measurements->getMeasurement(i)->swrGraph.value(fq1).value;
//                        double m2 = m_measurements->getMeasurement(i)->swrGraph.value(fq2).value;
//                        if(m1 > 10)
//                            m1 = 10;
//                        if(m2 > 10)
//                            m2 = 10;
//                        dSwr = interpolate(fq1, fq0, fq2, m1, m2);
                        dSwr = interpolate(fq1, fq0, fq2, mm->swrGraph.value(fq1).value, mm->swrGraph.value(fq2).value);

//                        m1 = m_measurements->getMeasurement(i)->rlGraph.value(fq1).value;
//                        m2 = m_measurements->getMeasurement(i)->rlGraph.value(fq2).value;
//                        dRl = interpolate(fq1, fq0, fq2, m1, m2);
                        dRl = interpolate(fq1, fq0, fq2, mm->rlGraph.value(fq1).value, mm->rlGraph.value(fq2).value);
                    }

                    if (qIsNaN(dR) || (dR<0.001) )
                        dR = 0.01;
                    if (qIsNaN(dX))
                        dX = 0;
//                    dRpar = dR*(1+dX*dX/dR/dR);
//                    dXpar = dX*(1+dR*dR/dX/dX);
                    double dZpar = sqrt((dRpar*dRpar) + (dXpar*dXpar));

                    const double maxRp = VALUE_LIMIT;
                    if( dRpar > maxRp ) {
                        dRpar = maxRp;
                    }
                    if( dRpar < (-maxRp)) {
                        dRpar = -maxRp;
                    }
                    if( dXpar > maxRp ) {
                        dXpar = maxRp;
                    }
                    if( dXpar < (-maxRp)) {
                        dXpar = -maxRp;
                    }

//                    measurement* vmm = m_measurements->getMeasurementView(i);
//                    dZmod =  interpolate(fq1, fq0, fq2, vmm->rszGraph.value(fq1).value, vmm->rszGraph.value(fq2).value);
//                    dZpar =  interpolate(fq1, fq0, fq2, vmm->rpzGraph.value(fq1).value, vmm->rpzGraph.value(fq2).value);
                    dZmod =  interpolate(fq1, fq0, fq2, mm->rszGraph.value(fq1).value, mm->rszGraph.value(fq2).value);
                    dZpar =  interpolate(fq1, fq0, fq2, mm->rpzGraph.value(fq1).value, mm->rpzGraph.value(fq2).value);

                    zString+= QString::number(dR,'f', 2);
                    if(dX >= 0)
                    {
                        if (dX > maxRp)
                            dX = maxRp; // HUCK
                        zString+= " + j";
                        zString+= QString::number(dX,'f', 2);
                    }else
                    {
                        if (dX < -maxRp)
                            dX = -maxRp; // HUCK
                        zString+= " - j";
                        zString+= QString::number((dX * (-1)),'f', 2);
                    }
                    zparString += QString::number(dRpar,'f', 2);
                    if(dXpar >= 0)
                    {
                        if (dX > maxRp)
                            dX = maxRp; // HUCK
                        zparString+= " + j";
                        zparString+= QString::number(dXpar,'f', 2);
                    }else
                    {
                        if (dX < -maxRp)
                            dX = -maxRp; // HUCK
                        zparString+= " - j";
                        zparString+= QString::number((dXpar * (-1)),'f', 2);
                    }

                    dL = 1E9 * dX / (2*M_PI * fq0 * 1E3);//nH
                    dC = 1E12 / (2*M_PI * fq0 * (dX * (-1)) * 1E3);//pF

                    dLpar = 1E9 * dXpar / (2*M_PI * fq0 * 1E3);
                    dCpar = 1E12 / (2*M_PI * fq0 * (dXpar * (-1)) * 1E3);

                }
            }
            for (int j=MarkersHeaderColumn::fieldFQ+1; j<_columnTypes.size(); j++) {
                switch (_columnTypes[j]) {
                case MarkersHeaderColumn::fieldSWR:
                    row << QVariant(dSwr);
                    break;
                case MarkersHeaderColumn::fieldRL:
                    row << QVariant(dRl);
                    break;
                case MarkersHeaderColumn::fieldPhase:
                    row << QVariant(dPhase);
                    break;
                case MarkersHeaderColumn::fieldR:
                    row << QVariant(dR);
                    break;
                case MarkersHeaderColumn::fieldX:
                    row << QVariant(dX);
                    break;
                case MarkersHeaderColumn::fieldL:
                    row << QVariant(dL);
                    break;
                case MarkersHeaderColumn::fieldC:
                    row << QVariant(dC);
                    break;
                case MarkersHeaderColumn::fieldRpar:
                    row << QVariant(dRpar);
                    break;
                case MarkersHeaderColumn::fieldXpar:
                    row << QVariant(dXpar);
                    break;
                case MarkersHeaderColumn::fieldLpar:
                    row << QVariant(dLpar);
                    break;
                case MarkersHeaderColumn::fieldCpar:
                    row << QVariant(dCpar);
                    break;
                case MarkersHeaderColumn::fieldRho:
                    row << QVariant(dRho);
                    break;
                case MarkersHeaderColumn::fieldZ:
                    row << QVariant(zString);
                    break;
                case MarkersHeaderColumn::fieldZpar:
                    row << QVariant(zparString);
                    break;
                case MarkersHeaderColumn::fieldZmod:
                    row << QVariant(dZmod);
                    break;
                default:
                    row << QVariant();
                    break;
                }
            }
            info << row;
        } // for (m_measurements)
    } // for (m_markersList)
    return info;
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
    changeMarkersHint();
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
            m_markersHint->setVisible(false);
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
    if(m_currentTab == "tab_swr")
    {
        m_swrWidget->replot();
    }else if(m_currentTab == "tab_phase")
    {
        m_phaseWidget->replot();
    }else if(m_currentTab == "tab_rs")
    {
        m_rsWidget->replot();
    }else if(m_currentTab == "tab_rp")
    {
        m_rpWidget->replot();
    }else if(m_currentTab == "tab_rl")
    {
        m_rlWidget->replot();
    }else if(m_currentTab == "tab_s21")
    {
        m_s21Widget->replot();
    }else if(m_currentTab == "tab_tdr")
    {
        m_tdrWidget->replot();
    }else if(m_currentTab == "tab_smith")
    {
        m_smithWidget->replot();
    }
#ifndef NO_MULTITAB
    else if(m_currentTab == "tab_multi")
    {
        QString old_m_currentTab = m_currentTab;
        const QList<QString>& tabs = MainWindow::m_mainWindow->multiTabs();
        foreach (const QString& tab, tabs) {
            QCustomPlot* plot = MainWindow::m_mainWindow->plotForTab(tab);
            plot->replot();
        }
        m_currentTab = old_m_currentTab;
    }
#endif
}

void Markers::on_removeMarker(int number)
{
    if (!m_markersList.isEmpty()) {
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
    }
    changeMarkersHint();
    redraw();
    if (m_markersList.isEmpty()) {
       m_markersHint->setVisible(false);
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

void Markers::changeMarkersHint()
{
    if (m_markersList.size() != 0) {
        m_markersHint->updateMarkers(m_markersList.size(), m_measurements->getMeasurementLength());
        repaint();
    }
}

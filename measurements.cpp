#include "measurements.h"
#include "ProgressDlg.h"
#include "export.h"
#include "mainwindow.h"
#include "CustomPlot.h"
#include "customgraph.h"
#include "glwidget.h"

extern bool g_developerMode;
int g_maxMeasurements = MAX_MEASUREMENTS;

QVector<QColor> generateColors(int number) {
    const int MAX_COLOR = 360;
    const int MIN_COLOR = 0;
    QVector<QColor> colors;
    double jump = (MAX_COLOR-MIN_COLOR) / (number*1.0);
    for (int i = 0; i < number; i++) {
        int h = (int)(MIN_COLOR + (jump*i));
        colors.append(QColor::fromHsv(h, 255, 255));
    }
    return colors;
}

QColor getColor(int _index)
{
    static QColor colors[] = {
        QColor(30, 40, 255, 150),
        QColor(30, 255, 40, 150),
        QColor(255, 30, 40, 150),
        QColor(255, 127, 0, 255),
        QColor(255, 40, 255, 150)
    };
    if (_index >=0 && _index < 5)
        return colors[_index];

    int colorCount = g_maxMeasurements-4;
    const int MAX_COLOR = 360;
    const int MIN_COLOR = 0;
    double jump = (MAX_COLOR-MIN_COLOR) / (colorCount*1.0);
    int h = (int)(MIN_COLOR + (jump*(_index-5)));

    return QColor::fromHsv(h, 255, 255);
}

Measurements::Measurements(QObject *parent) : QObject(parent),
    m_currentIndex(0),
    m_graphHint(NULL),
    m_graphBriefHint(NULL),
    m_swrLine(NULL),
    m_swrLine2(NULL),
    m_phaseLine(NULL),
    m_phaseLine2(NULL),
    m_rsLine(NULL),
    m_rpLine(NULL),
    m_rlLine(NULL),
    m_rlLine2(NULL),
    m_tdrLine(NULL),
    m_settings(NULL),
    m_calibration(NULL),
    m_graphHintEnabled(true),
    m_graphBriefHintEnabled(true),
    m_calibrationMode(false),
    m_Z0(50),
    m_dotsNumber(50),
    m_smithTracer(NULL)
{    
    QString path = Settings::setIniFile();
    m_settings = new QSettings(path,QSettings::IniFormat);
    m_settings->beginGroup("Measurements");
    m_graphHintEnabled = m_settings->value("GraphHintEnabled",true).toBool();
    m_graphBriefHintEnabled = m_settings->value("GraphBriefHintEnabled",true).toBool();
    m_settings->endGroup();

    m_settings->beginGroup("Cable");
    m_cableVelFactor = m_settings->value("VelFactor",0.66 ).toDouble();
    m_settings->endGroup();

    m_pdTdrImp =  new double[TDR_MAXARRAY];
    m_pdTdrStep =  new double[TDR_MAXARRAY];

    if(m_graphHint == NULL)
    {
        m_graphHint = new PopUp();
        m_graphHint->setHiding(false);
        m_settings->beginGroup("Settings");
        bool darkTheme = m_settings->value("darkColorTheme", true).toBool();
        m_settings->endGroup();

        changeColorTheme(darkTheme);

        m_graphHint->setPopupText(tr("Frequency = \n"
                                     "SWR = \n"
                                     "RL = \n"
                                     "Z = \n"
                                     "|Z| = \n"
                                     "|rho| = \n"
                                     "C = \n"
                                     "Zpar = \n"
                                     "Cpar = \n"
                                     "Cable: "));
        if(m_graphHintEnabled)
        {
            m_graphHint->show();
        }
        m_graphHint->setName(tr("Hint"));
    }

    if(m_graphBriefHint == NULL)
    {
        m_graphBriefHint = new PopUp();
        m_graphBriefHint->setHiding(false);
        //m_graphBriefHint->setPopupText("0\n0");
        m_graphBriefHint->setName(tr("BriefHint"));
    }
}

Measurements::~Measurements()
{
    m_settings->beginGroup("Measurements");
    m_settings->setValue("GraphHintEnabled",m_graphHintEnabled);
    m_settings->setValue("GraphBriefHintEnabled",m_graphBriefHintEnabled);
    m_settings->endGroup();

    delete []m_pdTdrImp;
    delete []m_pdTdrStep;

    if(m_graphHint)
    {
        delete m_graphHint;
    }
    if (m_graphBriefHint)
    {
        delete m_graphBriefHint;
    }
}

void Measurements::setWidgets(QCustomPlot * swr,   QCustomPlot * phase,
                              QCustomPlot * rs,    QCustomPlot * rp,
                              QCustomPlot * rl,    QCustomPlot * tdr,
                              QCustomPlot * smith, QTableWidget * table)
{
    m_swrWidget = swr;
    m_phaseWidget = phase;
    m_rsWidget = rs;
    m_rsWidget->legend->setVisible(true);
    m_rsWidget->legend->removeAt(0);
    m_rpWidget = rp;
    m_rpWidget->legend->setVisible(true);
    m_rpWidget->legend->removeAt(0);
    m_rlWidget = rl;
    m_tdrWidget = tdr;
    m_tdrWidget->legend->setVisible(true);
    m_tdrWidget->legend->removeAt(0);
    m_smithWidget = smith;
    m_tableWidget = table;
    drawSmithImage();

    if(m_graphBriefHint != NULL)
    {
        m_graphBriefHint->setPenColor(QColor(0,0,0,0));
        m_graphBriefHint->setBackgroundColor(QColor(0,0,0,0));
        m_graphBriefHint->setTextColor("black");
    }
}

void Measurements::setUserWidget(QCustomPlot * user) {
    m_userWidget = user;
    m_userWidget->legend->setVisible(true);
    m_userWidget->legend->removeAt(0);
}

void Measurements::setCalibration(Calibration * _calibration)
{
    m_calibration = _calibration;
}

bool Measurements::getCalibrationEnabled(void)
{
    return ((m_calibration != nullptr) && (m_calibration->getCalibrationEnabled()));
}

void Measurements::deleteRow(int row)
{
    m_tableNames.remove(row, 1);
    m_tableWidget->removeRow(row);

    for(int j = 0; j < 5; ++j)
    {
        if(m_tableNames.length() > j)
        {
            QTableWidgetItem *item = m_tableWidget->item(j,0);
            if(item == NULL)
            {
                item = new QTableWidgetItem();
                QString str = m_tableNames.at(j);
                item->setText(str);
                m_tableWidget->setItem(j,0, item);
            }else
            {
                QString str = m_tableNames.at(j);
                item->setText(str);
            }
        }else
        {
            QTableWidgetItem *item = m_tableWidget->item(j,0);
            if(item != NULL)
            {
                delete item;
            }
        }
    }
    int count = m_swrWidget->graphCount();
    if(count)
    {
        int row_ = row+1;
        delete m_measurements[row].smithCurve;
        measurement mm = m_measurements[row];
        m_measurements.removeAt(row);
        m_viewMeasurements.removeAt(row);
        m_farEndMeasurementsAdd.removeAt(row);
        m_farEndMeasurementsSub.removeAt(row);

        m_swrWidget->removeGraph(row_);
        m_phaseWidget->removeGraph(row_);
        m_rsWidget->removeGraph(1+row*3);
        m_rsWidget->removeGraph(1+row*3);
        m_rsWidget->removeGraph(1+row*3);
        m_rpWidget->removeGraph(1+row*3);
        m_rpWidget->removeGraph(1+row*3);
        m_rpWidget->removeGraph(1+row*3);
        m_rlWidget->removeGraph(row_);
        m_tdrWidget->removeGraph(1+row*2);
        m_tdrWidget->removeGraph(1+row*2);
        if (g_developerMode) {
            int index = getBaseUserGraphIndex(row);
            int cnt = mm.userGraphs.size();
            for (int i=0; i<cnt; i++)
                m_userWidget->removeGraph(index);
        }

        if(row == m_tableNames.length())
        {
            QModelIndex myIndex = m_tableWidget->model()->index( row-1, 0,
                                                                 QModelIndex());
            m_tableWidget->selectionModel()->select(myIndex,
                                                    QItemSelectionModel::Select);

            myIndex = m_tableWidget->model()->index( row-1, 1, QModelIndex());
            m_tableWidget->selectionModel()->select(myIndex,
                                                    QItemSelectionModel::Select);

        }else
        {
            QModelIndex myIndex = m_tableWidget->model()->index( row, 0,
                                                                 QModelIndex());
            m_tableWidget->selectionModel()->select(myIndex,
                                                    QItemSelectionModel::Select);

            myIndex = m_tableWidget->model()->index( row, 1, QModelIndex());
            m_tableWidget->selectionModel()->select(myIndex,
                                                    QItemSelectionModel::Select);

        }
    }
    m_tableWidget->setRowCount(m_tableNames.length());
}

void Measurements::on_newMeasurement(QString name, qint64 fq, qint64 sw, qint32 dots)
{
    on_newMeasurement(name);

    m_measurements.last().set(fq, sw, dots);
    m_viewMeasurements.last().set(fq, sw, dots);
    m_farEndMeasurementsAdd.last().set(fq, sw, dots);
    m_farEndMeasurementsSub.last().set(fq, sw, dots);

    double range = sw;
    double center = fq;
    QString fmt;
    if (m_RangeMode)
    {
        fmt = tr("FQ:%1kHz SW:%2kHz Points:%3");
        range = (sw - fq)/2.0;
        center = fq + range;
    } else {
        fmt = tr("Start:%1kHz Stop:%2kHz Points:%3");
    }
    QString tips = QString(fmt)
            .arg((long)(center/1000))
            .arg((long)(range/1000))
            .arg(dots);
    QTableWidgetItem *item = m_tableWidget->item(m_tableWidget->rowCount()-1,0);
    item->setToolTip(tips);
    m_measuringInProgress = true;
    qDebug() << "Measurements::on_newMeasurement";
}

void Measurements::on_newMeasurement(QString name)
{
    m_interrupted = false;
    // name.isEmpty -> singlePoint measurement
    if (!name.isEmpty())
    {
        if(m_graphBriefHintEnabled)
        {
            m_graphBriefHint->show();
        }

        QString nextName = name;
        if (name.indexOf("##") == 0)
        {
            int next = 0;
            for (int idx=0; idx<m_tableNames.size(); idx++)
            {
                QString existed = m_tableNames[idx];
                if (existed.indexOf('>') == 2) {
                    QString num = existed.left(2);
                    bool ok = false;
                    int prefix = num.toInt(&ok);
                    if (ok) {
                        next = qMax(next, prefix);
                    }
                }
            }
            next++;
            if (next > 99)
                next = 1;

            nextName = QString("%1> %2").arg(next, 2, 10, QChar('0')).arg(name.mid(2));
        }
        while(m_tableNames.length() >= g_maxMeasurements)
        {
            m_tableNames.remove(0,1);
        }
        m_tableNames.append(nextName);
        //if(m_tableNames.length() > m_tableWidget->rowCount())
        {
            m_tableWidget->setRowCount(m_tableNames.length());
        }
        for(int i = 0; i < m_tableNames.length(); ++i)
        {
            QTableWidgetItem *item = m_tableWidget->item(i,0);
            if(item == NULL)
            {
                item = new QTableWidgetItem();
                QString str = m_tableNames.at(i);
                item->setText(str);
                m_tableWidget->setItem(i,0, item);
            }else
            {
                QString str = m_tableNames.at(i);
                item->setText(str);
            }
        }

        m_tableWidget->reset();
        QModelIndex myIndex = m_tableWidget->model()->index( m_tableNames.size()-1, 0, QModelIndex());
        m_tableWidget->selectionModel()->select(myIndex,QItemSelectionModel::Select);
        m_tableWidget->scrollToBottom();
    }

    while(m_measurements.length() >= g_maxMeasurements)
    {
        measurement mm = m_measurements.takeFirst();
        delete mm.smithCurve;
        delete m_viewMeasurements.takeFirst().smithCurve;
        delete m_farEndMeasurementsAdd.takeFirst().smithCurve;
        delete m_farEndMeasurementsSub.takeFirst().smithCurve;
        m_swrWidget->removeGraph(1);
        m_phaseWidget->removeGraph(1);
        m_rsWidget->removeGraph(1);
        m_rsWidget->removeGraph(1);
        m_rsWidget->removeGraph(1);
        m_rpWidget->removeGraph(1);
        m_rpWidget->removeGraph(1);
        m_rpWidget->removeGraph(1);
        m_rlWidget->removeGraph(1);
        m_tdrWidget->removeGraph(1);
        m_tdrWidget->removeGraph(1);

        if (g_developerMode) {
            int count = mm.userGraphs.size();
            for (int i=0; i<count; i++)
                m_userWidget->removeGraph(1);
        }
    }
    m_measurements.append( measurement());
    m_viewMeasurements.append( measurement());
    m_farEndMeasurementsAdd.append( measurement());
    m_farEndMeasurementsSub.append( measurement());

    QPen pen;
    if(m_swrWidget->graphCount() > 1)
    {
        pen = m_swrWidget->graph()->pen();
        pen.setWidth(3);
        m_swrWidget->graph()->setPen(pen);
        m_phaseWidget->graph()->setPen(pen);
        m_rlWidget->graph()->setPen(pen);
        m_smithWidget->graph()->setPen(pen);
        m_measurements.at(m_measurements.length()-2).smithCurve->setPen(pen);
    }    
    m_swrWidget->addGraph();
    m_swrWidget->graph()->setAntialiasedFill(false);
    m_phaseWidget->addGraph();

    m_rsWidget->setAutoAddPlottableToLegend(m_rsWidget->legend->itemCount() < 3);
    m_rsWidget->addGraph();
    m_rsWidget->graph()->setName("R");
    m_rsWidget->addGraph();
    m_rsWidget->graph()->setName("X");
    m_rsWidget->addGraph();
    m_rsWidget->graph()->setName("|Z|");
    m_rpWidget->setAutoAddPlottableToLegend(m_rpWidget->legend->itemCount() < 3);
    //m_rpWidget->addGraph();
    qobject_cast<CustomPlot*>(m_rpWidget)->addGraph();
    m_rpWidget->graph()->setName("Rp");
    m_rpWidget->addGraph();
    m_rpWidget->graph()->setName("Xp");
    m_rpWidget->addGraph();
    m_rpWidget->graph()->setName("|Zp|");
    m_rlWidget->addGraph();
    m_tdrWidget->setAutoAddPlottableToLegend(m_tdrWidget->legend->itemCount() < 2);
    m_tdrWidget->addGraph();
    m_tdrWidget->graph()->setName(tr("Impulse response"));
    m_tdrWidget->addGraph();
    m_tdrWidget->graph()->setName(tr("Step response"));

    m_measurements.last().smithCurve = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    m_viewMeasurements.last().smithCurve = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    m_farEndMeasurementsAdd.last().smithCurve = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    m_farEndMeasurementsSub.last().smithCurve = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);

    if(++m_currentIndex >= g_maxMeasurements+1)
    {
        m_currentIndex = 1;
    }
//    switch (m_currentIndex) {
//    case 1:
//        pen.setColor(QColor(30, 40, 255, 150));
//        break;
//    case 2:
//        pen.setColor(QColor(30, 255, 40, 150));
//        break;
//    case 3:
//        pen.setColor(QColor(255, 30, 40, 150));
//        break;
//    case 4:
//        pen.setColor(QColor(255, 127, 0, 255));
//        break;
//    case 5:
//        pen.setColor(QColor(255, 40, 255, 150));
//        break;
//    default:
//        break;
//    }
    pen.setColor(getColor(m_currentIndex));
    pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);

    m_swrWidget->setBackgroundScaled(true);

    m_swrWidget->graph()->setPen(pen);
    m_phaseWidget->graph()->setPen(pen);
    m_rlWidget->graph()->setPen(pen);
    m_smithWidget->graph()->setPen(pen);
    m_measurements.last().smithCurve->setPen(pen);

    int rsGraphCount = m_rsWidget->graphCount();
    int tdrGraphCount = m_tdrWidget->graphCount();

    QPen rpen;
    rpen.setColor(QColor(255, 30, 40, 150));
    rpen.setWidthF(3);
    QPen xpen;
    xpen.setColor(QColor(30, 255, 40, 150));
    xpen.setWidthF(3);
    QPen zpen;
    zpen.setColor(QColor(30, 40, 255, 150));
    zpen.setWidthF(3);

    m_rsWidget->graph(rsGraphCount-3)->setPen(rpen);
    m_rsWidget->graph(rsGraphCount-2)->setPen(xpen);
    m_rsWidget->graph(rsGraphCount-1)->setPen(zpen);

    m_rpWidget->graph(rsGraphCount-3)->setPen(rpen);
    m_rpWidget->graph(rsGraphCount-2)->setPen(xpen);
    m_rpWidget->graph(rsGraphCount-1)->setPen(zpen);

    m_tdrWidget->graph(tdrGraphCount-2)->setPen(zpen);
    m_tdrWidget->graph(tdrGraphCount-1)->setPen(xpen);
}


void Measurements::on_continueMeasurement(qint64 fq, qint64 sw, qint32 dots)
{
    Q_UNUSED (fq);
    Q_UNUSED (sw);
    Q_UNUSED (dots);

    qDebug() << "Measurements::on_continueMeasurement" << fq << sw << dots;

    m_isContinuing = true;
    m_currentPoint = 0;

    delete m_measurements.last().smithCurve;
    delete m_viewMeasurements.last().smithCurve;
    delete m_farEndMeasurementsAdd.last().smithCurve;
    delete m_farEndMeasurementsSub.last().smithCurve;
    //m_measurements.last().dataRX.clear();
    //m_measurements.last().dataRXCalib.clear();
    m_viewMeasurements.last().dataRX.clear();
    m_viewMeasurements.last().dataRXCalib.clear();
    m_farEndMeasurementsAdd.last().dataRX.clear();
    m_farEndMeasurementsAdd.last().dataRXCalib.clear();
    m_farEndMeasurementsSub.last().dataRX.clear();
    m_farEndMeasurementsSub.last().dataRXCalib.clear();

    m_measurements.last().smithCurve = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    m_viewMeasurements.last().smithCurve = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    m_farEndMeasurementsAdd.last().smithCurve = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    m_farEndMeasurementsSub.last().smithCurve = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
}

void Measurements::on_newAnalyzerData(rawData _rawData)
{
    on_newData(_rawData, false);
}

void Measurements::on_newDataRedraw(rawData _rawData)
{
    on_newData(_rawData, true);
}

void Measurements::on_newUserDataHeader(QStringList fields)
{
    m_measurements.last().fieldsUser.clear();
    if (fields.isEmpty())
        return;
    m_measurements.last().fieldsUser.append(fields);
    QVector<QColor> colors = generateColors(fields.size());

    m_userWidget->setAutoAddPlottableToLegend(m_userWidget->legend->itemCount() < fields.size());
    for (int i=0; i<fields.size(); i++) {
        m_measurements.last().userGraphs.append(new QCPDataMap());
        m_viewMeasurements.last().userGraphs.append(new QCPDataMap());
        QCPGraph* grp = m_userWidget->addGraph();
        grp->setName(fields.at(i));
        QColor color = colors.takeFirst();
        QPen pen(color);
        pen.setWidth(3);
        grp->setPen(pen);
    }
}

void Measurements::on_newUserData(rawData _rawData, UserData _userData)
{
    on_newData(_rawData);

    m_measurements.last().dataUser.append(_userData);
    for (int idx=0; idx<_userData.values.size(); idx++) {
        QCPData qcpData;
        qcpData.key = _userData.fq*1000;
        qcpData.value = _userData.values.at(idx);
        QCPDataMap* map = m_measurements.last().userGraphs.at(idx);
        map->insert(qcpData.key,qcpData);
        QCPDataMap* vmap = m_viewMeasurements.last().userGraphs.at(idx);
        vmap->insert(qcpData.key,qcpData);
    }
    QVector <double> x,y;
    x.append(_userData.fq*1000);
    x.append(_userData.fq*1000);
    y.append(m_userWidget->yAxis->getRangeLower());
    y.append(m_userWidget->yAxis->getRangeUpper());
    m_userWidget->graph(0)->setData(x,y);

    on_redrawGraphs(true);
}

void Measurements::on_newData(rawData _rawData, bool _redraw)
{
    if (m_oneFqMode) {
        GraphData _data;
        GraphData _calibData;
        prepareGraphs(_rawData, _data, _calibData);
        updateOneFqWidget(getCalibrationEnabled() ? _calibData : _data);
        return;
    }

    if(m_calibrationMode)
    {
        return;
    }

    // fix popup hint bug
    if (m_isContinuing) {
        if (m_currentPoint < m_measurements.last().dataRX.size()) {
            m_measurements.last().dataRX[m_currentPoint] = _rawData;
        } else {
            m_measurements.last().dataRX.append(_rawData);
        }
    } else {
        m_measurements.last().dataRX.append(_rawData);
    }

    updateTDRProgress(m_measurements.last().dataRX.size());

    double VSWR;
    double RL;
    if(!computeSWR(_rawData.fq, m_Z0,_rawData.r,_rawData.x,&VSWR,&RL))
    {
        if(m_measurements.last().swrGraph.size() > 0)
        {
            VSWR = m_measurements.last().swrGraph.last().value;
            RL = m_measurements.last().rlGraph.last().value;
        }else
        {
            return;
        }
    }
    double maxSwr = m_swrWidget->yAxis->range().upper;
    double maxRs = m_rsWidget->yAxis->range().upper;
    double maxRp = m_rpWidget->yAxis->range().upper;

    QVector <double> x,y;
    double fq = _rawData.fq*1000;

    x.append(fq);
    x.append(fq);
    y.append(MIN_SWR);
    y.append(MAX_SWR);

    QCPData data;
    data.key = fq;
    data.value = VSWR;

    m_measurements.last().swrGraph.insert(data.key,data);

    m_swrWidget->graph(0)->setData(x,y);

    y.clear();
    y.append(m_phaseWidget->yAxis->getRangeLower());
    y.append(m_phaseWidget->yAxis->getRangeUpper());
    m_phaseWidget->graph(0)->setData(x,y);

    y.clear();
    y.append(m_rsWidget->yAxis->getRangeLower());
    y.append(m_rsWidget->yAxis->getRangeUpper());
    m_rsWidget->graph(0)->setData(x,y);

    y.clear();
    y.append(m_rpWidget->yAxis->getRangeLower());
    y.append(m_rpWidget->yAxis->getRangeUpper());
    m_rpWidget->graph(0)->setData(x,y);

    y.clear();
    y.append(m_rlWidget->yAxis->getRangeLower());
    y.append(m_rlWidget->yAxis->getRangeUpper());
    m_rlWidget->graph(0)->setData(x,y);
//------------------------------------------------------------------------------
//------------------RXZ---------------------------------------------------------
//------------------------------------------------------------------------------
    double R = _rawData.r;
    double X = _rawData.x;
    double Z = computeZ(R, X);

    data.value = R;
    m_measurements.last().rsrGraph.insert(data.key,data);
    if( R > maxRs )
    {
        data.value = maxRs;
    }else if( R < (-maxRs) )
    {
        data.value = -maxRs;
    }
    m_viewMeasurements.last().rsrGraph.insert(data.key,data);

    data.value = X;
    m_measurements.last().rsxGraph.insert(data.key,data);
    if( X > maxRs )
    {
        data.value = maxRs;
    }else if( X < (-maxRs) )
    {
        data.value = -maxRs;
    }
    m_viewMeasurements.last().rsxGraph.insert(data.key,data);

    data.value = Z;
    m_measurements.last().rszGraph.insert(data.key,data);
    if( Z > maxRs )
    {
        data.value = maxRs;
    }else if( Z < (-maxRs) )
    {
        data.value = -maxRs;
    }
    m_viewMeasurements.last().rszGraph.insert(data.key,data);
//------------------------------------------------------------------------------
//------------------RXZ par-----------------------------------------------------
//------------------------------------------------------------------------------
    if (qIsNaN(R) || (R<0.001) )
    {
        R = 0.01;
    }
    if (qIsNaN(X))
    {
        X = 0;
    }
    double Rpar = R*(1+X*X/R/R);
    double Xpar = X*(1+R*R/X/X);
    double Zpar = computeZ(R, X);

    data.value = Rpar;
    m_measurements.last().rprGraph.insert(data.key,data);
    if( Rpar > maxRp )
    {
        data.value = maxRp;
    }else if( Rpar < (-maxRp) )
    {
        data.value = -maxRp;
    }
    m_viewMeasurements.last().rprGraph.insert(data.key,data);

    data.value = Xpar;
    m_measurements.last().rpxGraph.insert(data.key,data);
    if( Xpar > maxRp )
    {
        data.value = maxRp;
    }else if( Xpar < (-maxRp) )
    {
        data.value = -maxRp;
    }
    m_viewMeasurements.last().rpxGraph.insert(data.key,data);

    data.value = Zpar;
    m_measurements.last().rpzGraph.insert(data.key,data);
    if( Zpar > maxRp )
    {
        data.value = maxRp;
    }else if( Zpar < (-maxRp) )
    {
        data.value = -maxRp;
    }
    m_viewMeasurements.last().rpzGraph.insert(data.key,data);

    data.value = RL;
    m_measurements.last().rlGraph.insert(data.key,data);

//------------------------------------------------------------------------------
//----------------------calc phase----------------------------------------------
//------------------------------------------------------------------------------

    if (qIsNaN(_rawData.r) || (_rawData.r<0.001) )
    {
        _rawData.r = 0.01;
    }
    if (qIsNaN(_rawData.x))
    {
        _rawData.x = 0;
    }
    double Rnorm = _rawData.r/m_Z0;
    double Xnorm = _rawData.x/m_Z0;
    double Denom = (Rnorm+1)*(Rnorm+1)+Xnorm*Xnorm;
    double RhoReal = ((Rnorm-1)*(Rnorm+1)+Xnorm*Xnorm)/Denom;
    double RhoImag = 2*Xnorm/Denom;
    double RhoPhase = atan2(RhoImag, RhoReal) / M_PI * 180.0;
    double RhoMod = sqrt(RhoReal*RhoReal+RhoImag*RhoImag);
    data.value = RhoPhase;
    m_measurements.last().phaseGraph.insert(data.key,data);
    data.value = RhoMod;
    m_measurements.last().rhoGraph.insert(data.key,data);
//------------------------------------------------------------------------------
//----------------------calc smith----------------------------------------------
//------------------------------------------------------------------------------
    double pointX,pointY;
    NormRXtoSmithPoint(R/m_Z0, X/m_Z0, pointX, pointY);
    double len = m_measurements.last().dataRX.length();
    m_measurements.last().smithGraph.insert(len, QCPCurveData(len, pointX, pointY));
    len = m_measurements.last().dataRX.length()*2 - 1;
    m_measurements.last().smithGraphView.insert(len, QCPCurveData(len, pointX, pointY));

//------------------------------------------------------------------------------
//----------------------Calc calibration if performed---------------------------
//------------------------------------------------------------------------------
    if(m_calibration != NULL)
    {
        if(m_calibration->getCalibrationPerformed())
        {
            R = _rawData.r;
            X = _rawData.x;

            double Gre = (R*R-m_Z0*m_Z0+X*X)/((R+m_Z0)*(R+m_Z0)+X*X);
            double Gim = (2*m_Z0*X)/((R+m_Z0)*(R+m_Z0)+X*X);

            double GreOut;
            double GimOut;

            double SOR =  1; double SOI = 0; // Ideal model
            double SSR = -1; double SSI = 0;
            double SLR =  0; double SLI = 0;

            double COR, COI; // CalibrationReOpen, CalibrationImOpen
            double CSR, CSI; // CalibrationReShort, CalibrationImShort
            double CLR, CLI; // CalibrationReLoad, CalibrationImLoad
            bool res = m_calibration->interpolateS(_rawData.fq, COR, COI, CSR, CSI, CLR, CLI);
//            COR = 1;
//            COI = 0;
//            CSR = -1;
//            CSI = 0;
//            CLR = 0;
//            CLI = 0;

            if (!res)
            {
                SOR =  1; SOI = 0; // Ideal model
                SSR = -1; SSI = 0;
                SLR =  0; SLI = 0;
            }
            m_calibration->applyCalibration(Gre,Gim,  // Measured
                                            COR,COI,CSR,CSI,CLR,CLI, // Measured parameters of cal standards
                                            SOR,SOI,SSR,SSI,SLR,SLI, // Actual (Ideal) parameters of cal standards
                                            GreOut,GimOut); // Actual

            double calR = (1-GreOut*GreOut-GimOut*GimOut)/((1-GreOut)*(1-GreOut)+GimOut*GimOut);
            calR *= m_Z0;
            double calX = (2*GimOut)/((1-GreOut)*(1-GreOut)+GimOut*GimOut);
            calX *= m_Z0;
            double calZ = computeZ(calR,calX);

            rawData rawDataCalib = _rawData;
            rawDataCalib.r = calR;
            rawDataCalib.x = calX;

            //m_measurements.last().dataRXCalib.append(rawDataCalib);
            if (m_isContinuing) {
                if (m_currentPoint < m_measurements.last().dataRXCalib.size()) {
                    m_measurements.last().dataRXCalib[m_currentPoint] = rawDataCalib;
                } else {
                    m_measurements.last().dataRXCalib.append(rawDataCalib);
                }
            } else {
                m_measurements.last().dataRXCalib.append(rawDataCalib);
            }

            computeSWR(_rawData.fq, m_Z0, calR, calX,&VSWR,&RL);

            data.value = VSWR;
            m_measurements.last().swrGraphCalib.insert(data.key,data);
            if( VSWR > maxSwr )
            {
                data.value = maxSwr;
            }
            m_viewMeasurements.last().swrGraphCalib.insert(data.key,data);

            data.value = calR;
            m_measurements.last().rsrGraphCalib.insert(data.key,data);
            if( calR > maxRs )
            {
                data.value = maxRs;
            }else if( calR < (-maxRs) )
            {
                data.value = -maxRs;
            }
            m_viewMeasurements.last().rsrGraphCalib.insert(data.key,data);

            data.value = calX;
            m_measurements.last().rsxGraphCalib.insert(data.key,data);
            if( calX > maxRs )
            {
                data.value = maxRs;
            }else if( calX < (-maxRs) )
            {
                data.value = -maxRs;
            }
            m_viewMeasurements.last().rsxGraphCalib.insert(data.key,data);

            data.value = calZ;
            m_measurements.last().rszGraphCalib.insert(data.key,data);
            if( calZ > maxRs )
            {
                data.value = maxRs;
            }else if( calZ < (-maxRs) )
            {
                data.value = -maxRs;
            }
            m_viewMeasurements.last().rszGraphCalib.insert(data.key,data);


            double calRpar = calR*(1+calX*calX/calR/calR);
            double calZpar = computeZ(calRpar, calX);
            data.value = calRpar;
            m_measurements.last().rprGraphCalib.insert(data.key,data);
            if( calRpar > maxRp )
            {
                data.value = maxRp;
            }else if( calRpar < (-maxRp) )
            {
                data.value = -maxRp;
            }
            m_viewMeasurements.last().rprGraphCalib.insert(data.key,data);

            data.value = calX;
            m_measurements.last().rpxGraphCalib.insert(data.key,data);
            if( calX > maxRp )
            {
                data.value = maxRp;
            }else if( calX < (-maxRp) )
            {
                data.value = -maxRp;
            }
            m_viewMeasurements.last().rpxGraphCalib.insert(data.key,data);

            data.value = calZpar;
            m_measurements.last().rpzGraphCalib.insert(data.key,data);
            if( calZpar > maxRp )
            {
                data.value = maxRp;
            }else if( calZpar < (-maxRp) )
            {
                data.value = -maxRp;
            }
            m_viewMeasurements.last().rpzGraphCalib.insert(data.key,data);

            data.value = RL;
            m_measurements.last().rlGraphCalib.insert(data.key,data);

            //----------------------calc phase---------------------------
            if (qIsNaN(calR) || (calR<0.001) )
            {
                calR = 0.01;
            }
            if (qIsNaN(calX))
            {
                calX = 0;
            }
            Rnorm = calR/m_Z0;
            Xnorm = calX/m_Z0;

            Denom = (Rnorm+1)*(Rnorm+1)+Xnorm*Xnorm;
            RhoReal = ((Rnorm-1)*(Rnorm+1)+Xnorm*Xnorm)/Denom;
            RhoImag = 2*Xnorm/Denom;

            RhoPhase = atan2(RhoImag, RhoReal) / M_PI * 180.0;            
            RhoMod = sqrt(RhoReal*RhoReal+RhoImag*RhoImag);

            QString msg = QString("f=%1, r=%2, x=%3, RhoPhase=%4")
                    .arg(_rawData.fq, 0, 'f', 4, QLatin1Char(' '))
                    .arg(calR, 0, 'f', 4, QLatin1Char(' '))
                    .arg(calX, 0, 'f', 4, QLatin1Char(' '))
                    .arg(RhoPhase, 0, 'f', 4, QLatin1Char(' '));
            //qDebug() << "RhoPhase: " << msg;

            data.value = RhoPhase;
            m_measurements.last().phaseGraphCalib.insert(data.key,data);
            data.value = RhoMod;
            m_measurements.last().rhoGraphCalib.insert(data.key,data);
            //----------------------calc phase end---------------------------
            //----------------------calc smith-------------------------------

            double ptX,ptY;
            //NormRXtoSmithPoint(R/m_Z0, X/m_Z0, ptX, ptY);
            NormRXtoSmithPoint(Rnorm, Xnorm, ptX, ptY);
            int len = m_measurements.last().dataRX.length();
            m_measurements.last().smithGraphCalib.insert(len, QCPCurveData(len, ptX, ptY));
            len = m_measurements.last().dataRX.length()*2 - 1;
            m_measurements.last().smithGraphViewCalib.insert(len, QCPCurveData(len, ptX, ptY));
            //----------------------calc smith end---------------------------
        }
    }
    m_currentPoint++;
    if (isTDRMode())
        return;

    //qint64 t1 = QDateTime::currentMSecsSinceEpoch();
    if (!_redraw)
        return;
    on_redrawGraphs(m_measuringInProgress && !m_isContinuing);
    //qDebug() << "on_newData: calc " << (t1-t0) << " msec, draw " << (QDateTime::currentMSecsSinceEpoch()-t1) << " msec";
}

void Measurements::prepareGraphs(rawData _rawData, GraphData& _data, GraphData& _calibData)
{
    _data.FQ = _rawData.fq;
    _data.R = _rawData.r;
    _data.X = _rawData.x;

    computeSWR(_rawData.fq, m_Z0,_data.R,_data.X,&_data.SWR,&_data.RL);
    _data.Z = computeZ(_data.R,_data.X);

    //------------------RXZ par-----------------------------------------------------
    double R = _rawData.r;
    double X = _rawData.x;
    if (qIsNaN(R) || (R<0.001) )
        R = 0.01;
    if (qIsNaN(X))
        X = 0;
    _data.Rpar = R*(1+X*X/R/R);
    _data.Xpar = X*(1+R*R/X/X);
    _data.Zpar = computeZ(R, X);

    //----------------------calc phase----------------------------------------------
    double Rnorm = R/m_Z0;
    double Xnorm = X/m_Z0;
    double Denom = (Rnorm+1)*(Rnorm+1)+Xnorm*Xnorm;
    double RhoReal = ((Rnorm-1)*(Rnorm+1)+Xnorm*Xnorm)/Denom;
    double RhoImag = 2*Xnorm/Denom;
    double RhoPhase = atan2(RhoImag, RhoReal) / M_PI * 180.0;
    double RhoMod = sqrt(RhoReal*RhoReal+RhoImag*RhoImag);
    _data.RhoPhase = RhoPhase;
    _data.RhoMod = RhoMod;

    //----------------------Calc calibration if performed---------------------------
    if(m_calibration != NULL)
    {
        if(m_calibration->getCalibrationPerformed())
        {
            _calibData.FQ = _rawData.fq;
            R = _rawData.r;
            X = _rawData.x;
            double Gre = (R*R-m_Z0*m_Z0+X*X)/((R+m_Z0)*(R+m_Z0)+X*X);
            double Gim = (2*m_Z0*X)/((R+m_Z0)*(R+m_Z0)+X*X);

            double GreOut;
            double GimOut;

            double SOR =  1; double SOI = 0; // Ideal model
            double SSR = -1; double SSI = 0;
            double SLR =  0; double SLI = 0;

            double COR, COI; // CalibrationReOpen, CalibrationImOpen
            double CSR, CSI; // CalibrationReShort, CalibrationImShort
            double CLR, CLI; // CalibrationReLoad, CalibrationImLoad
            bool res = m_calibration->interpolateS(_rawData.fq, COR, COI, CSR, CSI, CLR, CLI);
//            COR = 1;
//            COI = 0;
//            CSR = -1;
//            CSI = 0;
//            CLR = 0;
//            CLI = 0;

            if (!res)
            {
                SOR =  1; SOI = 0; // Ideal model
                SSR = -1; SSI = 0;
                SLR =  0; SLI = 0;
            }
            m_calibration->applyCalibration(Gre,Gim,  // Measured
                                            COR,COI,CSR,CSI,CLR,CLI, // Measured parameters of cal standards
                                            SOR,SOI,SSR,SSI,SLR,SLI, // Actual (Ideal) parameters of cal standards
                                            GreOut,GimOut); // Actual

            double calR = (1-GreOut*GreOut-GimOut*GimOut)/((1-GreOut)*(1-GreOut)+GimOut*GimOut);
            calR *= m_Z0;
            double calX = (2*GimOut)/((1-GreOut)*(1-GreOut)+GimOut*GimOut);
            calX *= m_Z0;
            double calZ = computeZ(calR,calX);

            _calibData.R = calR;
            _calibData.X = calX;
            _calibData.Z = calZ;
            computeSWR(_calibData.FQ, m_Z0, calR, calX, &_calibData.SWR, &_calibData.RL);

            double calRpar = calR*(1+calX*calX/calR/calR);
            double calZpar = computeZ(calRpar, calX);

            _calibData.Rpar = calRpar;
            _calibData.Xpar = calX;
            _calibData.Zpar = calZpar;

            if (qIsNaN(calR) || (calR<0.001) )
                calR = 0.01;
            if (qIsNaN(calX))
                calX = 0;
            Rnorm = calR/m_Z0;
            Xnorm = calX/m_Z0;

            Denom = (Rnorm+1)*(Rnorm+1)+Xnorm*Xnorm;
            RhoReal = ((Rnorm-1)*(Rnorm+1)+Xnorm*Xnorm)/Denom;
            RhoImag = 2*Xnorm/Denom;

            RhoPhase = atan2(RhoImag, RhoReal) / M_PI * 180.0;
            RhoMod = sqrt(RhoReal*RhoReal+RhoImag*RhoImag);

            _calibData.RhoPhase = RhoPhase;
            _calibData.RhoMod = RhoMod;
        }
    }
}


quint32 Measurements::computeSWR(double freq, double Z0, double R, double X, double *VSWR, double *RL)
{
    Q_UNUSED(freq);

    if (R <= 0)
    {
        R = 0.001;
    }
    double SWR, Gamma;
    double XX = X * X;								// always >= 0
    double denominator = (R + Z0) * (R + Z0) + XX;

    if (denominator == 0)
    {
        return 0;
    }
    Gamma = sqrt(((R - Z0) * (R - Z0) + XX) / denominator);
    if (Gamma == 1.0)
    {
        return 0;
    }
    SWR = (1 + Gamma) / (1 - Gamma);

    if ((SWR > 200) || (Gamma > 0.99))
    {
        SWR = 200;
    } else if (SWR < 1)
    {
        SWR = 1;
    }

    if (VSWR)
    {
        *VSWR = SWR;
    }
    if (RL)
    {
        if (Gamma == 0)
        {
            return 0;
        }
        *RL = -20 * log10(Gamma);
    }
    return 1;
}

double Measurements::computeZ (double R, double X)
{
    return sqrt((R*R) + (X*X));
}

void Measurements::on_currentTab(QString name)
{
    m_currentTab = name;
    on_redrawGraphs();
}

void Measurements::setGraphHintEnabled(bool enabled)
{
    m_graphHintEnabled = enabled;
    showHideHints();
}

void Measurements::setGraphBriefHintEnabled(bool enabled)
{
    m_graphBriefHintEnabled = enabled;
    showHideHints();
}

void Measurements::on_focus(bool focus)
{
    m_focus = focus;
    showHideHints();
}

void Measurements::hideGraphBriefHint()
{
    if(m_graphBriefHint)
    {
        m_graphBriefHint->focusHide();
    }
}

void Measurements::showHideHints()
{
    if(m_graphHint)
    {
        if(m_graphHintEnabled && m_focus)
        {
            m_graphHint->focusShow();
        }else
        {
            m_graphHint->focusHide();
        }
    }
    if(m_graphBriefHint)
    {
        if(m_graphBriefHintEnabled && m_focus)
        {
            m_graphBriefHint->focusShow();
        }else
        {
            m_graphBriefHint->focusHide();
        }
    }
}

void Measurements::on_newCursorFq(double x, int number, int mouseX, int mouseY)
{
    updatePopUp(x, number, mouseX, mouseY);
}

void Measurements::on_newCursorSmithPos (double x, double y, int index)
{
    QCPCurveDataMap map;
    if((m_calibration != NULL) && (m_calibration->getCalibrationEnabled()))
    {
        map = m_measurements.at(index).smithGraphCalib;
    }else
    {
        map = m_measurements.at(index).smithGraph;
    }
    QList <QCPCurveData> dataList = map.values();
    if(dataList.isEmpty())
    {
        return;
    }
    int findedNum = 0;
    bool finded = false;
    for(double n = 0; n < 6; n+=0.1)
    {
        for(int i = 0; i < dataList.length(); ++i)
        {
            if( ((x >= dataList.at(i).key - n) && (x <= dataList.at(i).key + n)) &&
                 (y >= dataList.at(i).value - n) && (y <= dataList.at(i).value + n))
            {
                findedNum = i;
                finded = true;
                break;
            }
        }
        if(finded)
        {
            break;
        }
    }
    if(m_smithTracer == NULL)
    {
        m_smithTracer = new QCPItemEllipse(m_smithWidget);
    }
    m_smithTracer->setAntialiased(true);
    QPen pen;
    pen.setColor(QColor(250,30,20,180));
    pen.setWidth(4);
    m_smithTracer->setPen(pen);
    m_smithTracer->topLeft->setCoords(dataList.at(findedNum).key-0.1, dataList.at(findedNum).value+0.1);
    m_smithTracer->bottomRight->setCoords(dataList.at(findedNum).key+0.1, dataList.at(findedNum).value-0.1);
    m_smithWidget->replot();



    QCPDataMap *swrmap;
    if((m_calibration != NULL) && (m_calibration->getCalibrationEnabled()))
    {
        swrmap = &(m_measurements[index].swrGraphCalib);
    }else
    {
        swrmap = &(m_measurements[index].swrGraph);
    }
    QList <double> swrkeys = swrmap->keys();

    double frequency = 0;
    double swr = 0;
    double phase = 0;
    double rho = 0;
    double rl = 0;
    double l = 0;
    double c = 0;
    double lpar = 0;
    double cpar = 0;
    double r = 0;
    double x1 = 0;
    double z = 0;
    double rpar = 0;
    double xpar = 0;
    QString zString;
    QString zparString;

    frequency = findedNum < swrkeys.size() ? swrkeys.at(findedNum) : swrkeys.last();

    if(m_calibration->getCalibrationEnabled())
    {
        if(m_farEndMeasurement == 1)
        {
            rho = m_farEndMeasurementsSub.at(index).rhoGraph.value(frequency).value;
            phase = m_farEndMeasurementsSub.at(index).phaseGraph.value(frequency).value;
            r = m_farEndMeasurementsSub.at(index).rsrGraph.value(frequency).value;//dataRX.at(previousI).r;
            x1 = m_farEndMeasurementsSub.at(index).rsxGraph.value(frequency).value;//dataRX.at(previousI).x;
        }else if(m_farEndMeasurement == 2)
        {
            rho = m_farEndMeasurementsAdd.at(index).rhoGraph.value(frequency).value;
            phase = m_farEndMeasurementsAdd.at(index).phaseGraph.value(frequency).value;
            r = m_farEndMeasurementsAdd.at(index).rsrGraph.value(frequency).value;//dataRX.at(previousI).r;
            x1 = m_farEndMeasurementsAdd.at(index).rsxGraph.value(frequency).value;//dataRX.at(previousI).x;
        }else
        {
            rho = m_measurements.at(index).rhoGraph.value(frequency).value;
            phase = m_measurements.at(index).phaseGraphCalib.value(frequency).value;
            r = m_measurements.at(index).dataRXCalib.at(findedNum).r;
            x1 = m_measurements.at(index).dataRXCalib.at(findedNum).x;
        }
        swr = m_measurements.at(index).swrGraphCalib.value(frequency).value;
        rl = m_measurements.at(index).rlGraphCalib.value(frequency).value;
        z = m_viewMeasurements.at(index).rszGraphCalib.value(frequency).value;
        rpar = m_viewMeasurements.at(index).rprGraphCalib.value(frequency).value;
        xpar = m_viewMeasurements.at(index).rpxGraphCalib.value(frequency).value;
    }else
    {
        if(m_farEndMeasurement == 1)
        {
            rho = m_farEndMeasurementsSub.at(index).rhoGraph.value(frequency).value;
            phase = m_farEndMeasurementsSub.at(index).phaseGraph.value(frequency).value;
            r = m_farEndMeasurementsSub.at(index).rsrGraph.value(frequency).value;//dataRX.at(previousI).r;
            x1 = m_farEndMeasurementsSub.at(index).rsxGraph.value(frequency).value;//dataRX.at(previousI).x;
        }else if(m_farEndMeasurement == 2)
        {
            rho = m_farEndMeasurementsAdd.at(index).rhoGraph.value(frequency).value;
            phase = m_farEndMeasurementsAdd.at(index).phaseGraph.value(frequency).value;
            r = m_farEndMeasurementsAdd.at(index).rsrGraph.value(frequency).value;//dataRX.at(previousI).r;
            x1 = m_farEndMeasurementsAdd.at(index).rsxGraph.value(frequency).value;//dataRX.at(previousI).x;
        }else
        {
            rho = m_measurements.at(index).rhoGraph.value(frequency).value;
            phase = m_measurements.at(index).phaseGraph.value(frequency).value;
            r = m_measurements.at(index).dataRX.at(findedNum).r;
            x1 = m_measurements.at(index).dataRX.at(findedNum).x;
        }
        swr = m_measurements.at(index).swrGraph.value(frequency).value;
        rl = m_measurements.at(index).rlGraph.value(frequency).value;
        z = m_viewMeasurements.at(index).rszGraph.value(frequency).value;
        rpar = m_viewMeasurements.at(index).rprGraph.value(frequency).value;
        xpar = m_viewMeasurements.at(index).rpxGraph.value(frequency).value;
    }

    zString+= QString::number(r,'f', 2);
    if(x1 >= 0)
    {
        if (x1 > 2000)
            x1 = 2000; // HUCK
        zString+= " + j";
        zString+= QString::number(x1,'f', 2);
    }else
    {
        if (x1 < -2000)
            x1 = -2000; // HUCK
        zString+= " - j";
        zString+= QString::number((x1 * (-1)),'f', 2);
    }
    zparString+= QString::number(rpar,'f', 2);
    if(x1 >= 0)
    {
        zparString+= " + j";
        zparString+= QString::number(xpar,'f', 2);
    }else
    {
        zparString+= " - j";
        zparString+= QString::number((xpar * (-1)),'f', 2);
    }
    l = 1E9 * x1 / (2*M_PI * frequency * 1E3);//nH
    c = 1E12 / (2*M_PI * frequency * (x1 * (-1)) * 1E3);//pF

    lpar = 1E9 * xpar / (2*M_PI * frequency * 1E3);//nH
    cpar = 1E12 / (2*M_PI * frequency * (xpar * (-1)) * 1E3);//pF

    QString str = QString::number(frequency);
    int pos = str.indexOf(".");
    if(pos < 0)
    {
        if(str.length() > 6)
        {
            str.insert(str.length()-6," ");
        }
        if(str.length() > 3)
        {
            str.insert(str.length()-3," ");
        }
    }else
    {
        int len = str.length() - pos;
        if((str.length()-len) > 6)
        {
            str.insert(str.length()-len-6," ");
        }
        if((str.length()-len) > 3)
        {
            str.insert(str.length()-len-3," ");
        }
    }


    QString text;
    if(x1 > 0)
    {
        text = QString(tr("Frequency = %1 kHz\n"
                          "SWR = %2\n"
                          "RL = %3 dB\n"
                          "Z = %4 Ohm\n"
                          "|Z| = %5 Ohm\n"
                          "|rho| = %6, phase = %7 °\n"
                          "L = %8 nH\n"
                          "Zpar = %9 Ohm\n"
                          "Lpar = %10 nH\n"))
        .arg(str)
        .arg(QString::number(swr,'f', 2))
        .arg(QString::number(rl,'f', 2))
        .arg(zString)
        .arg(QString::number(z,'f', 2))
        .arg(QString::number(rho,'f', 2))
        .arg(QString::number(phase,'f', 2))
        .arg(QString::number(l,'f', 2))
        .arg(zparString)
        .arg(QString::number(lpar,'f', 2));
    }else
    {
        text = QString(tr("Frequency = %1 kHz\n"
                          "SWR = %2\n"
                          "RL = %3 dB\n"
                          "Z = %4 Ohm\n"
                          "|Z| = %5 Ohm\n"
                          "|rho| = %6, phase = %7 °\n"
                          "C = %8 pF\n"
                          "Zpar = %9 Ohm\n"
                          "Cpar = %10 pF\n"))
        .arg(str)
        .arg(QString::number(swr,'f', 2))
        .arg(QString::number(rl,'f', 2))
        .arg(zString)
        .arg(QString::number(z,'f', 2))
        .arg(QString::number(rho,'f', 2))
        .arg(QString::number(phase,'f', 2))
        .arg(QString::number(c,'f', 2))
        .arg(zparString)
        .arg(QString::number(cpar,'f', 2));
    }

    if(!m_farEndMeasurement)
    {
        QString cableString;
        QString lenUnits;
        if(m_measureSystemMetric)
        {
            lenUnits = tr("m");
        }else
        {
            lenUnits = tr("ft");
        }

        double len14 = SPEEDOFLIGHT/4/frequency/1000*m_cableVelFactor;
        double len12 = SPEEDOFLIGHT/2/frequency/1000*m_cableVelFactor;

        if(!m_measureSystemMetric)
        {
            len14 *= FEETINMETER;
            len12 *= FEETINMETER;
        }

        cableString = tr("Cable: length(1/4) = %1 %2, length(1/2) = %3 %4")
                .arg(QString::number(len14,'f',2))
                .arg(lenUnits)
                .arg(QString::number(len12,'f',2))
                .arg(lenUnits);
        text += cableString;
    }

    m_graphHint->setPopupText(text);
}

void Measurements::updatePopUp(double xPos, int index, int mouseX, int mouseY)
{
#define DELTA 5
    if(m_graphHint)
    {
        if(m_currentTab == "tab_6")
        {
            QCPDataMap *tdrmapImp;
            QCPDataMap *tdrmapStep;
            double pdTdrImp;
            double pdTdrStep;
            if(m_farEndMeasurement == 1)
            {
                if(m_measureSystemMetric)
                {
                    tdrmapImp = &(m_farEndMeasurementsSub[index].tdrImpGraph);
                    tdrmapStep = &(m_farEndMeasurementsSub[index].tdrStepGraph);
                }else
                {
                    tdrmapImp = &(m_farEndMeasurementsSub[index].tdrImpGraphFeet);
                    tdrmapStep = &(m_farEndMeasurementsSub[index].tdrStepGraphFeet);
                }
            }else if(m_farEndMeasurement == 2)
            {
                if(m_measureSystemMetric)
                {
                    tdrmapImp = &(m_farEndMeasurementsAdd[index].tdrImpGraph);
                    tdrmapStep = &(m_farEndMeasurementsAdd[index].tdrStepGraph);
                }else
                {
                    tdrmapImp = &(m_farEndMeasurementsAdd[index].tdrImpGraphFeet);
                    tdrmapStep = &(m_farEndMeasurementsAdd[index].tdrStepGraphFeet);
                }
            }else
            {
                if(m_measureSystemMetric)
                {
                    tdrmapImp = &(m_measurements[index].tdrImpGraph);
                    tdrmapStep = &(m_measurements[index].tdrStepGraph);
                }else
                {
                    tdrmapImp = &(m_measurements[index].tdrImpGraphFeet);
                    tdrmapStep = &(m_measurements[index].tdrStepGraphFeet);
                }
            }

            QList <double> tdrkeys = tdrmapImp->keys();

            bool res = false;
            int start = m_previousI-DELTA;
            if(start < 0)
            {
                start = 0;
            }
            int stop = m_previousI+DELTA;
            if (stop > tdrkeys.length()-1)
            {
                stop = tdrkeys.length()-1;
            }
            for(int t = 0; t < 2; ++t)
            {
                if(t == 1)
                {
                    if(res)
                    {
                        break;
                    }
                    start = 0;
                    stop = tdrkeys.length()-1;
                }
                double place;
                for(int i = start; i < stop; ++i)
                {
                    if((tdrkeys.at(i) <= xPos) && (tdrkeys.at(i+1) >= xPos))
                    {
                        double center = (tdrkeys.at(i) + tdrkeys.at(i+1))/2;
                        if( xPos > center )
                        {
                            if(m_previousI == i+1)
                            {
                                return;
                            }
                            place = tdrkeys.at(i+1);
                            m_previousI = i+1;
                        }else
                        {
                            if(m_previousI == i)
                            {
                                return;
                            }
                            place = tdrkeys.at(i);
                            m_previousI = i;
                        }

                        /*
                        if(m_farEndMeasurement == 1)
                        {
                            pdTdrImp = m_farEndMeasurementsSub[index].tdrImpGraph.value(place).value;
                            pdTdrStep = m_farEndMeasurementsSub[index].tdrStepGraph.value(place).value;
                        }else if(m_farEndMeasurement == 2)
                        {
                            pdTdrImp = m_farEndMeasurementsAdd[index].tdrImpGraph.value(place).value;
                            pdTdrStep = m_farEndMeasurementsAdd[index].tdrStepGraph.value(place).value;
                        }else
                        {
                            pdTdrImp = m_measurements[index].tdrImpGraph.value(place).value;
                            pdTdrStep = m_measurements[index].tdrStepGraph.value(place).value;
                        }
                        */
                        pdTdrImp = tdrmapImp->value(place).value;
                        pdTdrStep = tdrmapStep->value(place).value;

                        if(!m_tdrLine)
                        {
                            m_tdrLine = new QCPItemStraightLine(m_tdrWidget);
                            m_tdrLine->setAntialiased(false);
                            m_tdrWidget->addItem(m_tdrLine);
                        }
                        m_tdrLine->point1->setCoords(place, -1);
                        m_tdrLine->point2->setCoords(place, 1);

                        double Z = m_Z0*(1+pdTdrStep)/(1-pdTdrStep);
                        if (Z<0)
                        {
                            Z = 0;
                        }

                        QString text;
                        QString distance;

                        QString lenUnits;
                        QString timeNs;
                        distance = QString::number(place,'f',3);
                        double airLen = place/m_cableVelFactor;
                        QString distanceInAir = QString::number(airLen,'f',3);//FEETINMETER
                        if(m_measureSystemMetric)
                        {
                            lenUnits = "m";
                            timeNs = QString::number(airLen/0.299792458,'f',2);
                        }else
                        {
                            lenUnits = "ft";
                            timeNs = QString::number(airLen/FEETINMETER/0.299792458,'f',2);
                        }


                        QString ir = QString::number(pdTdrImp,'f',3);
                        QString sr = QString::number(pdTdrStep,'f',3);

                        QString zStr = QString::number(Z,'f',1);
                        text = QString(tr("Distance = %1 %2\n"
                                          "(distance in the air = %3 %4)\n"
                                          "Time = %5 ns\n"
                                          "Impulse response = %6\n"
                                          "Step response = %7\n"
                                          "|Z| = %8 Ohm"))
                                .arg(distance)//1
                                .arg(lenUnits)//2
                                .arg(distanceInAir)//3
                                .arg(lenUnits)//4
                                .arg(timeNs)//5
                                .arg(ir)//6
                                .arg(sr)//7
                                .arg(zStr);//8

                        m_graphHint->setPopupText(text);

                        res = true;
                        break;
                    }
                }
            }
        }else
        {
            if(m_graphBriefHint != NULL)
            {
                m_graphBriefHint->setPosition(mouseX+1,mouseY+1);
            }

            QCPDataMap *swrmap;
            if((m_calibration != NULL) && (m_calibration->getCalibrationEnabled()))
            {
                swrmap = &(m_measurements[index].swrGraphCalib);
            }else
            {
                swrmap = &(m_measurements[index].swrGraph);
            }
            QList <double> swrkeys = swrmap->keys();

            double frequency = 0;
            double swr = 0;
            double phase = 0;
            double rho = 0;
            double rl = 0;
            double l = 0;
            double c = 0;
            double lpar = 0;
            double cpar = 0;
            double r = 0;
            double z = 0;
            double x = 0;
            double rpar = 0;
            double xpar = 0;
            QString zString;
            QString zparString;
            bool res = false;
            int start = m_previousI-DELTA;
            if(start < 0)
            {
                start = 0;
            }
            int stop = m_previousI+DELTA;
            if (stop > swrkeys.length()-1)
            {
                stop = swrkeys.length()-1;
            }
            for(int t = 0; t < 2; ++t)
            {
                if(t == 1)
                {
                    if(res)
                    {
                        break;
                    }
                    start = 0;
                    stop = swrkeys.length()-1;
                }
                for(int i = start; i < stop; ++i)
                {
                    if((swrkeys.at(i) <= xPos) && (swrkeys.at(i+1) >= xPos))
                    {
                        double center = (swrkeys.at(i) + swrkeys.at(i+1))/2;
                        if( xPos > center )
                        {
                            if(m_previousI == i+1)
                            {
                                return;
                            }
                            frequency = swrkeys.at(i+1);
                            m_previousI = i+1;
                        }else
                        {
                            if(m_previousI == i)
                            {
                                return;
                            }
                            frequency = swrkeys.at(i);
                            m_previousI = i;
                        }

                        int dataSize = m_measurements.at(index).dataRX.size();

                        if(m_calibration->getCalibrationEnabled())
                        {
                            if(m_farEndMeasurement == 1)
                            {
                                swr = m_farEndMeasurementsSub.at(index).swrGraph.value(frequency).value;
                                rl = m_farEndMeasurementsSub.at(index).rlGraph.value(frequency).value;
                                rho = m_farEndMeasurementsSub.at(index).rhoGraph.value(frequency).value;
                                phase = m_farEndMeasurementsSub.at(index).phaseGraph.value(frequency).value;
                                r = m_farEndMeasurementsSub.at(index).rsrGraph.value(frequency).value;
                                x = m_farEndMeasurementsSub.at(index).rsxGraph.value(frequency).value;
                            }else if(m_farEndMeasurement == 2)
                            {
                                swr = m_farEndMeasurementsAdd.at(index).swrGraph.value(frequency).value;
                                rl = m_farEndMeasurementsAdd.at(index).rlGraph.value(frequency).value;
                                rho = m_farEndMeasurementsAdd.at(index).rhoGraph.value(frequency).value;
                                phase = m_farEndMeasurementsAdd.at(index).phaseGraph.value(frequency).value;
                                r = m_farEndMeasurementsAdd.at(index).rsrGraph.value(frequency).value;
                                x = m_farEndMeasurementsAdd.at(index).rsxGraph.value(frequency).value;
                            }else
                            {
                                swr = m_measurements.at(index).swrGraphCalib.value(frequency).value;
                                rl = m_measurements.at(index).rlGraphCalib.value(frequency).value;
                                rho = m_measurements.at(index).rhoGraph.value(frequency).value;
                                phase = m_measurements.at(index).phaseGraphCalib.value(frequency).value;
                                if (m_previousI < 0 || m_previousI >=dataSize)
                                    return;
                                r = m_measurements.at(index).dataRXCalib.at(m_previousI).r;
                                if (m_previousI < 0 || m_previousI >=dataSize)
                                    return;
                                x = m_measurements.at(index).dataRXCalib.at(m_previousI).x;
                            }
                            //swr = m_measurements.at(index).swrGraphCalib.value(frequency).value;
                            //rl = m_measurements.at(index).rlGraphCalib.value(frequency).value;
                            z = m_viewMeasurements.at(index).rszGraph.value(frequency).value;
                            rpar = m_viewMeasurements.at(index).rprGraph.value(frequency).value;
                            xpar = m_viewMeasurements.at(index).rpxGraph.value(frequency).value;
                        }else
                        {
                            if(m_farEndMeasurement == 1)
                            {
                                swr = m_farEndMeasurementsSub.at(index).swrGraph.value(frequency).value;
                                rl = m_farEndMeasurementsSub.at(index).rlGraph.value(frequency).value;
                                rho = m_farEndMeasurementsSub.at(index).rhoGraph.value(frequency).value;
                                phase = m_farEndMeasurementsSub.at(index).phaseGraph.value(frequency).value;
                                r = m_farEndMeasurementsSub.at(index).rsrGraph.value(frequency).value;
                                x = m_farEndMeasurementsSub.at(index).rsxGraph.value(frequency).value;
                            }else if(m_farEndMeasurement == 2)
                            {
                                swr = m_farEndMeasurementsAdd.at(index).swrGraph.value(frequency).value;
                                rl = m_farEndMeasurementsAdd.at(index).rlGraph.value(frequency).value;
                                rho = m_farEndMeasurementsAdd.at(index).rhoGraph.value(frequency).value;
                                phase = m_farEndMeasurementsAdd.at(index).phaseGraph.value(frequency).value;
                                r = m_farEndMeasurementsAdd.at(index).rsrGraph.value(frequency).value;
                                x = m_farEndMeasurementsAdd.at(index).rsxGraph.value(frequency).value;
                            }else
                            {
                                swr = m_measurements.at(index).swrGraph.value(frequency).value;
                                rl = m_measurements.at(index).rlGraph.value(frequency).value;
                                rho = m_measurements.at(index).rhoGraph.value(frequency).value;
                                phase = m_measurements.at(index).phaseGraph.value(frequency).value;
                                if (m_previousI < 0 || m_previousI >=dataSize)
                                    return;
                                r = m_measurements.at(index).dataRX.at(m_previousI).r;
                                if (m_previousI < 0 || m_previousI >=dataSize)
                                    return;
                                x = m_measurements.at(index).dataRX.at(m_previousI).x;
                            }
                            //swr = m_measurements.at(index).swrGraph.value(frequency).value;
                            //rl = m_measurements.at(index).rlGraph.value(frequency).value;
                            z = m_viewMeasurements.at(index).rszGraph.value(frequency).value;
                            rpar = m_viewMeasurements.at(index).rprGraph.value(frequency).value;
                            xpar = m_viewMeasurements.at(index).rpxGraph.value(frequency).value;
                        }

                        zString+= QString::number(r,'f', 2);
                        if(x >= 0)
                        {
                            if (x > 2000)
                                x = 2000; // HUCK
                            zString+= " + j";
                            zString+= QString::number(x,'f', 2);
                        }else
                        {
                            if (x < -2000)
                                x = -2000; // HUCK
                            zString+= " - j";
                            zString+= QString::number((x * (-1)),'f', 2);
                        }
                        zparString+= QString::number(rpar,'f', 2);
                        if(x >= 0)
                        {
                            zparString+= " + j";
                            zparString+= QString::number(xpar,'f', 2);
                        }else
                        {
                            zparString+= " - j";
                            zparString+= QString::number((xpar * (-1)),'f', 2);
                        }

                        l = 1E9 * x / (2*M_PI * frequency * 1E3);//nH
                        c = 1E12 / (2*M_PI * frequency * (x * (-1)) * 1E3);//pF

                        lpar = 1E9 * xpar / (2*M_PI * frequency * 1E3);
                        cpar = 1E12 / (2*M_PI * frequency * (xpar * (-1)) * 1E3);

                        res = true;
                        if(m_graphBriefHint != NULL)
                        {
                            QString str;
                            str = QString::number(frequency);
                            int idx = str.indexOf(".");
                            if(idx < 0)
                            {
                                if(str.length() > 6)
                                {
                                    str.insert(str.length()-6," ");
                                }
                                if(str.length() > 3)
                                {
                                    str.insert(str.length()-3," ");
                                }
                            }else
                            {
                                int len = str.length() - idx;
                                if((str.length()-len) > 6)
                                {
                                    str.insert(str.length()-len-6," ");
                                }
                                if((str.length()-len) > 3)
                                {
                                    str.insert(str.length()-len-3," ");
                                }
                            }
                            str += " kHz\n";
                            if(m_currentTab == "tab_1")
                            {
                                str += QString::number(swr,'f',2);
                            }else if(m_currentTab == "tab_2")
                            {
                                str += QString::number(phase,'f',2) + "°";
                            }else if(m_currentTab == "tab_3")
                            {
                                //str += QString::number(computeZ(r,x),'f',2);
                            }else if(m_currentTab == "tab_4")
                            {
                                //str += QString::number(computeZ(r,x),'f',2);
                            }else if(m_currentTab == "tab_5")
                            {
                                str += QString::number(rl,'f',2) + " dB";
                            }
                            m_graphBriefHint->setPopupText(str);
                        }
                        break;
                    }
                }
            }
            if(m_currentTab == "tab_1")
            {
                if(!m_swrLine)
                {
                    m_swrLine = new QCPItemStraightLine(m_swrWidget);
                    m_swrLine->setAntialiased(false);
                    m_swrWidget->addItem(m_swrLine);
                }
                if(!m_swrLine2)
                {
                    m_swrLine2 = new QCPItemStraightLine(m_swrWidget);
                    m_swrLine2->setAntialiased(false);
                    m_swrWidget->addItem(m_swrLine2);
                }
                m_swrLine->point1->setCoords(frequency, MIN_SWR);
                m_swrLine->point2->setCoords(frequency, MAX_SWR);

                m_swrLine2->point1->setCoords(m_swrWidget->yAxis->getRangeLower(), swr);
                m_swrLine2->point2->setCoords(m_swrWidget->yAxis->getRangeUpper(), swr);
            }else if(m_currentTab == "tab_2")
            {
                if(!m_phaseLine)
                {
                    m_phaseLine = new QCPItemStraightLine(m_phaseWidget);
                    m_phaseLine->setAntialiased(false);
                    m_phaseWidget->addItem(m_phaseLine);
                }
                if(!m_phaseLine2)
                {
                    m_phaseLine2 = new QCPItemStraightLine(m_phaseWidget);
                    m_phaseLine2->setAntialiased(false);
                    m_phaseWidget->addItem(m_phaseLine2);
                }
                m_phaseLine->point1->setCoords(frequency, -2000);
                m_phaseLine->point2->setCoords(frequency, 2000);
                m_phaseLine2->point1->setCoords(m_phaseWidget->yAxis->getRangeLower(), phase);
                m_phaseLine2->point2->setCoords(m_phaseWidget->yAxis->getRangeUpper(), phase);
            }else if(m_currentTab == "tab_3")
            {
                if(!m_rsLine)
                {
                    m_rsLine = new QCPItemStraightLine(m_rsWidget);
                    m_rsLine->setAntialiased(false);
                    m_rsWidget->addItem(m_rsLine);
                }
                m_rsLine->point1->setCoords(frequency, -2000);
                m_rsLine->point2->setCoords(frequency, 2000);
            }else if(m_currentTab == "tab_4")
            {
                if(!m_rpLine)
                {
                    m_rpLine = new QCPItemStraightLine(m_rpWidget);
                    m_rpLine->setAntialiased(false);
                    m_rpWidget->addItem(m_rpLine);
                }
                m_rpLine->point1->setCoords(frequency, -2000);
                m_rpLine->point2->setCoords(frequency, 2000);
            }else if(m_currentTab == "tab_5")
            {
                if(!m_rlLine)
                {
                    m_rlLine = new QCPItemStraightLine(m_rlWidget);
                    m_rlLine->setAntialiased(false);
                    m_rlWidget->addItem(m_rlLine);
                }
                if(!m_rlLine2)
                {
                    m_rlLine2 = new QCPItemStraightLine(m_rlWidget);
                    m_rlLine2->setAntialiased(false);
                    m_rlWidget->addItem(m_rlLine2);
                }
                m_rlLine->point1->setCoords(frequency, -2000);
                m_rlLine->point2->setCoords(frequency, 2000);
                m_rlLine2->point1->setCoords(m_rlWidget->yAxis->getRangeLower(), rl);
                m_rlLine2->point2->setCoords(m_rlWidget->yAxis->getRangeUpper(), rl);
            }

            QString str = QString::number(frequency);
            int idx = str.indexOf(".");
            if(idx < 0)
            {
                if(str.length() > 6)
                {
                    str.insert(str.length()-6," ");
                }
                if(str.length() > 3)
                {
                    str.insert(str.length()-3," ");
                }
            }else
            {
                int len = str.length() - idx;
                if((str.length()-len) > 6)
                {
                    str.insert(str.length()-len-6," ");
                }
                if((str.length()-len) > 3)
                {
                    str.insert(str.length()-len-3," ");
                }
            }

            QString text;
            if(x > 0)
            {
                text = QString(tr("Frequency = %1 kHz\n"
                                  "SWR = %2\n"
                                  "RL = %3 dB\n"
                                  "Z = %4 Ohm\n"
                                  "|Z| = %5 Ohm\n"
                                  "|rho| = %6, phase = %7 °\n"
                                  "L = %8 nH\n"
                                  "Zpar = %9 Ohm\n"
                                  "Lpar = %10 nH\n"))
                    .arg(str)
                    .arg(QString::number(swr,'f', 2))
                    .arg(QString::number(rl,'f', 2))
                    .arg(zString)
                    .arg(QString::number(z,'f', 2))
                    .arg(QString::number(rho,'f', 2))
                    .arg(QString::number(phase,'f', 2))
                    .arg(QString::number(l,'f', 2))
                    .arg(zparString)
                    .arg(QString::number(lpar,'f', 2));
            }else
            {
                text = QString(tr("Frequency = %1 kHz\n"
                                  "SWR = %2\n"
                                  "RL = %3 dB\n"
                                  "Z = %4 Ohm\n"
                                  "|Z| = %5 Ohm\n"
                                  "|rho| = %6, phase = %7 °\n"
                                  "C = %8 pF\n"
                                  "Zpar = %9 Ohm\n"
                                  "Cpar = %10 pF\n"))
                        .arg(str)
                        .arg(QString::number(swr,'f', 2))
                        .arg(QString::number(rl,'f', 2))
                        .arg(zString)
                        .arg(QString::number(z,'f', 2))
                        .arg(QString::number(rho,'f', 2))
                        .arg(QString::number(phase,'f', 2))
                        .arg(QString::number(c,'f', 2))
                        .arg(zparString)
                        .arg(QString::number(cpar,'f', 2));
            }

            if(!m_farEndMeasurement)
            {
                QString cableString;
                QString lenUnits;
                if(m_measureSystemMetric)
                {
                    lenUnits = tr("m");
                }else
                {
                    lenUnits = tr("ft");
                }

                double len14 = SPEEDOFLIGHT/4/frequency/1000*m_cableVelFactor;
                double len12 = SPEEDOFLIGHT/2/frequency/1000*m_cableVelFactor;

                if(!m_measureSystemMetric)
                {
                    len14 *= FEETINMETER;
                    len12 *= FEETINMETER;
                }

                cableString = tr("Cable: length(1/4) = %1 %2, length(1/2) = %3 %4")
                        .arg(QString::number(len14,'f',2))
                        .arg(lenUnits)
                        .arg(QString::number(len12,'f',2))
                        .arg(lenUnits);
                text += cableString;
            }
            m_graphHint->setPopupText(text);
        }
    }
    replot();
}

void Measurements::on_mainWindowPos(int x, int y)
{
    if(m_graphHint)
        m_graphHint->MainWindowPos(x, y);

    if (m_oneFqWidget)
        m_oneFqWidget->MainWindowPos(x, y);
}

void Measurements::setCalibrationMode(bool enabled)
{
    m_calibrationMode = enabled;
}

bool Measurements::getGraphHintEnabled(void)
{
    return m_graphHintEnabled;
}

void Measurements::on_calibrationEnabled(bool enabled)
{
    if(m_swrWidget->graphCount() == 1)
    {
        return;
    }

    int graphsCount = m_swrWidget->graphCount();
    for(int i = 1; i < graphsCount; ++i)
    {
        QCPDataMap swrmap;
        QCPDataMap rszmap;
        QCPDataMap rsxmap;
        QCPDataMap rsrmap;
        QCPDataMap rpzmap;
        QCPDataMap rpxmap;
        QCPDataMap rprmap;
        QCPDataMap rlmap;
        QCPDataMap phasemap;
        QCPCurveDataMap smithmap;
        int j = i-1;
        if(enabled)
        {
            swrmap = m_measurements.at(j).swrGraphCalib;
            phasemap = m_measurements.at(j).phaseGraphCalib;

            rszmap = m_measurements.at(j).rszGraphCalib;
            rsxmap = m_measurements.at(j).rsxGraphCalib;
            rsrmap = m_measurements.at(j).rsrGraphCalib;

            rpzmap = m_measurements.at(j).rpzGraphCalib;
            rpxmap = m_measurements.at(j).rpxGraphCalib;
            rprmap = m_measurements.at(j).rprGraphCalib;

            rlmap = m_measurements.at(j).rlGraphCalib;
            smithmap = m_measurements.at(j).smithGraphCalib;
        }else
        {
            swrmap = m_measurements.at(j).swrGraph;
            phasemap = m_measurements.at(j).phaseGraph;

            rszmap = m_measurements.at(j).rszGraph;
            rsxmap = m_measurements.at(j).rsxGraph;
            rsrmap = m_measurements.at(j).rsrGraph;

            rpzmap = m_measurements.at(j).rpzGraph;
            rpxmap = m_measurements.at(j).rpxGraph;
            rprmap = m_measurements.at(j).rprGraph;

            rlmap = m_measurements.at(j).rlGraph;
            smithmap = m_measurements.at(j).smithGraph;
        }

        m_swrWidget->graph(i)->setData(&swrmap, true);
        m_phaseWidget->graph(i)->setData(&phasemap, true);

        m_rsWidget->graph((i*3))->setData(&rszmap, true);
        m_rsWidget->graph((i*3)-1)->setData(&rsxmap, true);
        m_rsWidget->graph((i*3)-2)->setData(&rsrmap, true);

        m_rpWidget->graph((i*3))->setData(&rpzmap, true);
        m_rpWidget->graph((i*3)-1)->setData(&rpxmap, true);
        m_rpWidget->graph((i*3)-2)->setData(&rprmap, true);

        m_rlWidget->graph(i)->setData(&rlmap, true);
        //m_smithWidget->graph(i)->setData(&smithmap, true);
        m_measurements.at(j).smithCurve->setData(&smithmap, true);
    }
    replot();
    emit calibrationChanged();
}

void Measurements::saveData(quint32 number, QString path)
{
    if (number >= (quint32)g_maxMeasurements)
        number = g_maxMeasurements-1;

    if(path.indexOf(".asd") >= 0 )
    {
        QFile saveFile(path);

        if (!saveFile.open(QIODevice::WriteOnly))
        {
            qWarning("Couldn't open save file.");
            return;
        }

        QVector <rawData> data;
        if(m_calibration != NULL)
        {
            if(m_calibration->getCalibrationEnabled())
            {
                data = m_measurements.at(number).dataRXCalib;
            }else
            {
                data = m_measurements.at(number).dataRX;
            }
        }else
        {
            data = m_measurements.at(number).dataRX;
        }

        //Dots
        QJsonObject mainObj;
        mainObj["DotsNumber"] = data.length();

        //Measurements
        QJsonArray measurementsArray;
        for(int i = 0; i < data.length(); ++i)
        {
            QJsonObject obj;
            obj["fq"] = data.at(i).fq;
            obj["r"] = data.at(i).r;
            obj["x"] = data.at(i).x;
            measurementsArray.append(obj);
        }
        mainObj["Measurements"] = measurementsArray;

        QJsonDocument saveDoc(mainObj);

        saveFile.write(saveDoc.toJson());
    }
}

void Measurements::loadData(QString path)
{
    if(path.indexOf(".asd") >= 0 )
    {
        QStringList list;
        list = path.split("/");
        if(list.length() == 1)
        {
            list.clear();
            list = path.split("\\");
        }

        QFile loadFile(path);

        if (!loadFile.open(QIODevice::ReadOnly)) {
            QMessageBox::information(NULL, tr("Error"), tr("Couldn't open saved file."));
            qWarning("Couldn't open saved file.");
            return;
        }
        on_newMeasurement(list.last());

        QByteArray saveData = loadFile.readAll();

        QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
        QJsonObject mainObj = loadDoc.object();

        QJsonArray measureArray = mainObj["Measurements"].toArray();

        double fqMin = DBL_MAX;
        double fqMax = 0;
        int size = measureArray.size();

        ProgressDlg* progressDlg = new ProgressDlg();
        progressDlg->setValue(0);
        progressDlg->setProgressData(0, size, 1);
        progressDlg->updateActionInfo(tr("Load measurement"));
        progressDlg->updateStatusInfo(tr("please wait ...."));
        progressDlg->show();
        progressDlg->setWindowModality(Qt::WindowModal);
        QApplication::processEvents();

        for(int i = 0; i < size; ++i)
        {
            QJsonObject dataObject = measureArray[i].toObject();
            rawData data;
            data.read(dataObject);
            on_newData(data);
            fqMin = qMin(fqMin, data.fq);
            fqMax = qMax(fqMax, data.fq);
            if ((i%10) == 0) {
                progressDlg->setValue(i);
                progressDlg->updateStatusInfo(QString(tr("loaded %1 dots, from %2")).arg(i).arg(size));
            }
        }
        progressDlg->hide();
        delete progressDlg;

        emit import_finished(fqMin*1000, fqMax*1000);
    }else
    {
        importData(path);
    }
    on_redrawGraphs();
}

void Measurements::exportData(QString _name, int _type, int _number, bool _applyCable, QString _description)
{
    if (_number < 0 || m_measurements.isEmpty() || (_number >= m_measurements.size()))
        return;

    bool calibr = (m_calibration != nullptr) && (m_calibration->getCalibrationEnabled());
    QVector<rawData> vector;
    if (_applyCable)
    {
        switch(m_farEndMeasurement) {
        case 1:
            vector = calibr ? m_farEndMeasurementsSub.at(_number).dataRXCalib : m_farEndMeasurementsSub.at(_number).dataRX;
            break;
        case 2:
            vector = calibr ? m_farEndMeasurementsAdd.at(_number).dataRXCalib : m_farEndMeasurementsAdd.at(_number).dataRX;
            break;
        default:
            vector = calibr ? m_measurements.at(_number).dataRXCalib : m_measurements.at(_number).dataRX;
            break;
        }
    } else {
        vector = calibr ? m_measurements.at(_number).dataRXCalib : m_measurements.at(_number).dataRX;
    }
    exportData(_name, _type, vector, _description);
}

void Measurements::exportData(QString _name, int _type, QVector<rawData>& vector, QString _description)
{
    int len = vector.length();;

    if(_name.indexOf(".s1p") >= 0 )
    {
        QFile file(_name);

        if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))//if (!file.open(QFile::ReadWrite))
        {
            return;
        }

        QTextStream out(&file);

        out << "! Touchstone file generated by AntScope2";
        out << "\n";

        double Rswr = ((m_calibration != NULL) && (m_calibration->getCalibrationEnabled())) ? m_calibration->getZ0() : 50;

        if (_type == 0) // Z, RI
        {
            out << "# MHz Z RI R " << Rswr << "\n";
            out << "! Format: Frequency Z-real Z-imaginary (normalized to " << Rswr << " Ohm)\n";
        }else if (_type == 1) // S, RI
        {
            out << "# MHz S RI R " << Rswr << "\n";
            out << "! Format: Frequency S-real S-imaginary (normalized to " << Rswr << " Ohm)\n";
        }
        else if (_type == 2) // S, MA
        {
            out << "# MHz S MA R " << Rswr << "\n";
            out << "! Format: Frequency S-magnitude S-angle (normalized to " << Rswr << " Ohm, angle in degrees)\n";
        }

        if (!_description.isEmpty())
            out << _description << "\n";

        for (int i = 0; i < len; ++i)
        {
            QString s;

            s = QString("%1").arg(vector.at(i).fq, 0, 'f', 6);		// Fq
            out << s << " ";

            double R = vector.at(i).r;
            double X = vector.at(i).x;

            if (_type == 0) // Z, RI
            {
                if (!qIsNaN(R))
                    s = QString::number(R/Rswr,'g',4);           // R
                else
                    s = "0";
                out << s << " ";
                if (!qIsNaN(X))
                    s = QString::number(X/Rswr,'g',4);           // X
                else
                    s = "0";
                out << s << "\n";
            }
            else
            if (_type == 1) // S, RI
            {
                double Gre = (R*R-Rswr*Rswr+X*X)/((R+Rswr)*(R+Rswr)+X*X);
                double Gim = (2*Rswr*X)/((R+Rswr)*(R+Rswr)+X*X);

                if (!qIsNaN(Gre))
                    s = QString::number(Gre,'g',4);              // Real
                else
                    s = "0";
                out << s << " ";

                if (!qIsNaN(Gim))
                    s = QString::number(Gim,'g',4);              // Imaginary
                else
                    s = "0";
                out << s << "\n";

            }
            else
            if (_type == 2) // S, MA
            {
                double Gre = (R*R-Rswr*Rswr+X*X)/((R+Rswr)*(R+Rswr)+X*X);
                double Gim = (2*Rswr*X)/((R+Rswr)*(R+Rswr)+X*X);

                if (!qIsNaN(Gre))
                    s = QString::number(sqrt(Gre*Gre+Gim*Gim),'g',4);		// Magnitude
                else
                    s = "0";
                out << s << " ";

                if (!qIsNaN(Gim))
                    s = QString::number(atan2(Gim,Gre)/3.1415926*180.0,'g',4);		// Angle
                else
                    s = "0";
                out << s << "\n";
            }
        }
        out.flush();
    }else if(_name.indexOf(".csv") >= 0 )
    {
        QString str;
        QFile file(_name);
        bool result = file.open(QFile::ReadWrite);
        if(result)
        {
            extern bool g_developerMode;
            int rxWidth = g_developerMode ? 6 : 2;
            str = "#Frequency(MHz);R;X";
            file.write( str.toLocal8Bit(), str.length());
            file.write("\r\n", 2);
            for (int i = 0; i < len; ++i)
            {
                str = QString::number(vector.at(i).fq, 'f', 6) +
                "," +//";" +
                QString::number(vector.at(i).r,'f',rxWidth) +
                "," +//";" +
                QString::number(vector.at(i).x,'f',rxWidth);

                file.write( str.toLocal8Bit(), str.length());
                file.write("\r\n", 2);
            }
            file.close();
        }
    }else if(_name.indexOf(".nwl") >= 0 )
    {
        QString str;
        QFile file(_name);
        bool result = file.open(QFile::ReadWrite);
        if(result)
        {
            str = "/\"Freq(MHz)\" \"Rs\" \"Xs\"/";
            file.write( str.toLocal8Bit(), str.length());
            file.write("\r\n", 2);

            for (int i = 0; i < len; ++i)
            {
                str = QString::number(vector.at(i).fq, 'f', 6) +
                " " +//";" +
                QString::number(vector.at(i).r,'f',2) +
                " " +//";" +
                QString::number(vector.at(i).x,'f',2);

                file.write( str.toLocal8Bit(), str.length());
                file.write("\r\n", 2);
            }
            file.close();
        }
    }
}

void Measurements::importData(QString _name, bool /*user_format*/)
{
    QStringList list;
    list = _name.split("/");
    if(list.length() == 1)
    {
        list.clear();
        list = _name.split("\\");
    }
    on_newMeasurement(list.last());

    QFile file(_name);
    bool result = file.open(QFile::ReadOnly);
    if(result)
    {
        QString str = file.readAll();
        double fqMin = DBL_MAX;
        double fqMax = 0;
        QStringList nList = str.split('\n');

        str = nList.takeFirst();
        if (str.at(0) == '#') {
            str.replace('#', ' ');
            on_newUserDataHeader(str.trimmed().split(','));
        }

        while (!nList.isEmpty()) {
            str = nList.takeFirst();
            if (str.isEmpty())
                continue;
            QStringList fields = str.split(',');
            rawData rdata;
            UserData udata;
            bool ok;
            QString field = fields.takeFirst();
            rdata.fq = field.toDouble(&ok);
            if (!ok) {
                qDebug() << "***** ERROR: " << str;
                return;
            }
            udata.fq = rdata.fq;
            fqMin = qMin(fqMin, rdata.fq);
            fqMax = qMax(fqMax, rdata.fq);

            field = fields.takeFirst();
            rdata.r = field.toDouble(&ok);
            if (!ok) {
                qDebug() << "***** ERROR: " << str;
                return;
            }
            field = fields.takeFirst();
            rdata.x = field.toDouble(&ok);
            if (!ok) {
                qDebug() << "***** ERROR: " << str;
                return;
            }
            while (!fields.isEmpty()) {
                udata.values.append(fields.takeFirst().toDouble(&ok));
                if (!ok) {
                    qDebug() << "***** ERROR: " << str;
                    return;
                }
            }
            on_newUserData(rdata, udata);
        }
        emit import_finished(fqMin*1000, fqMax*1000);
        on_redrawGraphs();
    }
}

void Measurements::importData(QString _name)
{
    if(_name.indexOf(".s1p") >= 0 )
    {
        QStringList list;
        list = _name.split("/");
        if(list.length() == 1)
        {
            list.clear();
            list = _name.split("\\");
        }
        on_newMeasurement(list.last());

        QString sPathName = _name;

        if (sPathName.isEmpty())
        {
            return;
        }

        QFile ifs(sPathName);

        if (!ifs.open(QFile::ReadWrite))
        {
            return;
        }
        QTextStream in(&ifs);
        bool bGood = true;

        int iLines=0, iPoints=0;

        double  fqmul = 1000.0; // Default is GHz
        int iUnit = 1; // Default is S
        int iFormat = 1; // Default is MA

        QString line;//char str[1000]; // Whole string
        char strn[5][100]; // Substrings

        double f, param1, param2; // for reading S11 data lines

        double Z0 = 50;

        double fqMin = DBL_MAX;
        double fqMax = 0;
        do//while (ifs.isOpen() && (!ifs.eof()))
        {
            line = in.readLine();
            iLines++;

            if ( (line.length() > 2) && (line[0] == '#')) // Option line
            {
                line.remove(0,1);
                int ns = sscanf(line.toLocal8Bit(), "%s %s %s %s %s", strn[0], strn[1], strn[2], strn[3], strn[4]);
                for (int i=0; i<ns; i++)
                {
                    // Frequency unit

                    if (!strcmp(strn[i], "GHz"))
                        fqmul = 1000.0;
                    else
                    if (!strcmp(strn[i], "MHz"))
                        fqmul = 1.0;
                    else
                    if (!strcmp(strn[i], "KHz"))
                        fqmul = 0.001;
                    else
                    if (!strcmp(strn[i], "Hz"))
                        fqmul = 0.000001;
                    else

                    // Parameter

                    if (!strcmp(strn[i], "S"))
                        iUnit = 1;
                    else
                    if (!strcmp(strn[i], "Z"))
                        iUnit = 2;
                    else

                    // Format

                    if (!strcmp(strn[i], "MA"))
                        iFormat = 1;
                    else
                    if (!strcmp(strn[i], "RI"))
                        iFormat = 2;
                    else

                    // R n

                    if (!strcmp(strn[i], "R"))
                    {
                        if ( i < (ns-1) )
                        {
                            i++;

//                            setlocale(LC_NUMERIC,"C");
                            Z0 = atof(strn[i]);
//                            setlocale(LC_NUMERIC,"");

                            if ( (Z0<=0) || (Z0>10000) )
                            {
                                //bErr = true;
                                //break;
                                return;
                            }
                        }
                        else
                        {
                            //bErr = true;
                            //break;
                            return;
                        }
                    }
                    else
                    {
                        //bErr = true;
                        //break;
                        return;
                    }
                }

                // Check possible combinations
/*
                if ( (iUnit == 1) && (iFormat == 1) ) // S, MA
                {
                }
                else
                if ( (iUnit == 1) && (iFormat == 2) ) // S, RI
                {
                }
                else
                if ( (iUnit == 2) && (iFormat == 2) ) // Z, RI
                {
                }
                else
                {
                    bErr = true;
                }

                if (bErr == true)
                {
                    return;
                }
*/
                if(! ((iUnit == 1) && (iFormat == 1) // S, MA
                        || (iUnit == 1) && (iFormat == 2)  // S, RI
                        || (iUnit == 2) && (iFormat == 2)  // Z, RI
                    ))
                {
                    return;
                }

                continue;
            }

            if ( (strstr(line.toLocal8Bit(), "!") != NULL) || (strstr(line.toLocal8Bit(), ".") == NULL) ) // Comment or void line
                continue;

            // Scan data lines

            if ( sscanf(line.toLocal8Bit(), "%lf %lf %lf", &f, &param1, &param2) != 3)
            {
                return;
            }

            double r = 0,x = 0;

            if ( (iUnit == 1) && (iFormat == 1) ) // S, MA
            {
                    double Gr = param1 * cos(param2/180.0*M_PI);
                    double Gi = param1 * sin(param2/180.0*M_PI);

                    r = (1-Gr*Gr-Gi*Gi)/((1-Gr)*(1-Gr)+Gi*Gi);
                    x = (2*Gi)/((1-Gr)*(1-Gr)+Gi*Gi);
            }
            else
            if ( (iUnit == 1) && (iFormat == 2) ) // S, RI
            {
                    r = (1-param1*param1-param2*param2)/((1-param1)*(1-param1)+param2*param2);
                    x = (2*param2)/((1-param1)*(1-param1)+param2*param2);
            }
            else
            if ( (iUnit == 2) && (iFormat == 2) ) // Z, RI
            {
                    r = param1;
                    x = param2;
            }
            else
            {
                // Bug
            }

            if ( qIsNaN(r) || (r<0) )
            {
                r = 0;
            }
            if ( qIsNaN(x) )
            {
                x = 0;
            }

            rawData data;
            data.fq = f*fqmul;
            data.r =r*(Z0);
            data.x =x*(Z0);
            on_newData(data);
            iPoints++;
            fqMin = qMin(fqMin, data.fq);
            fqMax = qMax(fqMax, data.fq);
        }while (!line.isNull());
        emit import_finished(fqMin*1000, fqMax*1000);

        if (bGood && (iPoints>1) )
        {            
            return;
        }
        else
        {
            return;
        }
    }
    else if(_name.indexOf(".csv") >= 0 )
    {        
        //{ TODO USER_DATE debug
//        if (g_developerMode) {
//            importData(_name, true);
//            return;
//        }
        //}

        QStringList list;
        list = _name.split("/");
        if(list.length() == 1)
        {
            list.clear();
            list = _name.split("\\");
        }
        on_newMeasurement(list.last());

        QFile file(_name);
        bool result = file.open(QFile::ReadOnly);
        if(result)
        {
            QString str = file.readAll();
            double fqMin = DBL_MAX;
            double fqMax = 0;
            QStringList nList = str.split('\n');

            double mul=1.0;
            QString strFQ = nList.at(0);
            if (strFQ.contains("kHz", Qt::CaseInsensitive))
                mul = 0.001;
            else if (strFQ.contains("MHz", Qt::CaseInsensitive))
                mul = 1;
            else if (strFQ.contains("GHz", Qt::CaseInsensitive))
                mul = 1000;
            else if (strFQ.contains("Hz", Qt::CaseInsensitive))
                mul = 0.000001;
            else {
                QStringList dList = strFQ.split(',');
                if(dList.length() == 3)
                {
                    rawData data;
                    data.fq = dList.at(0).toDouble()*mul;
                    data.r = dList.at(1).toDouble();
                    data.x = dList.at(2).toDouble();
                    on_newData(data);
                    fqMin = qMin(fqMin, data.fq);
                    fqMax = qMax(fqMax, data.fq);
                }

            }
            for(int i = 1; i < nList.length(); ++i)
            {
                QStringList dList = nList.at(i).split(',');
                if(dList.length() == 3)
                {
                    rawData data;
                    data.fq = dList.at(0).toDouble()*mul;
                    data.r = dList.at(1).toDouble();
                    data.x = dList.at(2).toDouble();
                    on_newData(data);
                    fqMin = qMin(fqMin, data.fq);
                    fqMax = qMax(fqMax, data.fq);
                }
            }
            emit import_finished(fqMin*1000, fqMax*1000);
        }
    }else if(_name.indexOf(".nwl") >= 0 )
    {
        QStringList list;
        list = _name.split("/");
        if(list.length() == 1)
        {
            list.clear();
            list = _name.split("\\");
        }
        on_newMeasurement(list.last());

        QFile file(_name);
        bool result = file.open(QFile::ReadOnly);
        if(result)
        {
            QString str = file.readAll();

            double fqMin = DBL_MAX;
            double fqMax = 0;
            QStringList nList = str.split('\n');

            double mul=1.0;
            QString strFQ = nList.at(0);
            if (strFQ.contains("kHz", Qt::CaseInsensitive))
                mul = 0.001;
            else if (strFQ.contains("MHz", Qt::CaseInsensitive))
                mul = 1;
            else if (strFQ.contains("GHz", Qt::CaseInsensitive))
                mul = 1000;
            else if (strFQ.contains("Hz", Qt::CaseInsensitive))
                mul = 0.000001;

            for(int i = 1; i < nList.length(); ++i)
            {
                QStringList dList = nList.at(i).split(' ');
                if(dList.length() ==3)
                {
                    rawData data;
                    data.fq = dList.at(0).toDouble()*mul;
                    data.r = dList.at(1).toDouble();
                    data.x = dList.at(2).toDouble();
                    on_newData(data);
                    fqMin = qMin(fqMin, data.fq);
                    fqMax = qMax(fqMax, data.fq);
                }
            }
            emit import_finished(fqMin*1000, fqMax*1000);
        }
    } else {
        QMessageBox::information(nullptr, tr("Load data"), tr("Oops, this format is not supported!"), QMessageBox::Close);
    }
    /*
    if (_name.indexOf(".antdata") >= 0) {
        QStringList list;
        list = _name.split("/");
        if(list.length() == 1)
        {
            list.clear();
            list = _name.split("\\");
        }
        on_newMeasurement(list.last());

        QFile file(_name);
        bool result = file.open(QFile::ReadOnly);
        QString strInstrument;
        if(result)
        {
            QByteArray ar = file.readAll();
            if (ar.size() < 4)
                return; // empty file
            const char* pData = ar.constData();
            int comment = ar.indexOf(':');
            if (comment > 0)
            {
                const char* pInstrument = pData+comment+1;
                strInstrument = QString(pInstrument);
                strInstrument.remove(";");
            }
            double minFq = DBL_MAX;
            double maxFq = 0;
            qint16 points = *(qint16*)pData;
            points--;
            qint32* p = (qint32*)(pData+6);
            for (int idx=0; idx<points; idx++) {
                rawData data_;
                data_.fq = p[0] / 1000000.0;
                minFq = qMin(minFq, data_.fq);
                maxFq = qMax(maxFq, data_.fq);
                double* pd = (double*)(p+1);
                data_.r  = pd[0];
                data_.x  = pd[1];
                on_newData(data_);

                p += 14;
            }
            emit import_finished(minFq*1000, maxFq*1000);
        }
    }
    */
}

int Measurements::calcTdrDist(QVector<rawData> *data)
{
    if (data == nullptr || data->length() == 0)
        return 0;

    int asize = data->length();

    double minfq = data->at(0).fq;
    if ( minfq > 0.1 )
    {
        return 0; // Wrong fq
    }

    double maxfq = data->at(asize-1).fq;
    int iTdrFftSize = 0;

    int i;
    for (i=0; ; i++)
    {
        iTdrFftSize = (1<<i);
        if ( (iTdrFftSize/2) >= (asize-1) )
            break;

        if (i==14)
            return 0; // bug
    }

    iTdrFftSize *= 8;

    if (iTdrFftSize > TDR_MAXARRAY)
            return 0; // bug

    double tdrResolution = 1.0/(maxfq-minfq)/4*299792458*m_cableVelFactor / (iTdrFftSize/2) * (asize-1);

    double tdrRange = tdrResolution*iTdrFftSize/1000000;

    if (!m_measureSystemMetric)
        tdrRange *= FEETINMETER;

    return tdrRange;
}

int Measurements::CalcTdr(QVector <rawData> *data)
{
    if (data == nullptr || data->length() == 0)
        return 0;

    int asize = data->length();

    if (asize < (int)m_tdrDots)
    {
        return 0;
    }

    double minfq = data->at(0).fq;
    if ( minfq > 0.1 )
    {
        return 0; // Wrong fq
    }

    double maxfq = data->at(asize-1).fq;

    int m_iTdrFftSize = 0;

    int i;
    for (i=0; ; i++)
    {
        m_iTdrFftSize = (1<<i);
        if ( (m_iTdrFftSize/2) >= (asize-1) )
            break;

        if (i==14)
            return 0; // bug
    }

    m_iTdrFftSize *= 8;

    if (m_iTdrFftSize > TDR_MAXARRAY)
            return 0; // bug

    m_tdrResolution = 1.0/(maxfq-minfq)/4*299792458*m_cableVelFactor / (m_iTdrFftSize/2) * (asize-1);

    m_tdrRange = m_tdrResolution*m_iTdrFftSize/1000000;

    if (!m_measureSystemMetric)
        m_tdrRange *= FEETINMETER;

#if 0
    ///////////////////////
    const int dots = 1024;
    const int iTdrFftSize = 16384;
    const int iTdrFftSize2 = 8192;
    const double coeff = 1.0/4*299792458;


    double cableLength = 1500;
    double fqMin = minfq;
    double resolution = cableLength * 1000000.0/iTdrFftSize;
    double fqMax = fqMin +  coeff * m_cableVelFactor / iTdrFftSize2 * (dots-1) / resolution;
    fqMin = fqMax - coeff * m_cableVelFactor / iTdrFftSize2 * (dots-1) / resolution;
    ///
#endif

    float *TdrReal = new float[TDR_MAXARRAY];
    float *TdrImag = new float[TDR_MAXARRAY];

    //qDebug() << asize << " " << minfq << " " <<  maxfq << " " <<  m_iTdrFftSize << " " <<  m_tdrResolution << " " <<  m_tdrRange;

#define Rdevice 50.0

    for (i=0; i<=m_iTdrFftSize/2; i++)
    {
        double R=0;
        double X=0;
        double Gre=0;
        double Gim=0;
        double FQ=0;
        if (i < asize)
        {
            FQ = data->at(i).fq;
            R = data->at(i).r;
            X = data->at(i).x;

            Gre = (R*R-Rdevice*Rdevice+X*X)/((R+Rdevice)*(R+Rdevice)+X*X);
            Gim = (2*Rdevice*X)/((R+Rdevice)*(R+Rdevice)+X*X);

            if ( i==0)
            {
                double m_dFarEndImpedance = 50;
                Gre = (m_dFarEndImpedance-Rdevice)/(m_dFarEndImpedance+Rdevice);
                Gim = 0;
            }

#define KP (1.0/0.53836)
                double k = 0.53836-0.46146*cos(M_PI+M_PI*i/(asize-1));

                TdrReal[i] = Gre*m_iTdrFftSize/asize/2.0*k*KP;
                TdrImag[i] = Gim*m_iTdrFftSize/asize/2.0*k*KP;
        }
        else
        {
            TdrReal[i] = 0;
            TdrImag[i] = 0;
        }
    }

// Interpolate zero frequency

#define BDR 1
    for (i=0; i<BDR; i++)
    {
        double newreal = sqrt(TdrReal[BDR]*TdrReal[BDR]+TdrImag[BDR]*TdrImag[BDR]);

        if (TdrReal[BDR] < 0)
            TdrReal[i] = -newreal;
        else
            TdrReal[i] = newreal;

        TdrImag[i] = 0;

    }

// Mirror
    for (i=1; i<m_iTdrFftSize/2; i++)
    {
        TdrReal[m_iTdrFftSize-i] = TdrReal[i];
        TdrImag[m_iTdrFftSize-i] = -TdrImag[i];
    }
    TdrReal[m_iTdrFftSize/2] = 0;
    TdrImag[m_iTdrFftSize/2] = 0;

    FFT(TdrReal, TdrImag, m_iTdrFftSize, 1/*Inverse*/);	// Inverse FFT

    double ig = 0;
    for (i=0; i<m_iTdrFftSize; i++)
    {
        double Amp = TdrReal[i];
        if((Amp > 0.015) || (Amp < -0.015))
        {
            m_pdTdrImp[i] = Amp;
            ig += Amp/2/(((double)m_iTdrFftSize)/asize/2);
        }else
        {
            m_pdTdrImp[i] = 0;
        }

        m_pdTdrStep[i] = ig;
    }

    delete TdrReal;
    delete TdrImag;
    return m_iTdrFftSize;
}

void Measurements::FFT(float real[], float imag[], int length, int Inverse)
{
    double wreal, wpreal, wimag, wpimag, theta;
    double tempreal, tempimag, tempwreal, direction;

    int Addr, Position, Mask, BitRevAddr, PairAddr;
    int m, k;

    direction = -1.0;		// direction of rotating phasor for FFT

    if(Inverse)
        direction = 1.0;	// direction of rotating phasor for IFFT

    //  bit-reverse the addresses of both the real and imaginary arrays
    //  real[0..length-1] and imag[0..length-1] are the paired complex numbers

    for (Addr=0; Addr<length; Addr++)
    {
        // Derive Bit-Reversed Address
        BitRevAddr = 0;
        Position = length >> 1;
        Mask = Addr;
        while (Mask)
        {
            if(Mask & 1)
                BitRevAddr += Position;
            Mask >>= 1;
            Position >>= 1;
        }

        if (BitRevAddr > Addr)				// Swap
        {
            double s;
            s = real[BitRevAddr];			// real part
            real[BitRevAddr] = real[Addr];
            real[Addr] = s;
            s = imag[BitRevAddr];			// imaginary part
            imag[BitRevAddr] = imag[Addr];
            imag[Addr] = s;
        }
    }

    // FFT, IFFT Kernel

    for (k=1; k < length; k <<= 1)
    {
        theta = direction * M_PI / (double)k;
        wpimag = sin(theta);
        wpreal = cos(theta);
        wreal = 1.0;
        wimag = 0.0;

        for (m=0; m < k; m++)
        {
            for (Addr = m; Addr < length; Addr += (k*2))
            {
                PairAddr = Addr + k;

                tempreal = wreal * (double)real[PairAddr] - wimag * (double)imag[PairAddr];
                tempimag = wreal * (double)imag[PairAddr] + wimag * (double)real[PairAddr];


                real[PairAddr] = (double)real[Addr] - tempreal;
                imag[PairAddr] = (double)imag[Addr] - tempimag;
                real[Addr] += tempreal;
                imag[Addr] += tempimag;
            }
            tempwreal = wreal;
            wreal = wreal * wpreal - wimag * wpimag;
            wimag = wimag * wpreal + tempwreal * wpimag;
        }
    }

    if(Inverse)							// Normalize the IFFT coefficients
        for(int i=0; i<length; i++)
        {
            real[i] /= (double)length;
            imag[i] /= (double)length;
        }
}

void Measurements::on_dotsNumberChanged(int number)
{
    m_dotsNumber = number;
}

void Measurements::on_changeMeasureSystemMetric (bool state)
{
    m_measureSystemMetric = state;
    if(m_tdrWidget->graphCount()>2)
    {
        if(m_measureSystemMetric)
        {
            m_tdrWidget->graph(m_tdrWidget->graphCount()-2)->setData(&m_measurements.last().tdrImpGraph, true);
            m_tdrWidget->graph()->setData(&m_measurements.last().tdrStepGraph, true);
        }else
        {
            m_tdrWidget->graph(m_tdrWidget->graphCount()-2)->setData(&m_measurements.last().tdrImpGraphFeet, true);
            m_tdrWidget->graph()->setData(&m_measurements.last().tdrStepGraphFeet, true);
        }
    }
    if(m_measureSystemMetric)
    {
        m_tdrWidget->xAxis->setLabel(tr("Length, m"));
    }
    else
    {
        m_tdrWidget->xAxis->setLabel(tr("Length, feet"));
    }
}

void Measurements::on_redrawGraphs(bool _incrementally)
{
    if(m_calibration == NULL)
    {
        return;
    }
    if(m_measurements.length() == 0)
    {
        replot();
        return;
    }

    if(m_farEndMeasurement)
    {
        calcFarEnd(_incrementally);
    }

    if( m_currentTab == "tab_1")//SWR
    {
        redrawSWR(_incrementally);
    } else if( m_currentTab == "tab_2")//Phase
    {
        redrawPhase(_incrementally);
    } else if( m_currentTab == "tab_3")//Rs
    {
        redrawRs(_incrementally);
    } else if( m_currentTab == "tab_4")//Rp
    {
        redrawRp(_incrementally);
    } else if( m_currentTab == "tab_5")//Rl
    {
        redrawRl(_incrementally);
    } else if( m_currentTab == "tab_6")//TDR
    {
        redrawTDR();
    } else if( m_currentTab == "tab_7")//Smith
    {
        redrawSmith(_incrementally);
    } else if( m_currentTab == "tab_8")//User
    {
        redrawUser(_incrementally);
    }
}


void Measurements::restrictData(qreal _min, qreal _max, QCPData& _data)
{
    _data.value = (_data.value > _max) ? _max : ((_data.value < _min) ? _min : _data.value);
}

void Measurements::replot()
{
    if (m_measuringInProgress && !m_isContinuing) {
        on_drawPoint();
        return;
    }

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
    }else if(m_currentTab == "tab_8")
    {
        m_userWidget->replot();
    }
//    MainWindow* mainWindow = qobject_cast<MainWindow*>(parent());
//    QTabWidget* tabWidget = mainWindow->tabWidget();
//    QWidget* tab = tabWidget->currentWidget();
//    tab->repaint();
}

void Measurements::NormRXtoSmithPoint(double Rnorm, double Xnorm, double &x, double &y)
{
    double Denom = (Rnorm+1)*(Rnorm+1)+Xnorm*Xnorm;
    double RhoReal = ((Rnorm-1)*(Rnorm+1)+Xnorm*Xnorm)/Denom;
    double RhoImag = 2*Xnorm/Denom;

    x = RhoReal*6;// 6 - radius
    y = RhoImag*6;// 6 - radius
}

void Measurements::drawSmithImage (void)
{
    QPen pen;
    pen.setColor(Qt::black);
#define ROUND_DOTS_NUM 360
    QCPCurve *round1 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round7 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round2 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round3 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round4 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round5 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round6 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);

    QCPCurveDataMap map1;
    QCPCurveDataMap map2;
    QCPCurveDataMap map3;
    QCPCurveDataMap map4;
    QCPCurveDataMap map5;
    QCPCurveDataMap map6;
    QCPCurveDataMap map7;
    for(double i = 0; i < ROUND_DOTS_NUM; ++i)
    {
        map1.insert(i, QCPCurveData(i, (6 * qCos(i/57.02)), (6 * qSin(i/57.02))));
        map2.insert(i, QCPCurveData(i, (1 + 5 * qCos(i/57.02)), (5 * qSin(i/57.02))));
        map3.insert(i, QCPCurveData(i, (2 + 4 * qCos(i/57.02)), (4 * qSin(i/57.02))));
        map4.insert(i, QCPCurveData(i, (3 + 3 * qCos(i/57.02)), (3 * qSin(i/57.02))));
        map5.insert(i, QCPCurveData(i, (4 + 2 * qCos(i/57.02)), (2 * qSin(i/57.02))));
        map6.insert(i, QCPCurveData(i, (5 + 1 * qCos(i/57.02)), (1 * qSin(i/57.02))));
        map7.insert(i, QCPCurveData(i, (2 * qCos(i/57.02)), (2 * qSin(i/57.02))));
    }
    round1->setData( &map1,true);
    round1->setBrush(QBrush(QColor(0, 0, 255, 20)));
    round7->setData( &map7,true);
    round7->setBrush(QBrush(QColor(255, 255, 255, 255)));
    round2->setData( &map2,true);
    round3->setData( &map3,true);
    round4->setData( &map4,true);
    round5->setData( &map5,true);
    round6->setData( &map6,true);


    QCPCurve *round8 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round9 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurveDataMap map8;
    QCPCurveDataMap map9;
    for(double i = 0; i < 90; ++i)//1 line
    {
        map8.insert(i, QCPCurveData(i, (6 + 6 * qCos((i+179.15)/57.02)), (6 + 6 * qSin((i+179.15)/57.02))));
        map9.insert(i, QCPCurveData(i, (6 + 6 * qCos((i+179.15)/57.02)), (-1)*(6 + 6 * qSin((i+179.15)/57.02))));
    }
    round8->setData( &map8,true);
    round9->setData( &map9,true);

    QCPCurve *round10 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round11 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurveDataMap map10;
    QCPCurveDataMap map11;
    for(double i = 0; i < 53; ++i)//0.5 line
    {
        map10.insert(i, QCPCurveData(i, (6 + 12 * qCos((i+215.85)/57.02)), (12 + 12 * qSin((i+215.85)/57.02))));
        map11.insert(i, QCPCurveData(i, (6 + 12 * qCos((i+215.85)/57.02)), (-1)*(12 + 12 * qSin((i+215.85)/57.02))));
    }
    round10->setData( &map10,true);
    round11->setData( &map11,true);

    QCPCurve *round12 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round13 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurveDataMap map12;
    QCPCurveDataMap map13;
    for(double i = 0; i < 127; ++i)//2 line
    {
        map12.insert(i, QCPCurveData(i, (6 + 3 * qCos((i+142.45)/57.02)), (3 + 3 * qSin((i+142.45)/57.02))));
        map13.insert(i, QCPCurveData(i, (6 + 3 * qCos((i+142.45)/57.02)), (-1)*(3 + 3 * qSin((i+142.45)/57.02))));
    }
    round12->setData( &map12,true);
    round13->setData( &map13,true);

    QCPCurve *round14 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round15 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurveDataMap map14;
    QCPCurveDataMap map15;
    for(double i = 0; i < 151; ++i)// 5 line
    {
        map14.insert(i, QCPCurveData(i, (6 + 1.2 * qCos((i+112)/57.02)), (1.2 + 1.2 * qSin((i+112)/57.02))));//117.5
        map15.insert(i, QCPCurveData(i, (6 + 1.2 * qCos((i+112)/57.02)), (-1)*(1.2 + 1.2 * qSin((i+112)/57.02))));
    }
    round14->setData( &map14,true);
    round15->setData( &map15,true);

    QCPCurve *round16 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurve *round17 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurveDataMap map16;
    QCPCurveDataMap map17;
    for(double i = 0; i < 23; ++i)//0.2 line
    {
        map16.insert(i, QCPCurveData(i, (6 + 30 * qCos((i+246.19)/57.02)), (30 + 30 * qSin((i+246.19)/57.02))));
        map17.insert(i, QCPCurveData(i, (6 + 30 * qCos((i+246.19)/57.02)), (-1)*(30 + 30 * qSin((i+246.19)/57.02))));
    }
    round16->setData( &map16,true);
    round17->setData( &map17,true);


    // 0 line
    QCPCurve *round18 = new QCPCurve(m_smithWidget->xAxis, m_smithWidget->yAxis);
    QCPCurveDataMap map18;
    map18.insert(0, QCPCurveData(0, -6, 0));
    map18.insert(1, QCPCurveData(1, 6, 0));
    round18->setData( &map18,true);


    round1->setPen(pen);
    round2->setPen(pen);
    round3->setPen(pen);
    round4->setPen(pen);
    round5->setPen(pen);
    round6->setPen(pen);
    round7->setPen(pen);
    round8->setPen(pen);
    round9->setPen(pen);
    round10->setPen(pen);
    round11->setPen(pen);
    round12->setPen(pen);
    round13->setPen(pen);
    round14->setPen(pen);
    round15->setPen(pen);
    round16->setPen(pen);
    round17->setPen(pen);
    round18->setPen(pen);

    QFont serifFont("Times", 12, QFont::Bold);
    QCPItemText *center5 = new QCPItemText(m_smithWidget);
    QCPItemText *center2 = new QCPItemText(m_smithWidget);
    QCPItemText *center1 = new QCPItemText(m_smithWidget);
    QCPItemText *center05 = new QCPItemText(m_smithWidget);
    QCPItemText *center02 = new QCPItemText(m_smithWidget);
    QCPItemText *center0 = new QCPItemText(m_smithWidget);

    QCPItemText *up5 = new QCPItemText(m_smithWidget);
    QCPItemText *up2 = new QCPItemText(m_smithWidget);
    QCPItemText *up1 = new QCPItemText(m_smithWidget);
    QCPItemText *up05 = new QCPItemText(m_smithWidget);
    QCPItemText *up02 = new QCPItemText(m_smithWidget);

    QCPItemText *down5 = new QCPItemText(m_smithWidget);
    QCPItemText *down2 = new QCPItemText(m_smithWidget);
    QCPItemText *down1 = new QCPItemText(m_smithWidget);
    QCPItemText *down05 = new QCPItemText(m_smithWidget);
    QCPItemText *down02 = new QCPItemText(m_smithWidget);

    center5->position->setCoords(4.2, -0.3);
    center5->setText("5");
    center5->setFont(serifFont);
    center5->setColor(QColor(0, 0, 0, 150));

    center2->position->setCoords(2.2, -0.3);
    center2->setText("2");
    center2->setFont(serifFont);
    center2->setColor(QColor(0, 0, 0, 150));

    center1->position->setCoords(0.2, -0.3);
    center1->setText("1");
    center1->setFont(serifFont);
    center1->setColor(QColor(0, 0, 0, 150));

    center05->position->setCoords(-2.3, -0.3);
    center05->setText("0.5");
    center05->setFont(serifFont);
    center05->setColor(QColor(0, 0, 0, 150));

    center02->position->setCoords(-4.3, -0.3);
    center02->setText("0.2");
    center02->setFont(serifFont);
    center02->setColor(QColor(0, 0, 0, 150));

    center0->position->setCoords(-6.5, 0);
    center0->setText("0");
    center0->setFont(serifFont);
    center0->setColor(QColor(0, 0, 0, 150));

    up5->position->setCoords(6, 2.5);
    up5->setText("5");
    up5->setFont(serifFont);
    up5->setColor(QColor(0, 0, 0, 150));

    up2->position->setCoords(3.8, 5.4);
    up2->setText("2");
    up2->setFont(serifFont);
    up2->setColor(QColor(0, 0, 0, 150));

    up1->position->setCoords(0, 6.5);
    up1->setText("1");
    up1->setFont(serifFont);
    up1->setColor(QColor(0, 0, 0, 150));

    up05->position->setCoords(-4, 5.4);
    up05->setText("0.5");
    up05->setFont(serifFont);
    up05->setColor(QColor(0, 0, 0, 150));

    up02->position->setCoords(-6.5, 2.5);
    up02->setText("0.2");
    up02->setFont(serifFont);
    up02->setColor(QColor(0, 0, 0, 150));

    down5->position->setCoords(6, -2.5);
    down5->setText("-5");
    down5->setFont(serifFont);
    down5->setColor(QColor(0, 0, 0, 150));

    down2->position->setCoords(3.8, -5.4);
    down2->setText("-2");
    down2->setFont(serifFont);
    down2->setColor(QColor(0, 0, 0, 150));

    down1->position->setCoords(0, -6.5);
    down1->setText("-1");
    down1->setFont(serifFont);
    down1->setColor(QColor(0, 0, 0, 150));

    down05->position->setCoords(-4, -5.4);
    down05->setText("-0.5");
    down05->setFont(serifFont);
    down05->setColor(QColor(0, 0, 0, 150));

    down02->position->setCoords(-6.5, -2.5);
    down02->setText("-0.2");
    down02->setFont(serifFont);
    down02->setColor(QColor(0, 0, 0, 150));

}
//Cable-------------------------------------------------------------------------
void Measurements::setCableVelFactor(double value)
{
    m_cableVelFactor = value;
}
//------------------------------------------------------------------------------
void Measurements::setCableResistance(double value)
{
    m_cableResistance = value;
}
//------------------------------------------------------------------------------
void Measurements::setCableLossConductive(double value)
{
    m_cableLossConductive = value;
}
//------------------------------------------------------------------------------
void Measurements::setCableLossDielectric(double value)
{
    m_cableLossDielectric = value;
}
//------------------------------------------------------------------------------
void Measurements::setCableLossFqMHz(double value)
{
    m_cableLossFqMHz = value;
}
//------------------------------------------------------------------------------
void Measurements::setCableLossUnits(int value)
{
    m_cableLossUnits = value;
}
//------------------------------------------------------------------------------
void Measurements::setCableLossAtAnyFq(bool value)
{
    m_cableLossAtAnyFq = value;
}
//------------------------------------------------------------------------------
void Measurements::setCableLength(double value)
{
    m_cableLength = value;
}
//------------------------------------------------------------------------------
void Measurements::setCableFarEndMeasurement(int value)
{
    m_farEndMeasurement = value;
}

void Measurements::calcFarEnd(bool _incrementally)
{
    //if(m_calibration != NULL)
    {
        int count = m_measurements.length();
        int dataCount;
        QVector <rawData> data;
        int i = _incrementally ? (count-1) : 0;
        for( ; i < count; ++i)
        {
            if(m_calibration != nullptr && m_calibration->getCalibrationEnabled())
            {
                dataCount = m_measurements.at(i).dataRXCalib.length();
                data = m_measurements.at(i).dataRXCalib;
            }else
            {
                dataCount = m_measurements.at(i).dataRX.length();
                data = m_measurements.at(i).dataRX;
            }
            if (! _incrementally)
            {
                if(m_farEndMeasurement==1) // subtract cable
                {
                    m_farEndMeasurementsSub[i].dataRX.clear();
                    m_farEndMeasurementsSub[i].swrGraph.clear();
                    m_farEndMeasurementsSub[i].rlGraph.clear();
                    m_farEndMeasurementsSub[i].rsrGraph.clear();
                    m_farEndMeasurementsSub[i].rsxGraph.clear();
                    m_farEndMeasurementsSub[i].rszGraph.clear();
                    m_farEndMeasurementsSub[i].rprGraph.clear();
                    m_farEndMeasurementsSub[i].rpxGraph.clear();
                    m_farEndMeasurementsSub[i].rpzGraph.clear();
                    m_farEndMeasurementsSub[i].phaseGraph.clear();
                    m_farEndMeasurementsSub[i].rhoGraph.clear();
                    m_farEndMeasurementsSub[i].smithGraph.clear();
                }else if(m_farEndMeasurement==2) // add cable
                {
                    m_farEndMeasurementsAdd[i].dataRX.clear();
                    m_farEndMeasurementsAdd[i].swrGraph.clear();
                    m_farEndMeasurementsAdd[i].rlGraph.clear();
                    m_farEndMeasurementsAdd[i].rsrGraph.clear();
                    m_farEndMeasurementsAdd[i].rsxGraph.clear();
                    m_farEndMeasurementsAdd[i].rszGraph.clear();
                    m_farEndMeasurementsAdd[i].rprGraph.clear();
                    m_farEndMeasurementsAdd[i].rpxGraph.clear();
                    m_farEndMeasurementsAdd[i].rpzGraph.clear();
                    m_farEndMeasurementsAdd[i].phaseGraph.clear();
                    m_farEndMeasurementsAdd[i].rhoGraph.clear();
                    m_farEndMeasurementsAdd[i].smithGraph.clear();
                }
            }
            int ii = _incrementally ? (dataCount-1) : 0;
            for( ; ii < dataCount; ++ii)
            {
                calcFarEnd(data.at(ii), i);
            }
        }
    }
}

int Measurements::CalcTdr2(QVector <rawData> *data)
{
    #define   MIN_VECTOR_SIZE    500
    #define   MAX_VECTOR_SIZE    16384    						// value must be by Radix2 !!!
    #define   PI                 (double)3.14159265358979323846
    #define   LIGHT_SPEED        (double)299792458.0

    //------  ZERO_STYLEs
    #define   ARTIFICAL_12        0  // phase of zero is calculated from 1 & 2 bin
    #define   FROM_LOWEST         1  // phase of zero is calculated from lowest frequency posible, this point must be located at zero index of R,X vector

    #define  FT_DIRECT        			-1    // Direct transform.
    #define  FT_INVERSE       			 1    // Inverse transform.

    //---------STEP flags
    #define STEPUNITY   		0
    #define STEPIMPEDANCE		1
    //-------------------

    double GK_Re[MAX_VECTOR_SIZE*2];           // vector size, doubling for mirroring
    double GK_Im[MAX_VECTOR_SIZE*2];           // vector size, doubling for mirroring

    int vector_length;
    int radix2length;
    int  log2N;

    int cnt;
    double R;
    double X;
    double t_Re;
    double t_Im;
    double hamm;
    if(data->length() < MIN_VECTOR_SIZE+1)
    {
        return 0;
    }
    if(data->length() > MAX_VECTOR_SIZE)
    {
        return 0;
    }
    //-----------cache vars, try to find radix2 length
    vector_length = data->length();

    radix2length = DTF_FindRadix2Length(vector_length, &log2N);

    if(radix2length == 0)
    {
        return 0;
    }
    //--------------

    //-------generating GK, calculation, windowing, padding and mirroring
    cnt=1;  //starting from first bin, zero will be inserted later
    do{
        if(cnt < vector_length)
        {
            //----GE calc
            X = data->at(cnt).x;
            R = data->at(cnt).r;
            t_Re = (R*R - m_Z0 * m_Z0 +X*X)/((R+ m_Z0)*(R+m_Z0)+X*X);
            t_Im = (2*m_Z0*X)/((R+m_Z0)*(R+m_Z0)+X*X);
            //----window calc,hamming, one half, positive frequency part
            hamm = 0.53836 - 0.46146 * cos(PI * (1.0 + ((double)cnt / (double)vector_length)));
            // zero case is drop out and it equals 1.0 and applies to zero bin

            t_Re *= hamm;
            t_Im *= hamm;
            //----positive frequencies
            GK_Re[cnt] = t_Re;
            GK_Im[cnt] = t_Im;
            //---negative frequencies, mirror
            GK_Re[radix2length*2-cnt]=t_Re;
            GK_Im[radix2length*2-cnt]=-t_Im;
        }else
        {
            //---padding to radix2length by zeroes
            GK_Re[cnt] = 0.0;
            GK_Im[cnt] = 0.0;
            GK_Re[radix2length*2-cnt]=0.0;
            GK_Im[radix2length*2-cnt]=0.0;
        }
    }while(++cnt<radix2length);

    //-----put N/2 value
    GK_Re[radix2length] = 0.0;
    GK_Im[radix2length] = 0.0;


    //-----deliver zero bin
    double p1;
    double p2;
    double mod;

    p1 = sqrt(GK_Re[1]*GK_Re[1]+GK_Im[1]*GK_Im[1]);
    p2 = sqrt(GK_Re[2]*GK_Re[2]+GK_Im[2]*GK_Im[2]);
    mod = 2*p1-p2;    // module is linear extrapolated from p1,p2

    p1 = atan2(GK_Re[1], GK_Im[1]);
    p2 = atan2(GK_Re[2], GK_Im[2]);

    double phase = p2-p1;

    while(phase > PI) {phase=phase-2*PI;}
    while(phase <-PI) {phase=phase+2*PI;}

    p2 = phase;

    phase = p1-p2;

    while(phase > PI) {phase=phase-2*PI;}
    while(phase <-PI) {phase=phase+2*PI;}

    p2 = phase;

    if((p2 < -PI/2)||(p2 > PI/2))
    {
        GK_Re[0] = mod * (-1.0);
    }else
    {
        GK_Re[0] = mod * (1.0);
    }

    GK_Im[0]=0.0;
    //------------------------------------------

    FFT2(GK_Im, GK_Re, radix2length*2, (log2N+1), FT_DIRECT);

    for(int i = 0; i < radix2length*2; ++i)
    {
        m_pdTdrImp[i] = GK_Re[i];
    }

    //dtf_do_step
    qint16 end,cnt1;
    double acc;
    //-------
    acc = 0.0;
    end = radix2length*2;
    cnt1 = 0;
    do{
        acc = acc + GK_Re[cnt1];
        m_pdTdrStep[cnt1] = acc;
    }while(++cnt1<end);

    //DTF_CalcResolution
    double tmp;
//    vector_length = UTIL_limit_u16(MIN_VECTOR_SIZE, MAX_VECTOR_SIZE, vector_length);
    tmp = (data->last().fq/1000 - data->first().fq/1000)/(vector_length-1);   // resolution by original set
    tmp = tmp * radix2length;                // new Fmax by radix2length
    tmp = (LIGHT_SPEED * m_cableVelFactor)/(( tmp - data->first().fq/1000)*2*1000000.0);

    m_tdrResolution = tmp*1000;

    m_tdrRange = m_tdrResolution * radix2length/1000000;


    return end;
}

qint16 Measurements::DTF_FindRadix2Length(qint16 length, int *log2N)
{
    qint8 log;
    qint16 result =0x0010;   //start from 16
    log = 4;
    while(result <= MAX_VECTOR_SIZE)
    {
        if (result >= length)
        {
            (*log2N)=log;
            return result;
        }
        result<<=1;
        log++;
    }
    return 0;
}

void  Measurements::FFT2(double *Rdat, double *Idat, int N, int LogN, int Ft_Flag)
{
  register int  i, j, n, k, io, ie, in, nn;
  double        ru, iu, rtp, itp, rtq, itq, rw, iw, sr;

  static const double Rcoef[14] =
  {  -1.0000000000000000,  0.0000000000000000,  0.7071067811865475,
      0.9238795325112867,  0.9807852804032304,  0.9951847266721969,
      0.9987954562051724,  0.9996988186962042,  0.9999247018391445,
      0.9999811752826011,  0.9999952938095761,  0.9999988234517018,
      0.9999997058628822,  0.9999999264657178
  };
  static const double Icoef[14] =
  {   0.0000000000000000, -1.0000000000000000, -0.7071067811865474,
     -0.3826834323650897, -0.1950903220161282, -0.0980171403295606,
     -0.0490676743274180, -0.0245412285229122, -0.0122715382857199,
     -0.0061358846491544, -0.0030679567629659, -0.0015339801862847,
     -0.0007669903187427, -0.0003834951875714
  };

  nn = N >> 1;
  ie = N;
  for(n=1; n<=LogN; n++)
  {
    rw = Rcoef[LogN - n];
    iw = Icoef[LogN - n];
    in = ie >> 1;
    ru = 1.0;
    iu = 0.0;
    for(j=0; j<in; j++)
    {
      for(i=j; i<N; i+=ie)
      {
        io       = i + in;
        rtp      = Rdat[i]  + Rdat[io];
        itp      = Idat[i]  + Idat[io];
        rtq      = Rdat[i]  - Rdat[io];
        itq      = Idat[i]  - Idat[io];
        Rdat[io] = rtq * ru - itq * iu;
        Idat[io] = itq * ru + rtq * iu;
        Rdat[i]  = rtp;
        Idat[i]  = itp;
      }

      if(Ft_Flag == FT_INVERSE) iw = -iw;
      sr = ru;
      ru = ru * rw - iu * iw;
      iu = iu * rw + sr * iw;
    }
    ie >>= 1;
  }

  for(j=i=1; i<N; i++)
  {
    if(i < j)
    {
      io       = i - 1;
      in       = j - 1;
      rtp      = Rdat[in];
      itp      = Idat[in];
      Rdat[in] = Rdat[io];
      Idat[in] = Idat[io];
      Rdat[io] = rtp;
      Idat[io] = itp;
    }

    k = nn;

    while(k < j)
    {
      j   = j - k;
      k >>= 1;
    }

    j = j + k;
  }

  if(Ft_Flag == FT_INVERSE) return;

  rw = 1.0 / N;

  for(i=0; i<N; i++)
  {
    Rdat[i] *= rw;
    Idat[i] *= rw;
  }

  return;
}


void Measurements::on_translate()
{
    if (m_graphHint != nullptr)
    {
        m_graphHint->setPopupText(tr("Frequency = \n"
                                     "SWR = \n"
                                     "RL = \n"
                                     "Z = \n"
                                     "|Z| = \n"
                                     "|rho| = \n"
                                     "C = \n"
                                     "Zpar = \n"
                                     "Cpar = \n"
                                     "Cable: "));
    }
    if (m_graphBriefHint != nullptr)
    {
        m_graphBriefHint->setName(tr("BriefHint"));
    }

    if (m_tdrWidget->xAxis != nullptr)
    {
        m_tdrWidget->xAxis->setLabel(m_measureSystemMetric ? tr("Length, m") : tr("Length, feet"));
    }
}


int Measurements::getBaseUserGraphIndex(int row)
{
    int idx = 1;
    for (int i=row-1; i>=0; i--) {
        idx += m_measurements[i].userGraphs.size();
    }
    return idx;
}

#ifdef OLD_TDR
void Measurements::startTDRProgress(QWidget* _parent, int _dots)
{
    m_tdrDots = _dots;
    delete m_tdrProgressDlg;

    m_tdrProgressDlg = new ProgressDlg(_parent);
    m_tdrProgressDlg->setWindowModality(Qt::WindowModal);
    m_tdrProgressDlg->setValue(0);
    m_tdrProgressDlg->setProgressData(0, _dots, 1);
    m_tdrProgressDlg->updateActionInfo(tr("TDR measuring"));
    m_tdrProgressDlg->updateStatusInfo(tr("please wait ...."));
    m_tdrProgressDlg->setCancelable();
    connect(m_tdrProgressDlg, &ProgressDlg::canceled, this, &Measurements::measurementCanceled);
    m_tdrProgressDlg->show();
}
#else
void Measurements::startTDRProgress(Analyzer* _analyzer, QWidget* _parent)
{
    m_tdrDots = 1024;
    if (m_tdrProgressDlg != nullptr)
        disconnect(m_tdrProgressDlg);

    delete m_tdrProgressDlg;

    m_tdrProgressDlg = new TDRProgressDialog(_analyzer, _parent);
    m_tdrProgressDlg->setCableVelFactor(m_cableVelFactor);
    m_tdrProgressDlg->setWindowModality(Qt::WindowModal);
    m_tdrProgressDlg->setValue(0);
    m_tdrProgressDlg->updateActionInfo(tr("TDR measuring"));
    m_tdrProgressDlg->updateStatusInfo(tr("Set cable  length and press Start"));
    connect(m_tdrProgressDlg, &TDRProgressDialog::canceled, this, &Measurements::measurementCanceled);
    m_tdrProgressDlg->show();
}
#endif

void Measurements::stopTDRProgress()
{
    if (m_tdrProgressDlg != nullptr)
    {
        m_tdrProgressDlg->hide();
        delete m_tdrProgressDlg;
        m_tdrProgressDlg = nullptr;
    }
    on_redrawGraphs();
}

void Measurements::updateTDRProgress(int dots)
{
    if (m_tdrProgressDlg != nullptr) {
        //if ((dots%10) == 0)
        {
            m_tdrProgressDlg->setValue(dots);
            m_tdrProgressDlg->updateStatusInfo(QString(tr("processed %1 dots, from %2")).arg(dots).arg(m_tdrDots));
        }
    }
}

void Measurements::startAutocalibrateProgress(QWidget* _parent, int _dots)
{
    delete m_autoCalibrateProgressDlg;

    m_autoCalibrateProgressDlg = new ProgressDlg(_parent);
    m_autoCalibrateProgressDlg->setWindowModality(Qt::WindowModal);
    m_autoCalibrateProgressDlg->setValue(0);
    m_autoCalibrateProgressDlg->setProgressData(0, _dots, 1);
    m_autoCalibrateProgressDlg->updateActionInfo(tr("Auto calibration"));
    m_autoCalibrateProgressDlg->updateStatusInfo(tr("please wait ...."));
    m_autoCalibrateProgressDlg->setCancelable();
    connect(m_autoCalibrateProgressDlg, &ProgressDlg::canceled, this, &Measurements::interrupt);
    m_autoCalibrateProgressDlg->show();
    QApplication::processEvents();
}

void Measurements::stopAutocalibrateProgress()
{
    if (m_autoCalibrateProgressDlg != nullptr)
    {
        m_autoCalibrateProgressDlg->hide();
        m_autoCalibrateProgressDlg->deleteLater();
        m_autoCalibrateProgressDlg = nullptr;
    }
}

void Measurements::updateAutocalibrateProgress(int _dots, QString _msg)
{
    if (m_autoCalibrateProgressDlg == nullptr)
        return;
    m_autoCalibrateProgressDlg->setValue(_dots);
    m_autoCalibrateProgressDlg->updateStatusInfo(QString(tr("Iteration %1. %2"))
                                       .arg(_dots)
                                       .arg(_msg));
}

void Measurements::showOneFqWidget(QWidget* _parent, int _dots)
{
    m_oneFqMode = true;
    if (m_graphHint != nullptr)
        m_graphHint->focusHide();
    delete m_oneFqWidget;
    m_oneFqWidget = new OneFqWidget(_dots, _parent);

    m_oneFqWidget->saveHintFlags(QPair<bool,bool>(m_graphHintEnabled, m_graphBriefHintEnabled));
    m_graphHintEnabled = false;
    m_graphBriefHintEnabled = false;

    connect(m_oneFqWidget, &OneFqWidget::canceled, this, &Measurements::hideOneFqWidget);
    m_oneFqWidget->show();
}

void Measurements::updateOneFqWidget(GraphData& _data)
{
    if (m_oneFqWidget == nullptr)
        return;
    m_oneFqWidget->addData(_data);
}

void Measurements::hideOneFqWidget(bool)
{
    m_oneFqMode = false;
    OneFqWidget* tmp = m_oneFqWidget;
    if (tmp != nullptr) {
        m_oneFqWidget = nullptr;
        QPair<bool,bool> hints = tmp->resoreHintFlags();
        m_graphHintEnabled = hints.first;
        m_graphBriefHintEnabled = hints.second;
        disconnect(tmp);
        tmp->hide();
        showHideHints();
        emit oneFqCanceled();
        delete tmp;
    }
}

void Measurements::on_newMeasurementOneFq(QWidget* parent, qint64 fq, qint32 dots)
{
    m_interrupted = false;
    Q_UNUSED (fq)
    showOneFqWidget(parent, dots);
}


rawData Measurements::calcFarEnd(const rawData& data, int idx, bool refreshGraphs)
{
    rawData da = data;

    double Rpar;
    double Xpar;

    double fq = data.fq;
    double R = data.r;
    double X = data.x;

    QCPData qdata;
    qdata.key = fq*1000;

    double Klen = 1;
    switch (m_cableLossUnits)
    {
    case 0: Klen = 1; break;
    case 1: Klen = 1*100.0; break;
    case 2: Klen = 1/FEETINMETER; break;
    case 3: Klen = 1/FEETINMETER*100.0; break;
    }

    Complex Zload = Complex( R, X);

    double dMatchedLossDb;  // Note that K1/K2 are in dB/100 ft
    if(!m_cableLossAtAnyFq)
        dMatchedLossDb = m_cableLossConductive*Klen*sqrt(fq) + m_cableLossDielectric*Klen*fq;
    else
        dMatchedLossDb = m_cableLossConductive*Klen + m_cableLossDielectric*Klen;


#define NEPER 8.68588963806504        // = 20 / Ln(10)

    double Alpha = dMatchedLossDb / 100.0 / NEPER; // Nepers (attenuation) per foot
    double Beta = (2*M_PI * fq) / (SPEEDOFLIGHT*FEETINMETER/1000000.0 * m_cableVelFactor); // Radians (phase constant) per foot

    double Alphal = Alpha * m_cableLength;
    double Betal = Beta * m_cableLength;

    da.r = R;
    da.x = X;

    if(m_farEndMeasurement==1) // subtract cable
    {
        Alphal = -Alphal;
        Betal = -Betal;
    }
    if (m_cableLossUnits==0)
    {
        Alphal *= FEETINMETER;
        Betal *= FEETINMETER;
    }

    Complex Sinh_gl = Complex( cos(Betal) * sinh(Alphal), sin(Betal) * cosh(Alphal) );
    Complex Cosh_gl = Complex( cos(Betal) * cosh(Alphal), sin(Betal) * sinh(Alphal) );

    Complex Zo = Complex(m_cableResistance, -m_cableResistance * (Alpha / Beta));

    Complex ZIZL = Zo * ( (Zload*Cosh_gl + Zo*Sinh_gl) /  (Zo*Cosh_gl + Zload*Sinh_gl) );

    R = ZIZL.real();
    if(R<0.0001)
        R = 0.0001;
    X = ZIZL.imag();

    Rpar = R*(1+X*X/R/R);
    Xpar = X*(1+R*R/X/X);

    if (qIsNaN(R) || (R<0.001) ) {R = 0.01;}
    if (qIsNaN(X)) {X = 0;}

    double Rnorm = R/m_Z0;
    double Xnorm = X/m_Z0;

    double Denom = (Rnorm+1)*(Rnorm+1)+Xnorm*Xnorm;
    double RhoReal = ((Rnorm-1)*(Rnorm+1)+Xnorm*Xnorm)/Denom;
    double RhoImag = 2*Xnorm/Denom;

    double RhoPhase = atan2(RhoImag, RhoReal) / M_PI * 180.0;
    double RhoMod = sqrt(RhoReal*RhoReal+RhoImag*RhoImag);

    double swr=1;
    double rl=0;
    computeSWR(fq, getZ0(), R, X, &swr, &rl);

    da.r = R;
    da.x = X;
    QList <measurement>& _farEndMeasurements = (m_farEndMeasurement==1)
            ? m_farEndMeasurementsSub
            : m_farEndMeasurementsAdd;

    _farEndMeasurements[idx].dataRX.append(da);

    if (refreshGraphs) {
        qdata.value = swr;
        _farEndMeasurements[idx].swrGraph.insert(qdata.key,qdata);

        qdata.value = rl;
        _farEndMeasurements[idx].rlGraph.insert(qdata.key,qdata);

        qdata.value = R;
        _farEndMeasurements[idx].rsrGraph.insert(qdata.key,qdata);
        qdata.value = X;
        _farEndMeasurements[idx].rsxGraph.insert(qdata.key,qdata);
        qdata.value = computeZ(R, X);
        _farEndMeasurements[idx].rszGraph.insert(qdata.key,qdata);

        qdata.value = Rpar;
        _farEndMeasurements[idx].rprGraph.insert(qdata.key,qdata);
        qdata.value = Xpar;
        _farEndMeasurements[idx].rpxGraph.insert(qdata.key,qdata);
        qdata.value = computeZ(R, X);
        _farEndMeasurements[idx].rpzGraph.insert(qdata.key,qdata);

        qdata.value = RhoPhase;
        _farEndMeasurements[idx].phaseGraph.insert(qdata.key,qdata);
        qdata.value = RhoMod;
        _farEndMeasurements[idx].rhoGraph.insert(qdata.key,qdata);

        double pointX,pointY;
        NormRXtoSmithPoint(R/m_Z0, X/m_Z0, pointX, pointY);
        int len = _farEndMeasurements[idx].dataRX.length();
        _farEndMeasurements[idx].smithGraph.insert(len, QCPCurveData(len, pointX, pointY));
    }
    return da;
}

/*
rawData Measurements::calcFarEnd(const rawData& data, int idx, bool refreshGraphs)
{
    rawData da = data;

    double Rpar;
    double Xpar;

    double fq = data.fq;
    double R = data.r;
    double X = data.x;

    QCPData qdata;
    qdata.key = fq*1000;

    double Klen = 1;
    switch (m_cableLossUnits)
    {
    case 0: Klen = 1; break;
    case 1: Klen = 1*100.0; break;
    case 2: Klen = 1/FEETINMETER; break;
    case 3: Klen = 1/FEETINMETER*100.0; break;
    }

    Complex Zload = Complex( R, X);

    double dMatchedLossDb;  // Note that K1/K2 are in dB/100 ft
    if(!m_cableLossAtAnyFq)
        dMatchedLossDb = m_cableLossConductive*Klen*sqrt(fq) + m_cableLossDielectric*Klen*fq;
    else
        dMatchedLossDb = m_cableLossConductive*Klen + m_cableLossDielectric*Klen;


#define NEPER 8.68588963806504        // = 20 / Ln(10)

    double Alpha = dMatchedLossDb / 100.0 / NEPER; // Nepers (attenuation) per foot
    double Beta = (2*M_PI * fq) / (SPEEDOFLIGHT*FEETINMETER/1000000.0 * m_cableVelFactor); // Radians (phase constant) per foot

    double Alphal = Alpha * m_cableLength;
    double Betal = Beta * m_cableLength;

    da.r = R;
    da.x = X;

    if(m_farEndMeasurement==1) // subtract cable
    {
        Alphal = -Alphal;
        Betal = -Betal;

        if (m_cableLossUnits==0)
        {
            Alphal *= FEETINMETER;
            Betal *= FEETINMETER;
        }

        Complex Sinh_gl = Complex( cos(Betal) * sinh(Alphal), sin(Betal) * cosh(Alphal) );
        Complex Cosh_gl = Complex( cos(Betal) * cosh(Alphal), sin(Betal) * sinh(Alphal) );

        Complex Zo = Complex(m_cableResistance, -m_cableResistance * (Alpha / Beta));

        Complex ZIZL = Zo * ( (Zload*Cosh_gl + Zo*Sinh_gl) /  (Zo*Cosh_gl + Zload*Sinh_gl) );

        R = ZIZL.real();
        if(R<0.0001)
            R = 0.0001;
        X = ZIZL.imag();

        Rpar = R*(1+X*X/R/R);
        Xpar = X*(1+R*R/X/X);

        if (qIsNaN(R) || (R<0.001) ) {R = 0.01;}
        if (qIsNaN(X)) {X = 0;}

        double Rnorm = R/m_Z0;
        double Xnorm = X/m_Z0;

        double Denom = (Rnorm+1)*(Rnorm+1)+Xnorm*Xnorm;
        double RhoReal = ((Rnorm-1)*(Rnorm+1)+Xnorm*Xnorm)/Denom;
        double RhoImag = 2*Xnorm/Denom;

        double RhoPhase = atan2(RhoImag, RhoReal) / M_PI * 180.0;
        double RhoMod = sqrt(RhoReal*RhoReal+RhoImag*RhoImag);

        double swr=1;
        double rl=0;
        computeSWR(fq, getZ0(), R, X, &swr, &rl);

        da.r = R;
        da.x = X;
        m_farEndMeasurementsSub[idx].dataRX.append(da);

        if (refreshGraphs) {
            qdata.value = swr;
            m_farEndMeasurementsSub[idx].swrGraph.insert(qdata.key,qdata);

            qdata.value = rl;
            m_farEndMeasurementsSub[idx].rlGraph.insert(qdata.key,qdata);

            qdata.value = R;
            m_farEndMeasurementsSub[idx].rsrGraph.insert(qdata.key,qdata);
            qdata.value = X;
            m_farEndMeasurementsSub[idx].rsxGraph.insert(qdata.key,qdata);
            qdata.value = computeZ(R, X);
            m_farEndMeasurementsSub[idx].rszGraph.insert(qdata.key,qdata);

            qdata.value = Rpar;
            m_farEndMeasurementsSub[idx].rprGraph.insert(qdata.key,qdata);
            qdata.value = Xpar;
            m_farEndMeasurementsSub[idx].rpxGraph.insert(qdata.key,qdata);
            qdata.value = computeZ(R, X);
            m_farEndMeasurementsSub[idx].rpzGraph.insert(qdata.key,qdata);

            qdata.value = RhoPhase;
            m_farEndMeasurementsSub[idx].phaseGraph.insert(qdata.key,qdata);
            qdata.value = RhoMod;
            m_farEndMeasurementsSub[idx].rhoGraph.insert(qdata.key,qdata);

            double pointX,pointY;
            NormRXtoSmithPoint(R/m_Z0, X/m_Z0, pointX, pointY);
            int len = m_farEndMeasurementsSub[idx].dataRX.length();
            m_farEndMeasurementsSub[idx].smithGraph.insert(len, QCPCurveData(len, pointX, pointY));
        }
    }else if (m_farEndMeasurement==2) // add cable
    {
        if (m_cableLossUnits==0)
        {
            Alphal *= FEETINMETER;
            Betal *= FEETINMETER;
        }

        Complex Sinh_gl = Complex( cos(Betal) * sinh(Alphal), sin(Betal) * cosh(Alphal) );
        Complex Cosh_gl = Complex( cos(Betal) * cosh(Alphal), sin(Betal) * sinh(Alphal) );

        Complex Zo = Complex(m_cableResistance, -m_cableResistance * (Alpha / Beta));

        Complex ZIZL = Zo * ( (Zload*Cosh_gl + Zo*Sinh_gl) /  (Zo*Cosh_gl + Zload*Sinh_gl) );

        R = ZIZL.real();
        if(R<0.0001)
            R = 0.0001;
        X = ZIZL.imag();

        Rpar = R*(1+X*X/R/R);
        Xpar = X*(1+R*R/X/X);

        if (qIsNaN(R) || (R<0.001) ) {R = 0.01;}
        if (qIsNaN(X)) {X = 0;}

        double Rnorm = R/m_Z0;
        double Xnorm = X/m_Z0;

        double Denom = (Rnorm+1)*(Rnorm+1)+Xnorm*Xnorm;
        double RhoReal = ((Rnorm-1)*(Rnorm+1)+Xnorm*Xnorm)/Denom;
        double RhoImag = 2*Xnorm/Denom;

        double RhoPhase = atan2(RhoImag, RhoReal) / M_PI * 180.0;
        double RhoMod = sqrt(RhoReal*RhoReal+RhoImag*RhoImag);

        double swr=1;
        double rl=0;
        computeSWR(fq, getZ0(), R, X, &swr, &rl);

        da.r = R;
        da.x = X;
        m_farEndMeasurementsAdd[idx].dataRX.append(da);

        if (refreshGraphs) {
            qdata.value = swr;
            m_farEndMeasurementsSub[idx].swrGraph.insert(qdata.key,qdata);

            qdata.value = rl;
            m_farEndMeasurementsSub[idx].rlGraph.insert(qdata.key,qdata);

            qdata.value = R;
            m_farEndMeasurementsAdd[idx].rsrGraph.insert(qdata.key,qdata);
            qdata.value = X;
            m_farEndMeasurementsAdd[idx].rsxGraph.insert(qdata.key,qdata);
            qdata.value = computeZ(R, X);
            m_farEndMeasurementsAdd[idx].rszGraph.insert(qdata.key,qdata);

            qdata.value = Rpar;
            m_farEndMeasurementsAdd[idx].rprGraph.insert(qdata.key,qdata);
            qdata.value = Xpar;
            m_farEndMeasurementsAdd[idx].rpxGraph.insert(qdata.key,qdata);
            qdata.value = computeZ(R, X);
            m_farEndMeasurementsAdd[idx].rpzGraph.insert(qdata.key,qdata);

            qdata.value = RhoPhase;
            m_farEndMeasurementsAdd[idx].phaseGraph.insert(qdata.key,qdata);
            qdata.value = RhoMod;
            m_farEndMeasurementsAdd[idx].rhoGraph.insert(qdata.key,qdata);

            double pointX,pointY;
            NormRXtoSmithPoint(R/m_Z0, X/m_Z0, pointX, pointY);
            int len = m_farEndMeasurementsAdd[idx].dataRX.length();
            m_farEndMeasurementsAdd[idx].smithGraph.insert(len, QCPCurveData(len, pointX, pointY));
        }
    }
    return da;
}
*/
QPair<double, double> Measurements::autoCalibrate()
{
    m_settings->beginGroup("Auto-calibration");
    double cable_length_min = m_settings->value("cable_length_min", 0).toDouble();
    double cable_length_max = m_settings->value("cable_length_max", 0.02).toDouble();
    double cable_length_steps = m_settings->value("cable_length_steps", 100).toDouble();
    double cable_res_min = m_settings->value("cable_res_min", 20).toDouble();
    double cable_res_max = m_settings->value("cable_res_max", 40).toDouble();
    double cable_res_steps = m_settings->value("cable_res_steps", 100).toDouble();
    m_settings->endGroup();

    startAutocalibrateProgress(nullptr, cable_length_steps * cable_res_steps);
    // backup settings
    double _cableResistance = m_cableResistance;
    double _cableLength = m_cableLength;

    if (m_autoCalibration == 1) {
        m_cableLossConductive = 0;
        m_cableLossDielectric = 0;
        m_cableLossFqMHz = 1;
        m_farEndMeasurement = 1;
        m_cableLossAtAnyFq = 1;
        m_cableLossUnits = 0;
        m_cableVelFactor = 1;


        double dBestLength = cable_length_min;
        double dBestResistance = cable_res_max;
        double dBestDistance = 100000000000.0;

        double dBestMaxSwrValue = 0;
        double dBestMaxSwrFq = 0;

        double dSqrDist = 0;
        double Rswr = getZ0();

        //bKeypressDetected = FALSE;

        int total = 0;
        double leStep = (cable_length_max-cable_length_min)/cable_length_steps;
        double reStep = (cable_res_max-cable_res_min)/cable_res_steps;
        for (double dLen = cable_length_min; dLen <cable_length_max; dLen += leStep)
        {
            total++;
            m_cableLength = dLen;

            for (double dRes = cable_res_min; dRes <cable_res_max; dRes += reStep)
            {
                total++;
                m_cableResistance = dRes;

                dSqrDist = 0;
                double dMaxSwrValue = 0;
                double dMaxSwrFq = 0;

                int count = getMeasurementLength();
                if (count == 0) {
                    return QPair<double, double>(_cableResistance, _cableLength);
                }
                measurement* mm = getMeasurement(count-1);
                m_farEndMeasurementsSub[count-1].dataRX.clear();
                const QVector<rawData>& data = mm->dataRX;

//                double dSqrDist0 = 0;
                for (int i=0; i<data.size(); i++) {
                    const rawData& inData = data.at(i);
                    if (inData.fq*1000 > 10000)
                    {
                        rawData outData = calcFarEnd(inData, count-1, true);
                        double R = outData.r;
                        double X = outData.x;

                        double Gre = (R*R-Rswr*Rswr+X*X)/((R+Rswr)*(R+Rswr)+X*X);
                        double Gim = (2*Rswr*X)/((R+Rswr)*(R+Rswr)+X*X);

                        dSqrDist += Gre*Gre+Gim*Gim;

                        double tmpSWR;
                        int ret = computeSWR(inData.fq*1000.0, getZ0(), R, X, &tmpSWR, nullptr);
                        if (ret != 0) {

//                            if (i == 0) {
//                                dSqrDist0 = tmpSWR*tmpSWR;
//                                dSqrDist = dSqrDist0;
//                            }
//                            else
//                            {
//                                dSqrDist += (tmpSWR - dSqrDist0)*(tmpSWR - dSqrDist0);
//                            }

                            double tmpFQ = inData.fq;
                            if (tmpSWR > dMaxSwrValue)
                            {
                                dMaxSwrValue = tmpSWR;
                                dMaxSwrFq = tmpFQ;
                            }
                        }
                    }
                }

                bool update = false;
                if (dSqrDist<dBestDistance)
                {
                    update = true;
                    dBestDistance = dSqrDist;
                    dBestLength = dLen;
                    dBestResistance = dRes;

                    on_redrawGraphs();
                    QApplication::processEvents();

                    dBestMaxSwrValue = dMaxSwrValue;
                    dBestMaxSwrFq = dMaxSwrFq;

//                    qDebug() << "m_cableResistance: " <<   m_cableResistance
//                             << ", m_cableLength: " << m_cableLength
//                             << ", dBestMaxSwrValue: " << dBestMaxSwrValue
//                             << ", dBestMaxSwrFq: " << dBestMaxSwrFq;
                    QString msg = QString("Connector compensation: SWR=%1 at Fq=%2 MHz: Rcable=%3, Lcable=%4")
                            .arg(dBestMaxSwrValue)
                            .arg(dBestMaxSwrFq)
                            .arg(m_cableResistance)
                            .arg(m_cableLength);
                    updateAutocalibrateProgress(total, msg);
                }
                if (update || ((total%100)==0)) {
                    QString msg = QString("Connector compensation: SWR=%1 at Fq=%2 MHz: Rcable=%3, Lcable=%4")
                            .arg(dBestMaxSwrValue)
                            .arg(dBestMaxSwrFq)
                            .arg(m_cableResistance)
                            .arg(m_cableLength);
                    updateAutocalibrateProgress(total, msg);
                }
                QApplication::processEvents();
                if (m_interrupted)
                    break;
            }

            QApplication::processEvents();
            if (m_interrupted)
                break;
        }

        if (m_interrupted) {
            m_interrupted = false;
            stopAutocalibrateProgress();
            return QPair<double, double>(_cableResistance, _cableLength);;
        }

        m_cableLength = dBestLength;
        m_cableResistance = dBestResistance;

        qDebug() << QString("Connector compensation: SWR=%1 at Fq=%2 MHz: Rcable=%3, Lcable=%4")
                    .arg(dBestMaxSwrValue).arg(dBestMaxSwrFq).arg(m_cableResistance).arg(m_cableLength);

        stopAutocalibrateProgress();
        on_redrawGraphs();
    }

    m_interrupted = false;
    QPair<double, double> result(m_cableResistance, m_cableLength);

    // restore settings
    //m_cableResistance = _cableResistance;
    //m_cableLength = _cableLength;

    setAutoCalibration(0);
    return result;
}

void Measurements::on_isRangeChanged(bool _range)
{
    m_RangeMode = _range;

    int len = getMeasurementLength();
    for (int i=0; i<m_tableWidget->rowCount(); i++)
    {
        measurement* mm = getMeasurement(len-i-1);
        qint64 sw = mm->qint64Sw;
        qint64 fq = mm->qint64Fq;
        double range = sw;
        double center = fq;
        QString fmt;
        if (m_RangeMode)
        {
            fmt = tr("FQ:%1kHz SW:%2kHz Points:%3");
            range = (sw - fq)/2.0;
            center = fq + range;
        } else {
            fmt = tr("Start:%1kHz Stop:%2kHz Points:%3");
        }
        QString tips = QString(fmt)
                .arg((long)(center/1000))
                .arg((long)(range/1000))
                .arg(mm->qint64Dots);
        QTableWidgetItem *item = m_tableWidget->item(i,0);
        item->setToolTip(tips);
    }
}

void Measurements::setZ0(double _Z0)
{
    m_Z0 = _Z0;
}

void Measurements::on_impedanceChanged(double _z0)
{
    m_Z0 = _z0;
    qint32 len = getMeasurementLength();

    m_currentIndex -= len;
    if (m_currentIndex < 1)
        m_currentIndex = 0;

    QList<QTableWidgetItem*> selected = m_tableWidget->selectedItems();
    for (int idx=0; idx<len; idx++) {
        QString name = m_tableNames.takeFirst();
        measurement mm = m_measurements.takeFirst();

        delete mm.smithCurve;
        delete m_viewMeasurements.takeFirst().smithCurve;
        delete m_farEndMeasurementsAdd.takeFirst().smithCurve;
        delete m_farEndMeasurementsSub.takeFirst().smithCurve;
        m_swrWidget->removeGraph(1);
        m_phaseWidget->removeGraph(1);
        m_rsWidget->removeGraph(1);
        m_rsWidget->removeGraph(1);
        m_rsWidget->removeGraph(1);
        m_rpWidget->removeGraph(1);
        m_rpWidget->removeGraph(1);
        m_rpWidget->removeGraph(1);
        m_rlWidget->removeGraph(1);
        m_tdrWidget->removeGraph(1);
        m_tdrWidget->removeGraph(1);

        on_newMeasurement(name, mm.qint64Fq, mm.qint64Sw, mm.qint64Dots);
        for (int i=0; i<mm.dataRX.size(); i++) {
            on_newData(mm.dataRX.at(i));
        }

        // restore user data
        if (g_developerMode) {
            int count = mm.userGraphs.size();
            for (int i=0; i<count; i++)
                m_userWidget->removeGraph(1);
            on_newUserDataHeader(mm.fieldsUser);
            for (int iu=0; iu<mm.dataUser.size(); iu++) {
                UserData _userData = mm.dataUser.at(iu);
                m_measurements.last().dataUser.append(_userData);
                for (int idx=0; idx<_userData.values.size(); idx++) {
                    QCPData qcpData;
                    qcpData.key = _userData.fq*1000;
                    qcpData.value = _userData.values.at(idx);
                    QCPDataMap* map = m_measurements.last().userGraphs.at(idx);
                    map->insert(qcpData.key,qcpData);
                    QCPDataMap* vmap = m_viewMeasurements.last().userGraphs.at(idx);
                    vmap->insert(qcpData.key,qcpData);
                }
                QVector <double> x,y;
                x.append(_userData.fq*1000);
                x.append(_userData.fq*1000);
                y.append(m_userWidget->yAxis->getRangeLower());
                y.append(m_userWidget->yAxis->getRangeUpper());
                m_userWidget->graph(0)->setData(x,y);
            }
        }
    }
    m_measuringInProgress = false;
    if (!selected.isEmpty()) {
        m_tableWidget->selectRow(selected.at(0)->row());
        emit selectMeasurement(selected.at(0)->row(), selected.at(0)->column());
    }
}

void Measurements::redrawTDR(int _index)
{
    int mode = m_farEndMeasurement;
    int begin = _index < 0 ? 0 : _index;
    int end = _index < 0 ? m_measurements.length() : (_index+1);
    for (int index=begin; index<end; index++) {
        measurement& mm = (mode == 1)
                ? m_farEndMeasurementsSub[index]
                : ( (mode == 2) ? m_farEndMeasurementsAdd[index] : m_measurements[index] );

        int len = CalcTdr(m_calibration->getCalibrationEnabled()
                      ? &mm.dataRXCalib
                      : &mm.dataRX);
        m_tdrWidget->xAxis->setRangeUpper(m_tdrRange);
        m_tdrWidget->xAxis->setRangeMax(m_tdrRange);
        double step = m_tdrRange/len;
        mm.tdrImpGraph.clear();
        mm.tdrStepGraph.clear();
        mm.tdrImpGraphFeet.clear();
        mm.tdrStepGraphFeet.clear();
        for(int i = 0; i < len; ++i)
        {
            double x = i;
            double y = m_pdTdrImp[i];
            QCPData data;
            data.key = x*step;
            data.value = y;
            mm.tdrImpGraph.insert(data.key,data);
            data.value = m_pdTdrStep[i];
            mm.tdrStepGraph.insert(data.key,data);

            QCPData dataFeet;
            dataFeet.key = x*step;
            dataFeet.value = y;
            mm.tdrImpGraphFeet.insert(dataFeet.key,dataFeet);
            dataFeet.value = m_pdTdrStep[i];
            mm.tdrStepGraphFeet.insert(dataFeet.key,dataFeet);
        }
        m_tdrWidget->graph(index*2+1)->setData(
                        m_measureSystemMetric ? &mm.tdrImpGraph : &mm.tdrImpGraphFeet, true);
        m_tdrWidget->graph(index*2+2)->setData(
                        m_measureSystemMetric ? &mm.tdrStepGraph : &mm.tdrStepGraphFeet, true);
    } // for index
    replot();
}

void Measurements::redrawSWR(bool _incrementally)
{
    if (m_measurements.isEmpty())
        return;
    int i = _incrementally ? (m_measurements.length()-1) : 0;
    if (m_calibration->getCalibrationEnabled())
    {
        if(m_farEndMeasurement != 0)
        {
            for(; i < m_measurements.length(); ++i)
            {
                m_swrWidget->graph(i+1)->setData(m_farEndMeasurement == 1
                                                 ? &m_farEndMeasurementsSub[i].swrGraph
                                                 : &m_farEndMeasurementsAdd[i].swrGraph,
                                                 true);
            }
        }else
        {
            for(; i < m_measurements.length(); ++i)
            {
                m_swrWidget->graph(i+1)->setData(&m_measurements[i].swrGraphCalib, true);
            }
        }
    } else {
        for(; i < m_measurements.length(); ++i)
        {
            if (!_incrementally)
                m_viewMeasurements[i].swrGraph.clear();

            QCPDataMap *map;
            switch (m_farEndMeasurement) {
            case 1:
                map = &m_farEndMeasurementsSub[i].swrGraph;
                break;
            case 2:
                map = &m_farEndMeasurementsAdd[i].swrGraph;
                break;
            default:
                map = &m_measurements[i].swrGraph;
                break;
            }
            QList <double> list = map->keys();
            if (list.isEmpty())
                continue;
            QCPData data;
            QCPData viewData;
            double maxSwr = MAX_SWR;//m_swrWidget->yAxis->range().upper;
            int n = _incrementally ? (list.length()-1) : 0;
            for(; n < list.length(); ++n)
            {
                data.key = list.at(n);
                viewData = map->value(list.at(n));
                if( viewData.value > maxSwr || viewData.value < 1)
                {
                    data.value = maxSwr;
                }else
                {
                    data.value = viewData.value;
                }
                m_viewMeasurements[i].swrGraph.insert(data.key,data);
            }
            if (!_incrementally) {
                m_swrWidget->graph(i+1)->setData(&m_viewMeasurements[i].swrGraph, true);
            } else {
                m_swrWidget->graph(i+1)->addData(m_viewMeasurements[i].swrGraph.last());
            }
        }
    }
    replot();
}



void Measurements::redrawPhase(bool _incrementally)
{
    if (m_measurements.isEmpty())
        return;
    bool calibr = m_calibration->getCalibrationEnabled();
    int i = _incrementally ? (m_measurements.length()-1) : 0;

    if(m_farEndMeasurement != 0)
    {
        for(; i < m_measurements.length(); ++i)
        {
            m_phaseWidget->graph(i+1)->setData(m_farEndMeasurement == 1
                                               ? &m_farEndMeasurementsSub[i].phaseGraph
                                               : &m_farEndMeasurementsAdd[i].phaseGraph,
                                               true);
        }
    }else
    {
        for(; i < m_measurements.length(); ++i)
        {
            m_phaseWidget->graph(i+1)->setData(calibr
                                               ? &m_measurements[i].phaseGraphCalib
                                               : &m_measurements[i].phaseGraph, true);
        }
    }
    replot();
}

void Measurements::redrawRs(bool _incrementally)
{
    if (m_measurements.isEmpty())
        return;
    bool calibr = m_calibration->getCalibrationEnabled();
    int i = _incrementally ? (m_measurements.length()-1) : 0;

    double maxVal = m_rsWidget->yAxis->range().upper;
    double minVal = m_rsWidget->yAxis->range().lower;

    QCPDataMap rMap;
    QCPDataMap xMap;
    QCPDataMap zMap;
    for (; i<m_measurements.length(); i++) {
        if (!_incrementally) {
            m_viewMeasurements[i].rsrGraph.clear();
            m_viewMeasurements[i].rsxGraph.clear();
            m_viewMeasurements[i].rszGraph.clear();
        }

        if(m_farEndMeasurement)
        {
            if(m_farEndMeasurement == 1)
            {
                rMap = m_farEndMeasurementsSub[i].rsrGraph;
                xMap = m_farEndMeasurementsSub[i].rsxGraph;
                zMap = m_farEndMeasurementsSub[i].rszGraph;
            }else if (m_farEndMeasurement == 2)
            {
                rMap = m_farEndMeasurementsAdd[i].rsrGraph;
                xMap = m_farEndMeasurementsAdd[i].rsxGraph;
                zMap = m_farEndMeasurementsAdd[i].rszGraph;
            }
        }else
        {
            rMap = calibr ? m_measurements[i].rsrGraphCalib : m_measurements[i].rsrGraph;
            xMap = calibr ? m_measurements[i].rsxGraphCalib : m_measurements[i].rsxGraph;
            zMap = calibr ? m_measurements[i].rszGraphCalib : m_measurements[i].rszGraph;
        }
        QList <double> rKeys = rMap.keys();
        QList <double> xKeys = xMap.keys();
        QList <double> zKeys = zMap.keys();

        if (rKeys.isEmpty())
            continue;
        QCPData data;
        int n = _incrementally ? (rKeys.length()-1) : 0;
        for( ; n < rKeys.length(); ++n)
        {
            data = rMap.value(rKeys.at(n));
            restrictData(minVal, maxVal, data);
            m_viewMeasurements[i].rsrGraph.insert(data.key, data);

            data = xMap.value(xKeys.at(n));
            restrictData(minVal, maxVal, data);
            m_viewMeasurements[i].rsxGraph.insert(data.key, data);

            data = zMap.value(zKeys.at(n));
            restrictData(minVal, maxVal, data);
            m_viewMeasurements[i].rszGraph.insert(data.key, data);
        }
        if (!_incrementally) {
            m_rsWidget->graph(i*3+3)->setData(&m_viewMeasurements[i].rszGraph, true);
            m_rsWidget->graph(i*3+2)->setData(&m_viewMeasurements[i].rsxGraph, true);
            m_rsWidget->graph(i*3+1)->setData(&m_viewMeasurements[i].rsrGraph, true);
        } else {
            m_rsWidget->graph(i*3+3)->addData(m_viewMeasurements[i].rszGraph.last());
            m_rsWidget->graph(i*3+2)->addData(m_viewMeasurements[i].rsxGraph.last());
            m_rsWidget->graph(i*3+1)->addData(m_viewMeasurements[i].rsrGraph.last());
        }
    }
    replot();
}

void Measurements::redrawRp(bool _incrementally)
{
    if (m_measurements.isEmpty())
        return;

    bool calibr = m_calibration->getCalibrationEnabled();
    int i = _incrementally ? (m_measurements.length()-1) : 0;

    double maxVal = m_rpWidget->yAxis->range().upper;
    double minVal = m_rpWidget->yAxis->range().lower;

    QCPDataMap rMap;
    QCPDataMap xMap;
    QCPDataMap zMap;
    for (; i<m_measurements.length(); i++) {
        if (!_incrementally) {
            m_viewMeasurements[i].rprGraph.clear();
            m_viewMeasurements[i].rpxGraph.clear();
            m_viewMeasurements[i].rpzGraph.clear();
        }

        if(m_farEndMeasurement)
        {
            if(m_farEndMeasurement == 1)
            {
                rMap = m_farEndMeasurementsSub[i].rprGraph;
                xMap = m_farEndMeasurementsSub[i].rpxGraph;
                zMap = m_farEndMeasurementsSub[i].rpzGraph;
            }else if (m_farEndMeasurement == 2)
            {
                rMap = m_farEndMeasurementsAdd[i].rprGraph;
                xMap = m_farEndMeasurementsAdd[i].rpxGraph;
                zMap = m_farEndMeasurementsAdd[i].rpzGraph;
            }
        }else
        {
            rMap = calibr ? m_measurements[i].rprGraphCalib : m_measurements[i].rprGraph;
            xMap = calibr ? m_measurements[i].rpxGraphCalib : m_measurements[i].rpxGraph;
            zMap = calibr ? m_measurements[i].rpzGraphCalib : m_measurements[i].rpzGraph;
        }
        QList <double> rKeys = rMap.keys();
        QList <double> xKeys = xMap.keys();
        QList <double> zKeys = zMap.keys();

        if (rKeys.isEmpty())
            continue;
        QCPData data;
        int n = _incrementally ? (rKeys.length()-1) : 0;
        for( ; n < rKeys.length(); ++n)
        {
            data = rMap.value(rKeys.at(n));
            restrictData(minVal, maxVal, data);
            m_viewMeasurements[i].rprGraph.insert(data.key, data);

            data = xMap.value(xKeys.at(n));
            restrictData(minVal, maxVal, data);
            m_viewMeasurements[i].rpxGraph.insert(data.key, data);

            data = zMap.value(zKeys.at(n));
            restrictData(minVal, maxVal, data);
            m_viewMeasurements[i].rpzGraph.insert(data.key, data);
        }
        if (!_incrementally) {
            m_rpWidget->graph(i*3+3)->setData(&m_viewMeasurements[i].rpzGraph, true);
            m_rpWidget->graph(i*3+2)->setData(&m_viewMeasurements[i].rpxGraph, true);
            m_rpWidget->graph(i*3+1)->setData(&m_viewMeasurements[i].rprGraph, true);
        } else {
            m_rpWidget->graph(i*3+3)->addData(m_viewMeasurements[i].rpzGraph.last());
            m_rpWidget->graph(i*3+2)->addData(m_viewMeasurements[i].rpxGraph.last());
            m_rpWidget->graph(i*3+1)->addData(m_viewMeasurements[i].rprGraph.last());
        }
    }
    replot();
}

void Measurements::redrawRl(bool _incrementally)
{
    if (m_measurements.isEmpty())
        return;
    bool calibr = m_calibration->getCalibrationEnabled();
    int i = _incrementally ? (m_measurements.length()-1) : 0;

    if(m_farEndMeasurement != 0)
    {
        for(; i < m_measurements.length(); ++i)
        {
            m_rlWidget->graph(i+1)->setData(m_farEndMeasurement == 1
                                               ? &m_farEndMeasurementsSub[i].rlGraph
                                               : &m_farEndMeasurementsAdd[i].rlGraph,
                                               true);
        }
    }else
    {
        for(; i < m_measurements.length(); ++i)
        {
            m_rlWidget->graph(i+1)->setData(calibr
                                            ? &m_measurements[i].rlGraphCalib
                                            : &m_measurements[i].rlGraph, true);
        }
    }
    replot();
}

void Measurements::redrawSmith(bool _incrementally)
{
    if (m_measurements.isEmpty())
        return;
    bool calibr = m_calibration->getCalibrationEnabled();
    int i = _incrementally ? (m_measurements.length()-1) : 0;

    if(m_farEndMeasurement)
    {
        if(m_farEndMeasurement == 1)
        {
            for(; i < m_measurements.length(); ++i)
            {
                m_measurements[i].smithCurve->setData(calibr
                                                      ? &m_farEndMeasurementsSub[i].smithGraphCalib
                                                      : &m_farEndMeasurementsSub[i].smithGraph,true);
            }
        }else if(m_farEndMeasurement == 2)
        {
            for(; i < m_measurements.length(); ++i)
            {
                m_measurements[i].smithCurve->setData(calibr
                                                      ? &m_farEndMeasurementsAdd[i].smithGraphCalib
                                                      : &m_farEndMeasurementsAdd[i].smithGraph,true);
            }
        }
    }else
    {
        for(; i < m_measurements.length(); ++i)
        {
            m_measurements[i].smithCurve->setData(&m_measurements[i].smithGraphViewCalib,true);
            m_measurements[i].smithCurve->setData(calibr
                                                  ? &m_measurements[i].smithGraphViewCalib
                                                  : &m_measurements[i].smithGraphView,true);
        }
    }
    replot();
}

void Measurements::redrawUser(bool _incrementally)
{
    if (m_measurements.isEmpty())
        return;
    int i = _incrementally ? (m_measurements.length()-1) : 0;

    for( ; i < m_measurements.length(); ++i)
    {
        int count = m_viewMeasurements[i].userGraphs.size();
        for (int idx=0; idx<count; idx++) {
            if (!_incrementally)
                m_viewMeasurements[i].userGraphs[idx]->clear();

            QCPData data;
            QCPData value;
            double maxVal = m_userWidget->yAxis->range().upper;
            double minVal = m_userWidget->yAxis->range().lower;

            QCPDataMap* map = m_measurements[i].userGraphs.at(idx);
            QCPDataMap* vmap = m_viewMeasurements[i].userGraphs.at(idx);

            QList <double> list = map->keys();
            if (list.isEmpty())
                continue;
            int n = _incrementally ? (list.length()-1) : 0;
            for( ; n < list.length(); ++n)
            {
                QCPData data = map->value(list.at(n));
                restrictData(minVal, maxVal, data);
                vmap->insertMulti(data.key,data);
            }
            int index = getBaseUserGraphIndex(i)+idx;
            m_userWidget->graph(index)->setData(m_viewMeasurements[i].userGraphs[idx], true);
        }
    }
}

void Measurements::on_exportCableSettings(QString _description)
{
    Export* exportDialog = new Export(m_tableWidget);
    exportDialog->setAttribute(Qt::WA_DeleteOnClose);
    exportDialog->setWindowTitle(tr("Export"));
    exportDialog->setMeasurements(this, m_measurements.size()-1, true, _description);

    exportDialog->exec();
}


void Measurements::on_drawPoint()
{
    //qint64 t0 = QDateTime::currentMSecsSinceEpoch();

    CustomPlot* plot = qobject_cast<CustomPlot*>(m_rpWidget);
    if(m_currentTab == "tab_1")
    {
        plot = qobject_cast<CustomPlot*>(m_swrWidget);
    }else if(m_currentTab == "tab_2")
    {
        plot = qobject_cast<CustomPlot*>(m_phaseWidget);
    }else if(m_currentTab == "tab_3")
    {
        plot = qobject_cast<CustomPlot*>(m_rsWidget);
    }else if(m_currentTab == "tab_4")
    {
        plot = qobject_cast<CustomPlot*>(m_rpWidget);
    }else if(m_currentTab == "tab_5")
    {
        plot = qobject_cast<CustomPlot*>(m_rlWidget);
    }else if(m_currentTab == "tab_6")
    {
        plot = qobject_cast<CustomPlot*>(m_tdrWidget);
    }else if(m_currentTab == "tab_7")
    {
        plot = qobject_cast<CustomPlot*>(m_smithWidget);
        plot->replot();
        return;
    }else if(m_currentTab == "tab_8")
    {
        plot = qobject_cast<CustomPlot*>(m_userWidget);
    }

    plot->drawIncrementally();
//    QCPPainter painter;
//    painter.begin(&plot->paintBuffer());

//    plot->setIncremental(true);
//    plot->draw(&painter);
//    plot->setIncremental(false);

//    painter.end();

//    QRect _clip = plot->getClipRect();
//    plot->update(_clip);

//    qDebug() << "Measurements::on_drawPoint()" << (QDateTime::currentMSecsSinceEpoch()-t0);
}

CustomPlot* Measurements::activePlot()
{
    QCustomPlot* plot = m_swrWidget;
    if(m_currentTab == "tab_1")
    {
        plot = m_swrWidget;
    }else if(m_currentTab == "tab_2")
    {
        plot = m_phaseWidget;
    }else if(m_currentTab == "tab_3")
    {
        plot = m_rsWidget;
    }else if(m_currentTab == "tab_4")
    {
        plot = m_rpWidget;
    }else if(m_currentTab == "tab_5")
    {
        plot = m_rlWidget;
    }else if(m_currentTab == "tab_6")
    {
        plot = m_tdrWidget;
    }else if(m_currentTab == "tab_7")
    {
        plot = m_smithWidget;
    }else if(m_currentTab == "tab_8")
    {
        plot = m_userWidget;
    }
    return qobject_cast<CustomPlot*>(plot);
}

void Measurements::on_measurementComplete()
{
    qDebug() << "Measurements::on_measurementComplete()";
    m_previousI = 0;
    m_measuringInProgress = false;
    m_isContinuing = false;
}

void Measurements::changeColorTheme(bool _dark)
{
    if (!_dark) {
        m_graphHint->setBackgroundColor(QColor(127,127,127,64 ));
        m_graphHint->setTextColor("#010101");
    } else {
        m_graphHint->setBackgroundColor(QColor(0,0,0,180 ));
        m_graphHint->setTextColor("#01b2ff");
    }
}


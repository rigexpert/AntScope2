#include "print.h"
#include "ui_print.h"

Print::Print(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Print),
    m_isSmithGraph(false)
{
    ui->setupUi(this);

    QString path = Settings::setIniFile();
    m_settings = new QSettings(path, QSettings::IniFormat);
    m_settings->beginGroup("Print");

    m_lastPath = m_settings->value("lastPath","").toString();

    QRect rect = m_settings->value("geometry", 0).toRect();
    if(rect.x() != 0)
        this->setGeometry(rect);

    m_settings->endGroup();

    QFont font = ui->widgetGraph->xAxis->tickLabelFont();
    font.setPointSize(12);
    ui->widgetGraph->xAxis->setTickLabelFont(font);
    ui->widgetGraph->yAxis->setTickLabelFont(font);

    font.setPointSize(14);
    ui->widgetGraph->xAxis->setLabelFont(font);
    ui->widgetGraph->yAxis->setLabelFont(font);

    m_markersNumberLabel.setText(tr("Marker"));
    m_measurementsNumberLabel.setText("#");
    m_fqLabel.setText(tr("Fq"));
    m_swrLabel.setText(tr("SWR"));
    m_rlLabel.setText(tr("RL"));
    m_zLabel.setText(tr("Z"));
    m_phaseLabel.setText(tr("Phase"));
    ui->markersLayout->addWidget(&m_markersNumberLabel,0,0);
    ui->markersLayout->addWidget(&m_measurementsNumberLabel,0,1);
    ui->markersLayout->addWidget(&m_fqLabel,0,2);
    ui->markersLayout->addWidget(&m_swrLabel,0,3);
    ui->markersLayout->addWidget(&m_rlLabel,0,4);
    ui->markersLayout->addWidget(&m_zLabel,0,5);
    ui->markersLayout->addWidget(&m_phaseLabel,0,6);

    ui->widgetGraph->legend->setVisible(true);
}

Print::~Print()
{
    m_settings->beginGroup("Print");
    m_settings->setValue("geometry", this->geometry());
    m_settings->setValue("lastPath",m_lastPath);
    m_settings->endGroup();

    delete ui;
}

void Print::addMarker(double fq, int number)
{
    m_mFqList.append(fq);

    QCPItemStraightLine *line = new QCPItemStraightLine(ui->widgetGraph);
    QCPItemText *text = new QCPItemText(ui->widgetGraph);
    line->setAntialiased(false);
    ui->widgetGraph->addItem(line);
    ui->widgetGraph->addItem(text);
    line->setPen(QPen(QColor(255,0,0,150)));
    text->setColor(QColor(255, 0, 0, 150));

    line->point1->setCoords(fq, -2000);
    line->point2->setCoords(fq, 2000);

    text->setText(QString::number(number));

    m_mStraightLineList.append(line);
    m_mTextList.append(text);

    rescale();
}

void Print::addRowText(const QStringList &list)
{
    m_markersNumberList.append(list.at(0));
    m_measurementsNumberList.append(list.at(0));
    m_fqList.append(list.at(1));
    m_swrList.append(list.at(2));
    m_rlList.append(list.at(3));
    m_zList.append(list.at(4));
    m_phaseList.append(list.at(5));

    QLabel* label = new QLabel();
    label->setText(list.at(0));
    ui->markersLayout->addWidget(label,m_markersNumberList.length(),0);
    m_markersNumberObjList.append(label);

    QLabel* label1 = new QLabel();
    label1->setText(list.at(1));
    ui->markersLayout->addWidget(label1,m_measurementsNumberList.length(),1);
    m_measurementsNumberObjList.append(label1);

    QLabel* label2 = new QLabel();
    label2->setText(list.at(2));
    ui->markersLayout->addWidget(label2,m_fqList.length(),2);
    m_fqObjList.append(label2);

    QLabel* label3 = new QLabel();
    label3->setText(list.at(3));
    ui->markersLayout->addWidget(label3,m_swrList.length(),3);
    m_swrObjList.append(label3);

    QLabel* label4 = new QLabel();
    label4->setText(list.at(4));
    ui->markersLayout->addWidget(label4,m_rlList.length(),4);
    m_rlObjList.append(label4);

    QLabel* label5 = new QLabel();
    label5->setText(list.at(5));
    ui->markersLayout->addWidget(label5,m_zList.length(),5);
    m_zObjList.append(label5);

    QLabel* label6 = new QLabel();
    label6->setText(list.at(6));
    ui->markersLayout->addWidget(label6,m_phaseList.length(),6);
    m_phaseObjList.append(label6);

    updateTable();
}

void Print::updateTable()
{
    adjustSize();
}

void Print::setRange(QCPRange x, QCPRange y)
{
    ui->widgetGraph->xAxis->setRangeMin(x.lower);
    ui->widgetGraph->xAxis->setRangeMax(x.upper);

    ui->widgetGraph->yAxis->setRangeMin(y.lower);
    ui->widgetGraph->yAxis->setRangeMax(y.upper);

    ui->widgetGraph->xAxis->setRange(x);
    ui->widgetGraph->yAxis->setRange(y);
}

void Print::setLabel(QString xLabel, QString yLabel)
{
    ui->widgetGraph->xAxis->setLabel(xLabel);
    ui->widgetGraph->yAxis->setLabel(yLabel);
}

void Print::setData(QCPDataMap *m, QPen pen, QString name)
{
    ui->widgetGraph->addGraph();

    ui->widgetGraph->graph()->setData(m,true);
    ui->widgetGraph->graph()->setPen(pen);
    ui->widgetGraph->graph()->setName(name);

    QPen gridPen = ui->widgetGraph->xAxis->grid()->pen();
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setColor(QColor(0, 0, 0, 255));
    ui->widgetGraph->xAxis->grid()->setPen(gridPen);
    ui->widgetGraph->yAxis->grid()->setPen(gridPen);

    m_isSmithGraph = false;
    rescale();
}

void Print::setSmithData(QCPCurveDataMap *map, QPen pen, QString name)
{
    QCPCurve *smithCurve = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    smithCurve->setData(map, true);
    smithCurve->setPen(pen);
    smithCurve->setName(name);
    m_isSmithGraph = true;
    rescale();
}

void Print::drawBands(QStringList* _bands, double y1, double y2)
{
    if (_bands == nullptr) {
        addBand(135.7, 137.8, y1, y2);
        addBand(472, 479, y1, y2);
        addBand(1800, 2000, y1, y2);
        addBand(3500, 3800, y1, y2);
        addBand(7000, 7300, y1, y2);
        addBand(10100, 10150, y1, y2);
        addBand(14000, 14350, y1, y2);
        addBand(18068, 18168, y1, y2);
        addBand(21000, 21450, y1, y2);
        addBand(24890, 24990, y1, y2);
        addBand(27075, 27295, y1, y2);
        addBand(28000, 29700, y1, y2);
        addBand(50000, 54000, y1, y2);
        addBand(144000, 148000, y1, y2);
        addBand(220000, 225000, y1, y2);
        addBand(420000, 450000, y1, y2);
        addBand(902000, 928000, y1, y2);
        addBand(1240000, 1300000, y1, y2);
    } else {
        foreach (QString str, *_bands)
        {
            QStringList list = str.split(',');
            if (list.size() == 2)
            {
                addBand(list[0].toDouble(), list[1].toDouble(), y1, y2);
            }
        }
    }
}

void Print::addBand (double x1, double x2, double y1, double y2)
{
    QCPItemRect * xRectItem = new QCPItemRect( ui->widgetGraph );
//    m_itemRectList.append(xRectItem);

    xRectItem->setVisible          (true);
    xRectItem->setPen              (QPen(Qt::transparent));
    xRectItem->setBrush            (QBrush(QColor(50,50,150,50)));

    xRectItem->topLeft->setType(QCPItemPosition::ptPlotCoords);
    xRectItem->topLeft->setAxisRect( ui->widgetGraph->xAxis->axisRect() );
    xRectItem->topLeft->setCoords( x1, y2 );

    xRectItem->bottomRight ->setType(QCPItemPosition::ptPlotCoords);
    xRectItem->bottomRight ->setAxisRect( ui->widgetGraph->xAxis->axisRect() );
    xRectItem->bottomRight ->setCoords( x2, y1 );
}

void Print::setHead(QString string)
{
    ui->lineEditHead->setText(string);
}

void Print::on_lineSlider_valueChanged(int value)
{
    for(int i = 0; i < ui->widgetGraph->graphCount(); ++i)
    {
        QPen pen = ui->widgetGraph->graph(i)->pen();
        pen.setWidthF(value);
        ui->widgetGraph->graph(i)->setPen(pen);
    }

    ui->widgetGraph->replot();
}

void Print::on_printBtn_clicked()
{
    QPixmap map = ui->widgetGraph->toPixmap(700,400,10);

    QPixmap markersMap(ui->markersWidget->size());
    ui->markersWidget->render(&markersMap);

    QPrinter printer;

    QPrintDialog *dlg = new QPrintDialog(&printer,0);
    if(dlg->exec() == QDialog::Accepted)
    {
        QPainter painter(&printer);
        QFont font = painter.font() ;
        font.setPointSize (14);
        painter.setFont(font);

        painter.drawText(50, 10, 600, 20, Qt::TextExpandTabs , ui->lineEditHead->text());

        painter.drawImage(QRect(10,50,700,400),map.toImage());

        //painter.drawImage(QRect(70, 460, 700, 300),markersMap.toImage());
        painter.drawImage(QRect(70, 460, 700, qMin(markersMap.height(), 300)),markersMap.toImage());

        painter.drawText(70, 760, 700, 300, Qt::TextExpandTabs , ui->textEditComment->toPlainText());

        painter.end();
    }

    delete dlg;
}

void Print::on_pdfPrintBtn_clicked()
{
    QString path = QFileDialog::getSaveFileName(this, "Export PDF", m_lastPath, "*.pdf");

    QPixmap map = ui->widgetGraph->toPixmap(700,400,10);

    QPixmap markersMap(ui->markersWidget->size());
    ui->markersWidget->render(&markersMap);

    QPrinter printer;

    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);

    if(path.indexOf(".pdf") < 0)
    {
        path.append(".pdf");
    }
    m_lastPath = path;

    printer.setOutputFileName(path);

    QPainter painter(&printer);
    QFont font = ui->widgetGraph->xAxis->tickLabelFont();
    font.setPointSize (13);
    painter.setFont(font);

    painter.drawText(50, 10, 600, 30, Qt::TextExpandTabs , ui->lineEditHead->text());

    painter.drawImage(QRect(10,60,700,400),map.toImage());

    painter.drawImage(QRect(70, 470, markersMap.width(), markersMap.height()),markersMap.toImage());

    painter.drawText(70, 760, 700, 300, Qt::TextExpandTabs , ui->textEditComment->toPlainText());

    painter.end();
}

void Print::on_pngPrintBtn_clicked()
{
    QString path = QFileDialog::getSaveFileName(this, "Export PNG", "", "*.png");

    QPixmap file(2000,2000);
    file.fill();
    QPixmap map = ui->widgetGraph->toPixmap(700,400,10);

    QPixmap markersMap(ui->markersWidget->size());
    ui->markersWidget->render(&markersMap);

    QPainter painter(&file);

    QFont font = ui->widgetGraph->xAxis->tickLabelFont();
    font.setPointSize (26);
    painter.setFont(font);

    painter.drawText(100, 20, 1200, 50, Qt::TextExpandTabs , ui->lineEditHead->text());

    painter.drawImage(QRect(20,100,1400,800),map.toImage());

    painter.drawImage(QRect(70*2, 470*2, markersMap.width()*2, markersMap.height()*2),markersMap.toImage());

    painter.drawText(140, 1520, 1400, 600, Qt::TextExpandTabs , ui->textEditComment->toPlainText());

    painter.end();

    if(path.indexOf(".png") < 0)
    {
        path.append(".png");
    }
    file.save(path,"PNG",100);
}

void Print::drawSmithImage(void)
{
    QPen pen;
    pen.setColor(Qt::black);
#define ROUND_DOTS_NUM 360
    QCPCurve *round1 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round7 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round2 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round3 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round4 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round5 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round6 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);

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


    QCPCurve *round8 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round9 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurveDataMap map8;
    QCPCurveDataMap map9;
    for(double i = 0; i < 90; ++i)//1 line
    {
        map8.insert(i, QCPCurveData(i, (6 + 6 * qCos((i+179.15)/57.02)), (6 + 6 * qSin((i+179.15)/57.02))));
        map9.insert(i, QCPCurveData(i, (6 + 6 * qCos((i+179.15)/57.02)), (-1)*(6 + 6 * qSin((i+179.15)/57.02))));
    }
    round8->setData( &map8,true);
    round9->setData( &map9,true);

    QCPCurve *round10 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round11 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurveDataMap map10;
    QCPCurveDataMap map11;
    for(double i = 0; i < 53; ++i)//0.5 line
    {
        map10.insert(i, QCPCurveData(i, (6 + 12 * qCos((i+215.85)/57.02)), (12 + 12 * qSin((i+215.85)/57.02))));
        map11.insert(i, QCPCurveData(i, (6 + 12 * qCos((i+215.85)/57.02)), (-1)*(12 + 12 * qSin((i+215.85)/57.02))));
    }
    round10->setData( &map10,true);
    round11->setData( &map11,true);

    QCPCurve *round12 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round13 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurveDataMap map12;
    QCPCurveDataMap map13;
    for(double i = 0; i < 127; ++i)//2 line
    {
        map12.insert(i, QCPCurveData(i, (6 + 3 * qCos((i+142.45)/57.02)), (3 + 3 * qSin((i+142.45)/57.02))));
        map13.insert(i, QCPCurveData(i, (6 + 3 * qCos((i+142.45)/57.02)), (-1)*(3 + 3 * qSin((i+142.45)/57.02))));
    }
    round12->setData( &map12,true);
    round13->setData( &map13,true);

    QCPCurve *round14 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round15 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurveDataMap map14;
    QCPCurveDataMap map15;
    for(double i = 0; i < 151; ++i)// 5 line
    {
        map14.insert(i, QCPCurveData(i, (6 + 1.2 * qCos((i+112)/57.02)), (1.2 + 1.2 * qSin((i+112)/57.02))));//117.5
        map15.insert(i, QCPCurveData(i, (6 + 1.2 * qCos((i+112)/57.02)), (-1)*(1.2 + 1.2 * qSin((i+112)/57.02))));
    }
    round14->setData( &map14,true);
    round15->setData( &map15,true);

    QCPCurve *round16 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
    QCPCurve *round17 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
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
    QCPCurve *round18 = new QCPCurve(ui->widgetGraph->xAxis, ui->widgetGraph->yAxis);
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
    QCPItemText *center5 = new QCPItemText(ui->widgetGraph);
    QCPItemText *center2 = new QCPItemText(ui->widgetGraph);
    QCPItemText *center1 = new QCPItemText(ui->widgetGraph);
    QCPItemText *center05 = new QCPItemText(ui->widgetGraph);
    QCPItemText *center02 = new QCPItemText(ui->widgetGraph);
    QCPItemText *center0 = new QCPItemText(ui->widgetGraph);

    QCPItemText *up5 = new QCPItemText(ui->widgetGraph);
    QCPItemText *up2 = new QCPItemText(ui->widgetGraph);
    QCPItemText *up1 = new QCPItemText(ui->widgetGraph);
    QCPItemText *up05 = new QCPItemText(ui->widgetGraph);
    QCPItemText *up02 = new QCPItemText(ui->widgetGraph);

    QCPItemText *down5 = new QCPItemText(ui->widgetGraph);
    QCPItemText *down2 = new QCPItemText(ui->widgetGraph);
    QCPItemText *down1 = new QCPItemText(ui->widgetGraph);
    QCPItemText *down05 = new QCPItemText(ui->widgetGraph);
    QCPItemText *down02 = new QCPItemText(ui->widgetGraph);

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

    ui->widgetGraph->xAxis->setTicks(false);
    ui->widgetGraph->yAxis->setTicks(false);
    ui->widgetGraph->xAxis->setVisible(false);
    ui->widgetGraph->yAxis->setVisible(false);

    m_isSmithGraph = true;
    rescale();
}

void Print::rescale()
{
    if(m_isSmithGraph)
    {
        int width = ui->widgetGraph->width();
        int height = ui->widgetGraph->height();
        if(width > height)
        {
            double alfa = (double)width/height;
            double range = 14 * alfa;
            ui->widgetGraph->xAxis->setRangeLower((-1)*range/2);
            ui->widgetGraph->xAxis->setRangeUpper(range/2);
        }else
        {
            double alfa = (double)height/width;
            double range = 14 * alfa;
            ui->widgetGraph->yAxis->setRangeLower((-1)*range/2);
            ui->widgetGraph->yAxis->setRangeUpper(range/2);
        }
    }else
    {
        for(int i = 0; i < m_mTextList.length(); ++i)
        {
            double offsetX = (ui->widgetGraph->xAxis->range().upper - ui->widgetGraph->xAxis->range().lower)/40;
            double offsetY = (ui->widgetGraph->yAxis->range().upper - ui->widgetGraph->yAxis->range().lower)/10;

            m_mTextList.at(i)->position->setCoords(m_mFqList.at(i) + offsetX, ui->widgetGraph->yAxis->range().center()-offsetY);
        }
    }
    ui->widgetGraph->replot();
}

#include "screenshot.h"
#include "ui_screenshot.h"
#include "analyzer/customanalyzer.h"


Screenshot::Screenshot(QWidget *parent, int _model, int height, int width) :
    QDialog(parent),
    ui(new Ui::Screenshot),
//    m_screenCounter(0),
    m_error(0)
{
    ui->setupUi(this);
    m_analyzerModel = _model;
    m_lcdHeight = height;
    m_lcdWidth = width;

    m_popUp = new PopUp();
    QString model = CustomAnalyzer::customized() ?
                CustomAnalyzer::currentPrototype() : names[m_analyzerModel];

    if(     (model == "AA-30") ||
            (model == "AA-54")||
            (model == "AA-170")    )
    {
        m_image = new QImage(m_lcdWidth*2, m_lcdHeight*2, QImage::Format_RGB32);
    }else
    {
        m_image = new QImage(m_lcdWidth, m_lcdHeight, QImage::Format_RGB32);
    }
    m_image->fill(Qt::black);
    connect(&m_errorTimer,SIGNAL(timeout()), this, SLOT(on_errorTimerTick()));
    m_errorTimer.start(5000);
}

Screenshot::~Screenshot()
{
    if(m_image)
    {
        delete m_image;
    }
    if(m_popUp)
    {
        delete m_popUp;
    }
    delete ui;
}

void Screenshot::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    int biasx,biasy;
    QString model = CustomAnalyzer::customized() ?
                CustomAnalyzer::currentPrototype() : names[m_analyzerModel];

    QRectF r;
    if(     (model == "AA-30") ||
            (model == "AA-54")||
            (model == "AA-170")    )
    {
        biasx = (320 - m_lcdWidth*2)/2;
        biasy = (240 - m_lcdHeight*2)/2;
        r.setRect(biasx, biasy, m_lcdWidth*2, m_lcdHeight*2);
    }else if (m_lcdWidth > 320 || m_lcdHeight > 240)
    {
        biasx = 0;//(320 - m_lcdWidth/2)/2;
        biasy = 0;//(240 - m_lcdHeight/2)/2;
        r.setRect(biasx, biasy, 320, 320*m_lcdHeight/m_lcdWidth);
    }else
    {
        biasx = (320 - m_lcdWidth)/2;
        biasy = (240 - m_lcdHeight)/2;
        r.setRect(biasx, biasy, m_lcdWidth, m_lcdHeight);
    }
    //painter.drawImage(biasx, biasy, *m_image);
    //qDebug() << r;
    painter.drawImage(r, *m_image);
}

void Screenshot::on_closeBtn_clicked()
{
    emit screenshotComplete();

    this->close();
}

void Screenshot::saveBMP(QString path)
{
    m_image->save(path,"BMP",100);

    //this->close();
}

void Screenshot::on_clipboardBtn_clicked()
{
    QApplication::clipboard()->setImage(*m_image,QClipboard::Clipboard);
    m_popUp->setPopupText(tr("Image added to clipboard"));
    m_popUp->setPosition(this->geometry().x()+this->width(),this->geometry().y()+this->height());
    m_popUp->show();
}

void Screenshot::savePDF(QString path, QString comment)
{
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(path);
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Portrait);

    QPainter painter(&printer);
    painter.drawImage(QPoint(m_lcdWidth*0.7, m_lcdWidth/4), *m_image);
    QFont font = painter.font() ;
    font.setPointSize (14);
    painter.setFont(font);
    painter.drawText(QPoint(50, m_lcdWidth/4 + m_lcdHeight + 100), comment);
    painter.end();

    //this->close();
}

void Screenshot::on_lineEdit_returnPressed()
{
    QDateTime datetime = QDateTime::currentDateTime();
    QString path = "PDFs/" + datetime.toString("dd.MM.yyyy_hh.mm.ss");
    QString str = QFileDialog::getSaveFileName(this, "Export PDF", path, "*.pdf");
    if(str.isEmpty())
    {
        return;
    }
    if(str.indexOf(".pdf") == -1)
    {
        str += ".pdf";
    }
    savePDF(str, ui->lineEdit->text());
}

void Screenshot::on_newData(QByteArray data)
{
    //----------------
    QString str2;
    //----------------
    for(int i = 0; i < data.length(); ++i)
    {
        m_inputData.append((unsigned char)data.at(i));
    }

    QString model = CustomAnalyzer::customized() ?
                CustomAnalyzer::currentPrototype() : names[m_analyzerModel];

    if(     (model == "AA-30") ||
            (model == "AA-54")||
            (model == "AA-170")    )
    {
        while(m_inputData.length() >= 2)
        {
            unsigned char c1 = m_inputData.takeFirst();
            unsigned char c2 = m_inputData.takeFirst();

            if (c1 <= '9')
                c1 = c1 - '0';
            else
                c1 = c1 + 10 - 'a';

            if (c2 <= '9')
                c2 = c2 - '0';
            else
                c2 = c2 + 10 - 'a';


            c2 |= c1<<4;

            for(int i = 0; i < 8; ++i)
            {
                QRgb rgb;
                if ( c2 & (0x80>>(i&7)))
                    rgb = qRgb(0,0,0);
                else
                    rgb = qRgb(255,255,255);
                m_imageVector.append(rgb);
            }
        }
    }
    else if (model == "Stick 230") {
        while (!m_inputData.isEmpty()) {
            unsigned char data = m_inputData.takeFirst();
            //if (data != 0)
              //  qDebug() << "-------- " << data;
            int mask = 0x80;
            for (int idx=0; idx<8; idx++) {
                //QRgb color = data&mask ? Qt::white : Qt::black;
                QRgb color = data&mask ? qRgb(255, 255, 255) : qRgb(0, 0, 0);
                m_imageVector.append(color);
                mask >>= 1;
            }
        }
    }else if (model == "StickPro") {
        // TODO not implemented yet
        while(m_inputData.length() > 3)
        {
            int data = (((int)m_inputData.takeFirst())<<8);
            data += (int)m_inputData.takeFirst();
            int quantity = (int)m_inputData.takeFirst();

            if (quantity == 0)
                continue;

            int blue = data&0x1F;
            int green = (data>>5)&0x3F;
            int red = (data>>11)&0x1F;

//            if (model == "AA-2000") {
//                int tmp = red;
//                red = blue;
//                blue = tmp;
//            }
            red = (red<<3) + ( (red&0x10) ? 0x07 : 0 );
            green = (green<<2) + ( (green&0x20) ? 0x03 : 0 );
            blue = (blue<<3)+ ( (blue&0x10) ? 0x07 : 0 );

            QRgb rgb = qRgb(red,green,blue);
            for(int i = 0; i < quantity; ++i)
            {
                m_imageVector.append(rgb);
            }
        }
    }else
    {
        while(m_inputData.length() > 3)
        {
            int data = (((int)m_inputData.takeFirst())<<8);
            data += (int)m_inputData.takeFirst();
            int quantity = (int)m_inputData.takeFirst();

            if (quantity == 0)
                continue;

            int red = data&0x1F;
            int green = (data>>5)&0x3F;
            int blue = (data>>11)&0x1F;

            //if (model == "AA-230 ZOOM" || model == "AA-2000") {
            if (model == "AA-2000") {
                int tmp = red;
                red = blue;
                blue = tmp;
            }
            red = (red<<3) + ( (red&0x10) ? 0x07 : 0 );
            green = (green<<2) + ( (green&0x20) ? 0x03 : 0 );
            blue = (blue<<3)+ ( (blue&0x10) ? 0x07 : 0 );

            QRgb rgb = qRgb(red,green,blue);
            for(int i = 0; i < quantity; ++i)
            {
                m_imageVector.append(rgb);
            }
        }
    }
    int percent = m_imageVector.length()/(m_lcdHeight*m_lcdWidth/100);

    ui->progressBar->setValue(percent);
    if(     (model == "AA-30") ||
            (model == "AA-54")||
            (model == "AA-170")    )
    {
        if(m_imageVector.length() >= m_lcdHeight*m_lcdWidth)
        {
            int x,y;
            int i = 0;
            for (x = 0; x < m_lcdWidth; ++x)
            {
                for (y = 0; y < m_lcdHeight; ++y)
                {
                    for(int t = 0; t < 2; ++t)
                    {
                        for(int n = 0; n < 2; ++n)
                        {
                            m_image->setPixel ((x*2)+t,(y*2)+n,m_imageVector.at(i));
                        }
                    }
                    ++i;
                }
            }
            m_inputData.clear();
            m_imageVector.clear();
            repaint();
        }
    } else if (model == "Stick 230") {
        //qDebug() << "AA-230 Stick: estimated = " << QString("%1 , obtained = %2").arg(m_lcdHeight*m_lcdWidth).arg(m_imageVector.length());
        if(m_imageVector.length() >= m_lcdHeight*m_lcdWidth)
        {
            int x,y;
            int i = 0;
            for (y = 0; y < m_lcdHeight; ++y)
            {
                for (x = 0; x < m_lcdWidth; ++x)
                {
                    m_image->setPixel (x, y, m_imageVector.at(i));
                    i++;
                }
            }
            m_inputData.clear();
            m_imageVector.clear();
            repaint();
        }
    } else if (model == "StickPro") {
        // TODO not implemented yet
        //qDebug() << "AA-230 Stick: estimated = " << QString("%1 , obtained = %2").arg(m_lcdHeight*m_lcdWidth).arg(m_imageVector.length());
        if(m_imageVector.length() >= m_lcdHeight*m_lcdWidth)
        {
            int x,y;
            int i = 0;
            for (y = 0; y < m_lcdHeight; ++y)
            {
                for (x = 0; x < m_lcdWidth; ++x)
                {
                    m_image->setPixel (x, y, m_imageVector.at(i));
                    i++;
                }
            }
            m_inputData.clear();
            m_imageVector.clear();
            repaint();
        }
    } else if((m_imageVector.length() >= m_lcdHeight*m_lcdWidth)
    //          || ( (model == "AA-230 ZOOM")&&(m_imageVector.length() >= 63604) )
              )
    {
            ui->progressBar->setValue(100);
            QApplication::clipboard()->setImage(*m_image,QClipboard::Clipboard);
            m_popUp->setPopupText(tr("Image added to clipboard"));
            m_popUp->setPosition(this->geometry().x()+this->width(),this->geometry().y()+this->height());
            //m_popUp->show();
            int x,y;
            int i = 0;
            for (x = 0; x < m_lcdHeight; ++x)
            {
                for (y = 0; y < m_lcdWidth; ++y)
                {
                    if(m_imageVector.length() > i)
                    {
                        m_image->setPixel (y,x,m_imageVector.at(i));
                        ++i;
                    }else
                    {
                        repaint();
                        return;
                    }
                }
            }


            QString str;
            for(int idx=0; idx<m_lcdWidth*2; idx++) {
                QString str1 = QString("%1 ").arg(m_imageVector.at(idx), 8, 16, QLatin1Char('0') );
                str += str1;
            }

            //emit screenshotComplete();
            m_inputData.clear();
            m_imageVector.clear();
            repaint();
        }

}

void Screenshot::on_saveAsBtn_clicked()
{
    QDateTime datetime = QDateTime::currentDateTime();
    QString path = "Images/" + datetime.toString("dd.MM.yyyy_hh.mm.ss");
    QString str = QFileDialog::getSaveFileName(this, "Export BMP", path, "*.bmp");
    if(str.isEmpty())
    {
        return;
    }
    if(str.indexOf(".bmp") == -1)
    {
        str += ".bmp";
    }
    saveBMP(str);
}

void Screenshot::on_exportToPdfBtn_clicked()
{
    QDateTime datetime = QDateTime::currentDateTime();
    QString path = "PDFs/" + datetime.toString("dd.MM.yyyy_hh.mm.ss");
    QString str = QFileDialog::getSaveFileName(this, "Export PDF", path, "*.pdf");
    if(str.isEmpty())
    {
        return;
    }
    if(str.indexOf(".pdf") == -1)
    {
        str += ".pdf";
    }
    savePDF(str, ui->lineEdit->text());
}

void Screenshot::on_refreshBtn_clicked()
{
    /*
    //{ debug
    QStringList list = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    QDir dir = list[1];
    QString binFileName = dir.absoluteFilePath("Rigexpert/Antscope2/antscope-screen.bin");
    QFile binFile(binFileName);
    if (binFile.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        QDataStream stream(&binFile);
        for (int idx=0; idx<m_inputDataDebug.size(); idx++)
        {
            stream << m_inputDataDebug[idx];
        }
        binFile.close();
    }

    QString textFileName = dir.absoluteFilePath("Rigexpert/Antscope2/antscope-screen.txt");
    QFile textFile(textFileName);
    if (textFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
    {
        QTextStream  stream(&textFile);
        for (int idx=0; idx<m_inputDataDebug.size(); idx++)
        {
            if (idx > 0 && (idx % 16) == 0)
                stream << endl;
            QString str = QString("%1 ").arg(m_inputDataDebug[idx], 2, 16, QLatin1Char('0')).toUpper();
            stream << str;
        }
        stream.flush();
        textFile.close();
    }
    m_inputDataDebug.clear();
    //} debug
*/
    m_inputData.clear();
    m_imageVector.clear();
    repaint();
    emit newScreenshot();
}

void Screenshot::on_errorTimerTick()
{
    if ((m_lcdHeight == 0) || (m_lcdWidth == 0)) {
        m_errorTimer.stop();
        QMessageBox::warning(NULL, tr("Error"), tr("Screenshot not supported on this device."));
        ui->progressBar->setValue(100);
        m_inputData.clear();
        m_imageVector.clear();
        m_error = 0;
        this->close();
        return;
    }
    if((m_error == m_imageVector.length()/(m_lcdHeight*m_lcdWidth/100)) && m_error != 0 && m_error != 100)
    {
        QMessageBox::warning(NULL, tr("Error"), tr("Error while make screenshot. Please try again."));
        ui->progressBar->setValue(100);
        m_inputData.clear();
        m_imageVector.clear();
    }
    m_error = m_imageVector.length()/(m_lcdHeight*m_lcdWidth/100);
}

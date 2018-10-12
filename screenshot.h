#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QDialog>
#include <QPainter>
#include <QFileDialog>
#include <QDebug>
#include <QPrinter>
#include <QTextDocument>
#include <QDateTime>
#include <analyzer/analyzerparameters.h>
#include <QClipboard>
#include <popup.h>

namespace Ui {
class Screenshot;
}

class Screenshot : public QDialog
{
    Q_OBJECT

public:
    explicit Screenshot(QWidget *parent = 0,int model = 0, int height = 0, int width = 0);
    ~Screenshot();


private:
    Ui::Screenshot *ui;
    int m_lcdHeight;
    int m_lcdWidth;
    QVector <unsigned char> m_inputData;
    QVector <unsigned char> m_parsedData;
    QVector <QRgb> m_imageVector;
//    int m_screenCounter;
    QImage *m_image;
    QImage *m_imageToPdf;

    quint32 m_analyzerModel;

    PopUp *m_popUp;

    qint32 m_error;
    QTimer m_errorTimer;

    void saveBMP(QString path);
    void savePDF(QString path, QString comment);

    //{ debug
    QVector <unsigned char> m_inputDataDebug;
    //} debug

signals:
    void screenshotComplete();
    void newScreenshot();

public slots:
    void paintEvent(QPaintEvent *);
private slots:
    void on_closeBtn_clicked();
    void on_lineEdit_returnPressed();
    void on_newData(QByteArray data);
    void on_saveAsBtn_clicked();
    void on_exportToPdfBtn_clicked();
    void on_refreshBtn_clicked();
    void on_clipboardBtn_clicked();
    void on_errorTimerTick();
};

#endif // SCREENSHOT_H

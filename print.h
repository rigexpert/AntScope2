#ifndef PRINT_H
#define PRINT_H

#include <QDialog>
#include <analyzer/analyzerparameters.h>
#include <markers.h>
#include <QSettings>
#include <settings.h>

namespace Ui {
class Print;
}

class Print : public QDialog
{
    Q_OBJECT

public:
    explicit Print(QWidget *parent = 0);
    ~Print();
    void addMarker(double fq, int number);

    void setRange(QCPRange x, QCPRange y);
    void setLabel(QString xLabel, QString yLabel);
    void setData(QCPDataMap *m, QPen pen, QString name);
    void setSmithData(QCPCurveDataMap *map, QPen pen, QString name);

    void drawBands(QStringList* _bands, double y1, double y2);
    void addBand (double x1, double x2, double y1, double y2);
    void setHead(QString string);

    void addRowText(const QStringList &list);
    void updateTable();
    void drawSmithImage (void);

    void rescale();


private slots:
    void on_lineSlider_valueChanged(int value);
    void on_printBtn_clicked();
    void on_pdfPrintBtn_clicked();
    void on_pngPrintBtn_clicked();

private:
    Ui::Print *ui;
    QSettings * m_settings;
    QString m_lastPath;

//    QGridLayout m_layout;

    QLabel m_markersNumberLabel;
    QLabel m_measurementsNumberLabel;
    QLabel m_fqLabel;
    QLabel m_swrLabel;
    QLabel m_rlLabel;
    QLabel m_zLabel;
    QLabel m_phaseLabel;


    QStringList m_markersNumberList;
    QStringList m_measurementsNumberList;
    QStringList m_fqList;
    QStringList m_swrList;
    QStringList m_rlList;
    QStringList m_zList;
    QStringList m_phaseList;

    QList <QLabel*> m_markersNumberObjList;
    QList <QLabel*> m_measurementsNumberObjList;
    QList <QLabel*> m_fqObjList;
    QList <QLabel*> m_swrObjList;
    QList <QLabel*> m_rlObjList;
    QList <QLabel*> m_zObjList;
    QList <QLabel*> m_phaseObjList;    

    QVector <double> m_mFqList;
    QVector <QCPCurve*> m_curveList;
    QVector <QCPItemStraightLine*> m_mStraightLineList;
    QVector <QCPItemText*> m_mTextList;
    QVector <QCPCurveDataMap*> m_curveDataList;
    QVector <QCPItemText*> m_textList;

    bool m_isSmithGraph;

};

#endif // PRINT_H

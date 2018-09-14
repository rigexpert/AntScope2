#ifndef MARKERSPOPUP_H
#define MARKERSPOPUP_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QTimer>
#include <QSettings>
#include <settings.h>

class MarkersPopUp : public QWidget
{
    Q_OBJECT

    // Свойство полупрозрачности
    Q_PROPERTY(float popupOpacity READ getPopupOpacity WRITE setPopupOpacity)

    void setPopupOpacity(float opacity);
    float getPopupOpacity() const;

public:
    explicit MarkersPopUp(QWidget *parent = 0);
    ~MarkersPopUp();
    void setName(QString name);
    int getDurability (void) const {return m_durability;}
    void setDurability (int durability) {m_durability = durability;}
    bool getHiding (void) const {return m_hiding;}
    void setHiding (bool hiding) {m_hiding = hiding;}

    void setX(int x){m_x = x;}
    void setY(int y){m_y = y;}
    void setPosition(int x, int y);
    void setParentPosition(int x, int y)
    {
        m_parentX = x;
        m_parentY = y;
    }

    void setBackgroundColor(QColor color){ m_bgColor = color;}
    void setPenColor(QColor color){ m_penColor = color;}
    void setTextColor(QString color);

    void MainWindowPos(int x, int y);
    void on_translate();

protected:
    void paintEvent(QPaintEvent *event);    // Фон будет отрисовываться через метод перерисовки

public slots:
    QString getPopupText();
    QList <QStringList> getPopupList();

//    void show();                            /* Собственный метод показа виджета
//                                             * Необходимо для преварительной настройки анимации
//                                             * */
    void focusShow();
    void focusHide();

    void addRowText(int markerNumber, QVector<int> *measurement, QVector<double> *fq, QVector<double> *swr, QVector<double> *rl, QVector<QString> *z, QVector<double> *phase);
    void clearTable(void);
    void on_remove();

private slots:
    void show();
    void hideAnimation();                   // Слот для запуска анимации скрытия
    void hide();                            /* По окончании анимации, в данном слоте делается проверка,
                                             * виден ли виджет, или его необходимо скрыть
                                             * */

signals:
    void removeMarker(int);


private:
    QLabel m_removeLabel;
    QLabel m_numberLabel;
    QLabel m_measurementLabel;
    QLabel m_fqLabel;
    QLabel m_swrLabel;
    QLabel m_rlLabel;
    QLabel m_zLabel;
    QLabel m_phaseLabel;


    QStringList m_markersList;
    QStringList m_measurementsList;
    QStringList m_fqList;
    QStringList m_swrList;
    QStringList m_rlList;
    QStringList m_zList;
    QStringList m_phaseList;

    QList <QPushButton*> m_buttonsObjList;
    QList <QLabel*> m_markersObjList;
    QList <QLabel*> m_measurementsObjList;
    QList <QLabel*> m_fqObjList;
    QList <QLabel*> m_swrObjList;
    QList <QLabel*> m_rlObjList;
    QList <QLabel*> m_zObjList;
    QList <QLabel*> m_phaseObjList;


    QGridLayout m_layout;
    QPropertyAnimation animation;
    float popupOpacity;
    QTimer *m_timer;
    int m_durability;
    bool m_hiding;
    int m_x;
    int m_y;
    int m_biasX;
    int m_biasY;
    int m_mainX;
    int m_mainY;
    int m_mainBiasX;
    int m_mainBiasY;
    int m_parentX;
    int m_parentY;

    QColor m_bgColor;
    QColor m_penColor;
    QString m_textColor;

    QString m_name;

    QSettings *m_settings;

    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent *);

    void updateTable();
};

#endif // MARKERSPOPUP_H

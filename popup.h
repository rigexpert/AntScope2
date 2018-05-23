#ifndef POPUP_H
#define POPUP_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QTimer>
#include <QSettings>
#include <settings.h>

class PopUp : public QWidget
{
    Q_OBJECT

    // Свойство полупрозрачности
    Q_PROPERTY(float popupOpacity READ getPopupOpacity WRITE setPopupOpacity)

    void setPopupOpacity(float opacity);
    float getPopupOpacity() const;

public:
    explicit PopUp(QWidget *parent = 0);
    ~PopUp();
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
    void setTextColor(QString color){
        m_textColor = color;
        label.setStyleSheet("QLabel { color : " + m_textColor + ";"
                            "margin-top: 6px;"
                            "margin-bottom: 6px;"
                            "margin-left: 10px;"
                            "margin-right: 10px; }");}

    void MainWindowPos(int x, int y);

protected:
    void paintEvent(QPaintEvent *event);    // Фон будет отрисовываться через метод перерисовки

public slots:
    void setPopupText(const QString& text); // Установка текста в уведомление
    QString getPopupText();
    void show();                            /* Собственный метод показа виджета
                                             * Необходимо для преварительной настройки анимации
                                             * */
    void focusShow();
    void focusHide();

private slots:
    void hideAnimation();                   // Слот для запуска анимации скрытия
    void hide();                            /* По окончании анимации, в данном слоте делается проверка,
                                             * виден ли виджет, или его необходимо скрыть
                                             * */


private:
    QLabel label;           // Label с сообщением
//    QPushButton button;     // Кнопка сворачивания
//    QGridLayout layout;     // Размещение для лейбла
    QVBoxLayout layout;
    QPropertyAnimation animation;   // Свойство анимации для всплывающего сообщения
    float popupOpacity;     // Свойства полупрозрачности виджета
    QTimer *timer;          // Таймер, по которому виджет будет скрыт
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
};

#endif // POPUP_H

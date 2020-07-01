#ifndef POPUPINDICATOR_H
#define POPUPINDICATOR_H

#include <popup.h>

class PopUpIndicator : public PopUp {

    static PopUpIndicator* m_popUpIndicator;
public:
    static void showIndicator(QWidget* parent=0);
    static void hideIndicator(QWidget* parent=0);
    static void setIndicatorVisible(bool visible);

protected:
    explicit PopUpIndicator(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent * ) {}
    void mouseMoveEvent(QMouseEvent *) {}

private:

public slots:
    void show();
};


#endif // POPUPINDICATOR_H

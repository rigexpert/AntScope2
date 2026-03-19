#ifndef STYLE_H
#define STYLE_H

#include <QWidget>

class Style
{
public:
    static QString label();
    static QString pushButton(bool checkable=false);
    static QString lineEdit();
    static QString tabWidget();
    static QString checkBox();
    static QString groupBox();
    static QString spinBox();
    static QString tableWidget();
    static QString headerView();
    static QString radioButton();
    static QString toolButton();
    static QString comboBox();
    static QString progressBar();
    static QString dialog();
};

#endif // STYLE_H

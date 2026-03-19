#include "style.h"

QString Style::label()
{
    QString style;

    style = "QLabel {color: white; }";
    style += "QLabel:disabled {color: grey; }";
    return style;
}

QString Style::pushButton(bool checkable)
{
    QString style;
    if (checkable)    {
        style += "QPushButton {background-color: rgb(26, 45, 98); color: white; border: none;} ";
        style += "QPushButton:checked {background-color: rgb(26, 45, 198); color: white;} ";
    } else {
        style = "QPushButton {background-color: rgb(26, 45, 98); color: white;}";
        //style += "QPushButton:disabled {background-color: #06171f; color: gray;};";
        //style += "QPushButton:checked {background-color: #10215d; color: white;};";
        style += "QPushButton:disabled {background-color: rgb(2, 9, 12); color: gray;};";
        style += "QPushButton:checked {background-color: rgb(6, 13, 37); color: white;};";
    }
    return style;
}

QString Style::lineEdit()
{
    QString style;

    style = "QLineEdit {background: rgb(26, 45, 98); color: white};";
    style += "QLineEdit[readOnly=""true""] {background-color: rgb(0, 0, 98); color: red};";
    style += "QLineEdit:disabled {background-color: green; color: red};";
    return style;
}

QString Style::tabWidget()
{
    QString style;

    style = "QTabBar::tab:selected {background: rgb(26, 45, 198); color: white;}";
    style += "QTabBar::tab:hover {background: rgb(126, 145, 198); color: gray;}";
    style += "QTabBar::tab:disabled {background-color: rgb(26, 45, 98); color: white;}";
    style += "QTabBar::tab {background-color: rgb(26, 45, 98); color: white;}";
    style += "QTabWidget::pane {background-color: #2F2F2F; border: 1px solid #C2C7CB;}";
    return style;
}

QString Style::checkBox()
{
    QString style;

    style = "QCheckBox {color: white; }";
    style += "QCheckBox:indicator:checked {image: url (:/new/prefix1/checker.png); background-color: rgb(26, 45, 198);}";
    //style += "QCheckBox:indicator:checked {image: url (:/new/prefix1/checker.png)}";
    return style;
}

QString Style::groupBox()
{
    QString style;

    style = "QGroupBox:title {color: white; padding: 3px 0; subcontrol-origin: 1ex;}";
    style += "QGroupBox {border: 2px solid gray; border-radius: 5px;}";
    return style;
}

QString Style::spinBox()
{
    QString style;

    style = "QSpinBox {background-color: rgb(26, 45, 98); color: white} ";
    return style;
}

QString Style::tableWidget()
{
    QString style;

    style = "QTableWidget {"
            "background-color: #2F2F2F;"
            "border: 1px solid #4181C0;"
            "color: white;"
            "selection-background-color: #4181C0; /* Color when an item is selected */"
            "selection-color: green;"
            "}";
    style += "QScrollBar:horizontal { height: 5px; background: rgb(26, 45, 98); border: 1px solid green;}";
    style += "QScrollBar::handle:horizontal {background: rgb(26, 45, 198); min-width: 20px;}";
    style += "QScrollBar:vertical { width: 5px; background: rgb(26, 45, 98); }";
    style += "QScrollBar::handle:vertical {background: rgb(26, 45, 198); min-width: 20px;}";
    return style;
}

QString Style::headerView()
{
    QString style;

    style = "QHeaderView::section {"
            "    background-color: rgb(26, 45, 98);"
            "    color: white;"
            "    border: 1px solid black;"
            "    padding: 5px;"
            "    font-size: 12px;"
            "    text-align: center;"
            "}";
    return style;
}

QString Style::radioButton()
{
    QString style;
    style = "QRadioButton {color: white;}";
    style += "QRadioButton::indicator::unchecked{ border: 1px solid; border-color: gray;"
             " border-radius: 5px; background-color: rgb(26, 45, 98); width: 11px; height: 11px; }";
    style += "QRadioButton::indicator::checked{ border: 1px solid; border-color: white;"
             " border-radius: 5px; background-color: rgb(26, 45, 198); width: 11px; height: 11px; }";

    return style;
}

QString Style::toolButton()
{
    QString style = "QToolButton {background-color: rgb(26, 45, 198); color: white; }";
    return style;
}

QString Style::comboBox()
{
    QString style;
    style += "QComboBox {background-color: rgb(1, 70, 100); color: white; border: 1px solid #555; padding: 5px;}";
    style += "QComboBox QAbstractItemView {background-color: rgb(1, 70, 100);color: white; selection-background-color: #0078d7; selection-color: white;";
    return style;
}

QString Style::progressBar()
{
    QString style = R"(
    QProgressBar {
        border: 1px solid #555;
        border-radius: 5px;
        background-color: #2b2b2b;
        text-align: center;
        color: white;
    }

    QProgressBar::chunk {
        background-color: #00aaff;
        border-radius: 5px;
    }
    )";
    return style;
}

QString Style::dialog()
{
    return "QDialog{background-color: #2F2F2F}";

}

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
        style = "QPushButton {background-color: rgb(66, 85, 138); color: white; ";
        style += " border-radius: 3px; border: 1px solid rgb(26, 45, 98); padding: 2px;} ";
        style += "QPushButton:disabled {background-color: rgb(2, 9, 12); color: gray;} ";
        style += "QPushButton:checked {background-color: rgb(1, 178, 255); color: white;} ";
    return style;
}

QString Style::lineEdit()
{
    QString style;

    style = "QLineEdit {background: rgb(26, 45, 198); color: white;} ";
    style += "QLineEdit[readOnly=""true""] {background-color: rgb(0, 0, 98); color: white;} ";
    style += "QLineEdit:disabled {rgb(26, 45, 98); color: white;} ";
    return style;
}

QString Style::tabWidget()
{
    QString style;

    style += "QTabWidget::pane {background-color: rgb(18, 18, 18); border: 1px solid rgb(76, 78, 80); }";
    style += "QTabBar::tab {background: rgb(26, 45, 98); color: white; } ";
    style += "QTabBar::tab:hover {background: rgb(126, 145, 198); color: gray; } ";
    style += "QTabBar::tab:disabled {background: rgb(26, 45, 98); color: white; }";
    style += "QTabBar::tab:selected {background: rgb(26, 45, 198); color: white; } ";
    style += "QTabBar::tab:!selected {background: rgb(26, 45, 98); color: white; } ";
    return style;
}

QString Style::checkBox()
{
    QString style;

    style = "QCheckBox {color: white; } ";
    style += "QCheckBox::indicator:checked {image: url(:/new/prefix1/checked.png);} ";
    style += "QCheckBox::indicator:unchecked {image: url(:/new/prefix1/unchecked.png);} ";
    return style;
}

QString Style::groupBox()
{
    QString style;

    style = "QGroupBox:title {color: white; padding: 3px 0; subcontrol-origin: 1ex;} ";
    style += "QGroupBox {border: 2px solid gray; border-radius: 5px;} ";
    return style;
}

QString Style::spinBox()
{
    QString style;

    style = "QSpinBox {background-color: rgb(26, 45, 198); color: white;} ";
    return style;
}

QString Style::tableWidget()
{
    QString style;

    style = "QTableWidget {background-color: rgb(18, 18, 18); border: 1px solid rgb(25, 51, 75);color: white;} "
            "QTableWidget::item:selected {background-color: rgb(26, 45, 198); color: white;} "
            "QTableWidget::indicator:checked {image: url(:/new/prefix1/checked.png);} "
            "QTableWidget::indicator:unchecked {image: url(:/new/prefix1/unchecked.png);} "
        ;
    style += "QScrollBar:horizontal { height: 5px; background: rgb(26, 45, 98); ;} ";
    style += "QScrollBar::handle:horizontal {background: rgb(26, 45, 198); min-width: 20px;} ";
    style += "QScrollBar:vertical { width: 5px; background: rgb(26, 45, 98); } ";
    style += "QScrollBar::handle:vertical {background: rgb(26, 45, 198); min-width: 20px;} ";
    return style;
}

QString Style::listWidget()
{
    QString style;

    style = "QListWidget {"
            "background-color: rgb(18, 18, 18);"
            "border: 1px solid rgb(25, 51, 75);"
            "color: white;"
            "selection-background-color: rgb(32, 60, 82); /* Color when an item is selected */"
            "selection-color: white;"
            "} "
            "QListWidget::item:selected { background-color: rgb(0, 41, 84); color: white;} ";

    style += "QScrollBar:horizontal { height: 5px; background: rgb(26, 45, 98);} ";
    style += "QScrollBar::handle:horizontal {background: rgb(26, 45, 198); min-width: 20px;} ";
    style += "QScrollBar:vertical { width: 5px; background: rgb(26, 45, 98); } ";
    style += "QScrollBar::handle:vertical {background: rgb(26, 45, 198); min-width: 20px;} ";
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
            "} ";
    return style;
}

QString Style::radioButton()
{
    QString style;
    style = "QRadioButton {color: white;} ";
    style += "QRadioButton::indicator::unchecked{ border: 1px solid; border-color: gray;"
             " border-radius: 5px; background-color: rgb(26, 45, 98); width: 11px; height: 11px; } ";
    style += "QRadioButton::indicator::checked{ border: 1px solid; border-color: white;"
             " border-radius: 5px; background-color: rgb(26, 45, 198); width: 11px; height: 11px; } ";

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
    style += "QComboBox {background-color: rgb(1, 70, 100); color: white; border: 1px solid rgb(33, 33, 33); padding: 5px;} ";
    style += "QComboBox QAbstractItemView {background-color: rgb(1, 70, 100);color: white; "
             "selection-background-color: #0078d7; selection-color: white;} ";
    return style;
}

QString Style::progressBar()
{
    QString style = R"(
    QProgressBar {
        border: 1px solid rgb(33, 33, 33);
        border-radius: 5px;
        background-color: rgb(17, 17, 17);
        text-align: center;
        color: white;
    }

    QProgressBar::chunk {
        background-color: rgb(0, 68, 100);
        border-radius: 5px;
    }
    )";
    return style;
}

QString Style::slider()
{
    QString style = R"(
    QSlider::groove:horizontal {
        height: 4px;
        background: rgb(17, 17, 17);
        border-radius: 3px;
    }

    QSlider::sub-page:horizontal {
        background: rgb(0, 68, 100);
        border-radius: 1px;
    }

    QSlider::add-page:horizontal {
        background: rgb(33, 33, 33);
        border-radius: 1px;
    }

    QSlider::handle:horizontal {
        background: rgb(0, 68, 100);
        border: 1px solid rgb(0, 47, 67);
        width: 5px;
        margin: -5px 0;
        border-radius: 5px;
    }

    QSlider::handle:horizontal:hover {
        background: rgb(20, 73, 100);
    }

    QSlider::handle:horizontal:pressed {
        background: rgb(0, 53, 80);
    }
    )";
    return style;
}

QString Style::dialog()
{
    return "QDialog{background-color: rgb(18, 18, 18);} ";
}

QString Style::messageBox()
{
    QString style = R"(
    QMessageBox {
        background-color: rgb(18, 18, 18);
    }

    QMessageBox QLabel {
        color: white;
    }

    QMessageBox QPushButton {
        background-color: rgb(26, 45, 98);
        color: white;
        border: 1px solid rgb(33, 33, 33);
        padding: 5px 12px;
        border-radius: 4px;
        min-width: 80px;
    }

    QMessageBox QPushButton:hover {
        background-color: rgb(33, 33, 33);
    }

    QMessageBox QPushButton:pressed {
        background-color: rgb(33, 33, 33);
    }
    )";
    return style;
}

QString Style::fileDialog()
{
    QString style = (R"(
    QFileDialog {
        background-color: rgb(18, 18, 18);
        color: white;
    }
    QPushButton {
        background-color: rgb(33, 33, 33);
        color: white;
        border: 1px solid rgb(33, 33, 33);
        padding: 5px;
    }
    QPushButton:hover {
        background-color: rgb(33, 33, 33);
    }
    QLineEdit {
        background-color: rgb(24, 24, 24);
        color: white;
        border: 1px solid rgb(33, 33, 33);
    }
    QListView, QTreeView {
        background-color: rgb(18, 18, 18);
        alternate-background-color: rgb(24, 24, 24);
    }
    )");
    return style;
}


/********************************************************************************
** Form generated from reading UI file 'screenshot.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCREENSHOT_H
#define UI_SCREENSHOT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Screenshot
{
public:
    QPushButton *closeBtn;
    QPushButton *saveAsBtn;
    QPushButton *exportToPdfBtn;
    QLineEdit *lineEdit;
    QLabel *label;
    QProgressBar *progressBar;
    QPushButton *refreshBtn;

    void setupUi(QDialog *Screenshot)
    {
        if (Screenshot->objectName().isEmpty())
            Screenshot->setObjectName(QStringLiteral("Screenshot"));
        Screenshot->resize(320, 350);
        Screenshot->setMinimumSize(QSize(320, 350));
        Screenshot->setMaximumSize(QSize(320, 350));
        closeBtn = new QPushButton(Screenshot);
        closeBtn->setObjectName(QStringLiteral("closeBtn"));
        closeBtn->setGeometry(QRect(230, 320, 81, 23));
        saveAsBtn = new QPushButton(Screenshot);
        saveAsBtn->setObjectName(QStringLiteral("saveAsBtn"));
        saveAsBtn->setGeometry(QRect(140, 320, 81, 23));
        exportToPdfBtn = new QPushButton(Screenshot);
        exportToPdfBtn->setObjectName(QStringLiteral("exportToPdfBtn"));
        exportToPdfBtn->setGeometry(QRect(50, 320, 81, 23));
        lineEdit = new QLineEdit(Screenshot);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));
        lineEdit->setGeometry(QRect(10, 290, 301, 20));
        label = new QLabel(Screenshot);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 270, 121, 16));
        progressBar = new QProgressBar(Screenshot);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setGeometry(QRect(10, 240, 301, 23));
        progressBar->setValue(0);
        refreshBtn = new QPushButton(Screenshot);
        refreshBtn->setObjectName(QStringLiteral("refreshBtn"));
        refreshBtn->setGeometry(QRect(10, 320, 31, 23));

        retranslateUi(Screenshot);

        QMetaObject::connectSlotsByName(Screenshot);
    } // setupUi

    void retranslateUi(QDialog *Screenshot)
    {
        Screenshot->setWindowTitle(QApplication::translate("Screenshot", "Dialog", 0));
        closeBtn->setText(QApplication::translate("Screenshot", "Close", 0));
        saveAsBtn->setText(QApplication::translate("Screenshot", "Export to BMP", 0));
        exportToPdfBtn->setText(QApplication::translate("Screenshot", "Export to PDF", 0));
        label->setText(QApplication::translate("Screenshot", "Add comment (for .pdf):", 0));
        refreshBtn->setText(QApplication::translate("Screenshot", "R", 0));
    } // retranslateUi

};

namespace Ui {
    class Screenshot: public Ui_Screenshot {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCREENSHOT_H

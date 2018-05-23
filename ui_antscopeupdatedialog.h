/********************************************************************************
** Form generated from reading UI file 'antscopeupdatedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ANTSCOPEUPDATEDIALOG_H
#define UI_ANTSCOPEUPDATEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_AntScopeUpdateDialog
{
public:
    QProgressBar *downloadProgressBar;
    QLabel *installUpdates;
    QPushButton *dowloadNow;
    QPushButton *close;
    QLabel *newVersion;
    QPushButton *afterClosing;

    void setupUi(QDialog *AntScopeUpdateDialog)
    {
        if (AntScopeUpdateDialog->objectName().isEmpty())
            AntScopeUpdateDialog->setObjectName(QStringLiteral("AntScopeUpdateDialog"));
        AntScopeUpdateDialog->resize(700, 141);
        downloadProgressBar = new QProgressBar(AntScopeUpdateDialog);
        downloadProgressBar->setObjectName(QStringLiteral("downloadProgressBar"));
        downloadProgressBar->setGeometry(QRect(10, 50, 681, 31));
        downloadProgressBar->setValue(0);
        installUpdates = new QLabel(AntScopeUpdateDialog);
        installUpdates->setObjectName(QStringLiteral("installUpdates"));
        installUpdates->setGeometry(QRect(190, 10, 501, 31));
        QFont font;
        font.setPointSize(16);
        installUpdates->setFont(font);
        dowloadNow = new QPushButton(AntScopeUpdateDialog);
        dowloadNow->setObjectName(QStringLiteral("dowloadNow"));
        dowloadNow->setGeometry(QRect(550, 100, 141, 31));
        QFont font1;
        font1.setPointSize(10);
        dowloadNow->setFont(font1);
        close = new QPushButton(AntScopeUpdateDialog);
        close->setObjectName(QStringLiteral("close"));
        close->setGeometry(QRect(400, 100, 141, 31));
        close->setFont(font1);
        newVersion = new QLabel(AntScopeUpdateDialog);
        newVersion->setObjectName(QStringLiteral("newVersion"));
        newVersion->setGeometry(QRect(140, 10, 441, 31));
        newVersion->setFont(font);
        afterClosing = new QPushButton(AntScopeUpdateDialog);
        afterClosing->setObjectName(QStringLiteral("afterClosing"));
        afterClosing->setGeometry(QRect(330, 100, 211, 31));
        afterClosing->setFont(font1);

        retranslateUi(AntScopeUpdateDialog);

        QMetaObject::connectSlotsByName(AntScopeUpdateDialog);
    } // setupUi

    void retranslateUi(QDialog *AntScopeUpdateDialog)
    {
        AntScopeUpdateDialog->setWindowTitle(QApplication::translate("AntScopeUpdateDialog", "Dialog", 0));
        installUpdates->setText(QApplication::translate("AntScopeUpdateDialog", "Do you want to install new version?", 0));
        dowloadNow->setText(QApplication::translate("AntScopeUpdateDialog", "Download and Install", 0));
        close->setText(QApplication::translate("AntScopeUpdateDialog", "Close", 0));
        newVersion->setText(QApplication::translate("AntScopeUpdateDialog", "There are new version of AntScope2 available!", 0));
        afterClosing->setText(QApplication::translate("AntScopeUpdateDialog", "Download and Install after closing", 0));
    } // retranslateUi

};

namespace Ui {
    class AntScopeUpdateDialog: public Ui_AntScopeUpdateDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ANTSCOPEUPDATEDIALOG_H

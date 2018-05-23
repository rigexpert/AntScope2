/********************************************************************************
** Form generated from reading UI file 'updatedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UPDATEDIALOG_H
#define UI_UPDATEDIALOG_H

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

class Ui_UpdateDialog
{
public:
    QLabel *label;
    QPushButton *updateBtn;
    QPushButton *cancelBtn;
    QProgressBar *progressBar;
    QLabel *statusLabel;

    void setupUi(QDialog *UpdateDialog)
    {
        if (UpdateDialog->objectName().isEmpty())
            UpdateDialog->setObjectName(QStringLiteral("UpdateDialog"));
        UpdateDialog->setEnabled(true);
        UpdateDialog->resize(474, 154);
        label = new QLabel(UpdateDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 10, 461, 31));
        QFont font;
        font.setPointSize(14);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);
        updateBtn = new QPushButton(UpdateDialog);
        updateBtn->setObjectName(QStringLiteral("updateBtn"));
        updateBtn->setGeometry(QRect(380, 120, 75, 23));
        cancelBtn = new QPushButton(UpdateDialog);
        cancelBtn->setObjectName(QStringLiteral("cancelBtn"));
        cancelBtn->setGeometry(QRect(300, 120, 75, 23));
        progressBar = new QProgressBar(UpdateDialog);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setEnabled(true);
        progressBar->setGeometry(QRect(10, 50, 451, 23));
        progressBar->setValue(0);
        statusLabel = new QLabel(UpdateDialog);
        statusLabel->setObjectName(QStringLiteral("statusLabel"));
        statusLabel->setGeometry(QRect(180, 80, 121, 16));

        retranslateUi(UpdateDialog);

        QMetaObject::connectSlotsByName(UpdateDialog);
    } // setupUi

    void retranslateUi(QDialog *UpdateDialog)
    {
        UpdateDialog->setWindowTitle(QApplication::translate("UpdateDialog", "Dialog", 0));
        label->setText(QString());
        updateBtn->setText(QApplication::translate("UpdateDialog", "Update", 0));
        cancelBtn->setText(QApplication::translate("UpdateDialog", "Close", 0));
        statusLabel->setText(QApplication::translate("UpdateDialog", "Updating, please wait...", 0));
    } // retranslateUi

};

namespace Ui {
    class UpdateDialog: public Ui_UpdateDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UPDATEDIALOG_H

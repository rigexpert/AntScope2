/********************************************************************************
** Form generated from reading UI file 'export.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXPORT_H
#define UI_EXPORT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_Export
{
public:
    QPushButton *csvBtn;
    QPushButton *nwlBtn;
    QGroupBox *groupBox;
    QPushButton *zRiBtn;
    QPushButton *sRiBtn;
    QPushButton *sMaBtn;

    void setupUi(QDialog *Export)
    {
        if (Export->objectName().isEmpty())
            Export->setObjectName(QStringLiteral("Export"));
        Export->resize(342, 198);
        csvBtn = new QPushButton(Export);
        csvBtn->setObjectName(QStringLiteral("csvBtn"));
        csvBtn->setGeometry(QRect(20, 80, 75, 31));
        nwlBtn = new QPushButton(Export);
        nwlBtn->setObjectName(QStringLiteral("nwlBtn"));
        nwlBtn->setGeometry(QRect(20, 120, 75, 31));
        groupBox = new QGroupBox(Export);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 251, 61));
        zRiBtn = new QPushButton(groupBox);
        zRiBtn->setObjectName(QStringLiteral("zRiBtn"));
        zRiBtn->setGeometry(QRect(10, 20, 75, 31));
        sRiBtn = new QPushButton(groupBox);
        sRiBtn->setObjectName(QStringLiteral("sRiBtn"));
        sRiBtn->setGeometry(QRect(90, 20, 75, 31));
        sMaBtn = new QPushButton(groupBox);
        sMaBtn->setObjectName(QStringLiteral("sMaBtn"));
        sMaBtn->setGeometry(QRect(170, 20, 75, 31));

        retranslateUi(Export);

        QMetaObject::connectSlotsByName(Export);
    } // setupUi

    void retranslateUi(QDialog *Export)
    {
        Export->setWindowTitle(QApplication::translate("Export", "Dialog", 0));
        csvBtn->setText(QApplication::translate("Export", "CSV", 0));
        nwlBtn->setText(QApplication::translate("Export", "NWL", 0));
        groupBox->setTitle(QApplication::translate("Export", "Touchstone", 0));
        zRiBtn->setText(QApplication::translate("Export", "Z, RI", 0));
        sRiBtn->setText(QApplication::translate("Export", "S, RI", 0));
        sMaBtn->setText(QApplication::translate("Export", "S, MA", 0));
    } // retranslateUi

};

namespace Ui {
    class Export: public Ui_Export {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXPORT_H

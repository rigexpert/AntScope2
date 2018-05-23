/********************************************************************************
** Form generated from reading UI file 'fqsettings.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FQSETTINGS_H
#define UI_FQSETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_FqSettings
{
public:
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QSlider *dotsSlider;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *dotsLine;
    QLabel *label_3;
    QPushButton *OKBtn;

    void setupUi(QDialog *FqSettings)
    {
        if (FqSettings->objectName().isEmpty())
            FqSettings->setObjectName(QStringLiteral("FqSettings"));
        FqSettings->resize(462, 131);
        horizontalLayout = new QHBoxLayout(FqSettings);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        groupBox = new QGroupBox(FqSettings);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        dotsSlider = new QSlider(groupBox);
        dotsSlider->setObjectName(QStringLiteral("dotsSlider"));
        dotsSlider->setGeometry(QRect(50, 20, 321, 19));
        dotsSlider->setMinimum(5);
        dotsSlider->setMaximum(1000);
        dotsSlider->setSingleStep(3);
        dotsSlider->setPageStep(100);
        dotsSlider->setValue(100);
        dotsSlider->setOrientation(Qt::Horizontal);
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 20, 31, 16));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(380, 20, 51, 16));
        dotsLine = new QLineEdit(groupBox);
        dotsLine->setObjectName(QStringLiteral("dotsLine"));
        dotsLine->setGeometry(QRect(320, 50, 113, 20));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(240, 50, 71, 16));
        OKBtn = new QPushButton(groupBox);
        OKBtn->setObjectName(QStringLiteral("OKBtn"));
        OKBtn->setGeometry(QRect(360, 80, 75, 23));

        horizontalLayout->addWidget(groupBox);


        retranslateUi(FqSettings);

        QMetaObject::connectSlotsByName(FqSettings);
    } // setupUi

    void retranslateUi(QDialog *FqSettings)
    {
        FqSettings->setWindowTitle(QApplication::translate("FqSettings", "Dialog", 0));
        groupBox->setTitle(QApplication::translate("FqSettings", "Measurement speed", 0));
        label->setText(QApplication::translate("FqSettings", "Fast", 0));
        label_2->setText(QApplication::translate("FqSettings", "Accurately ", 0));
        label_3->setText(QApplication::translate("FqSettings", "Dots number:", 0));
        OKBtn->setText(QApplication::translate("FqSettings", "OK", 0));
    } // retranslateUi

};

namespace Ui {
    class FqSettings: public Ui_FqSettings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FQSETTINGS_H

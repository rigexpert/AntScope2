/********************************************************************************
** Form generated from reading UI file 'print.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRINT_H
#define UI_PRINT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_Print
{
public:
    QHBoxLayout *horizontalLayout_2;
    QGroupBox *groupBoxGraph;
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *verticalLayout_4;
    QLabel *lineEditHead;
    QCustomPlot *widgetGraph;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_2;
    QWidget *markersWidget;
    QHBoxLayout *horizontalLayout_3;
    QGridLayout *markersLayout;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QTextEdit *textEditComment;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBoxSettings;
    QLabel *label_3;
    QSlider *lineSlider;
    QHBoxLayout *horizontalLayout;
    QPushButton *pngPrintBtn;
    QPushButton *pdfPrintBtn;
    QPushButton *printBtn;

    void setupUi(QDialog *Print)
    {
        if (Print->objectName().isEmpty())
            Print->setObjectName(QStringLiteral("Print"));
        Print->resize(910, 650);
        horizontalLayout_2 = new QHBoxLayout(Print);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        groupBoxGraph = new QGroupBox(Print);
        groupBoxGraph->setObjectName(QStringLiteral("groupBoxGraph"));
        groupBoxGraph->setStyleSheet(QStringLiteral("background-color: white;"));
        verticalLayout_5 = new QVBoxLayout(groupBoxGraph);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        lineEditHead = new QLabel(groupBoxGraph);
        lineEditHead->setObjectName(QStringLiteral("lineEditHead"));

        verticalLayout_4->addWidget(lineEditHead);

        widgetGraph = new QCustomPlot(groupBoxGraph);
        widgetGraph->setObjectName(QStringLiteral("widgetGraph"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(widgetGraph->sizePolicy().hasHeightForWidth());
        widgetGraph->setSizePolicy(sizePolicy);
        widgetGraph->setMinimumSize(QSize(500, 200));

        verticalLayout_4->addWidget(widgetGraph);


        verticalLayout_5->addLayout(verticalLayout_4);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        label_2 = new QLabel(groupBoxGraph);
        label_2->setObjectName(QStringLiteral("label_2"));

        verticalLayout_2->addWidget(label_2);

        markersWidget = new QWidget(groupBoxGraph);
        markersWidget->setObjectName(QStringLiteral("markersWidget"));
        horizontalLayout_3 = new QHBoxLayout(markersWidget);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        markersLayout = new QGridLayout();
        markersLayout->setObjectName(QStringLiteral("markersLayout"));

        horizontalLayout_3->addLayout(markersLayout);


        verticalLayout_2->addWidget(markersWidget);


        verticalLayout_5->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(groupBoxGraph);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(label);

        textEditComment = new QTextEdit(groupBoxGraph);
        textEditComment->setObjectName(QStringLiteral("textEditComment"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(textEditComment->sizePolicy().hasHeightForWidth());
        textEditComment->setSizePolicy(sizePolicy2);
        textEditComment->setMinimumSize(QSize(500, 50));
        textEditComment->setMaximumSize(QSize(16777215, 175));

        verticalLayout->addWidget(textEditComment);


        verticalLayout_5->addLayout(verticalLayout);


        horizontalLayout_2->addWidget(groupBoxGraph);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        groupBoxSettings = new QGroupBox(Print);
        groupBoxSettings->setObjectName(QStringLiteral("groupBoxSettings"));
        groupBoxSettings->setMinimumSize(QSize(225, 0));
        label_3 = new QLabel(groupBoxSettings);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(10, 20, 71, 16));
        lineSlider = new QSlider(groupBoxSettings);
        lineSlider->setObjectName(QStringLiteral("lineSlider"));
        lineSlider->setGeometry(QRect(69, 20, 151, 20));
        lineSlider->setMinimum(1);
        lineSlider->setMaximum(10);
        lineSlider->setPageStep(1);
        lineSlider->setSliderPosition(3);
        lineSlider->setOrientation(Qt::Horizontal);

        verticalLayout_3->addWidget(groupBoxSettings);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        pngPrintBtn = new QPushButton(Print);
        pngPrintBtn->setObjectName(QStringLiteral("pngPrintBtn"));
        pngPrintBtn->setMinimumSize(QSize(75, 75));
        pngPrintBtn->setMaximumSize(QSize(75, 75));

        horizontalLayout->addWidget(pngPrintBtn);

        pdfPrintBtn = new QPushButton(Print);
        pdfPrintBtn->setObjectName(QStringLiteral("pdfPrintBtn"));
        pdfPrintBtn->setMinimumSize(QSize(75, 75));
        pdfPrintBtn->setMaximumSize(QSize(75, 75));

        horizontalLayout->addWidget(pdfPrintBtn);

        printBtn = new QPushButton(Print);
        printBtn->setObjectName(QStringLiteral("printBtn"));
        printBtn->setMinimumSize(QSize(75, 75));
        printBtn->setMaximumSize(QSize(75, 75));

        horizontalLayout->addWidget(printBtn);


        verticalLayout_3->addLayout(horizontalLayout);


        horizontalLayout_2->addLayout(verticalLayout_3);


        retranslateUi(Print);

        QMetaObject::connectSlotsByName(Print);
    } // setupUi

    void retranslateUi(QDialog *Print)
    {
        Print->setWindowTitle(QApplication::translate("Print", "Print", 0));
        groupBoxGraph->setTitle(QApplication::translate("Print", "Preview", 0));
        lineEditHead->setText(QApplication::translate("Print", "TextLabel", 0));
        label_2->setText(QApplication::translate("Print", "Markers:", 0));
        label->setText(QApplication::translate("Print", "Comment:", 0));
        groupBoxSettings->setTitle(QApplication::translate("Print", "Settings", 0));
        label_3->setText(QApplication::translate("Print", "Line width", 0));
        pngPrintBtn->setText(QApplication::translate("Print", "Save as .png", 0));
        pdfPrintBtn->setText(QApplication::translate("Print", "Save as .pdf", 0));
        printBtn->setText(QApplication::translate("Print", "Print", 0));
    } // retranslateUi

};

namespace Ui {
    class Print: public Ui_Print {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRINT_H

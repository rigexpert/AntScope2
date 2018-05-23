/********************************************************************************
** Form generated from reading UI file 'analyzerdata.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ANALYZERDATA_H
#define UI_ANALYZERDATA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AnalyzerData
{
public:
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QListWidget *listWidget;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AnalyzerData)
    {
        if (AnalyzerData->objectName().isEmpty())
            AnalyzerData->setObjectName(QStringLiteral("AnalyzerData"));
        AnalyzerData->resize(400, 300);
        horizontalLayout = new QHBoxLayout(AnalyzerData);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        listWidget = new QListWidget(AnalyzerData);
        listWidget->setObjectName(QStringLiteral("listWidget"));

        verticalLayout->addWidget(listWidget);

        buttonBox = new QDialogButtonBox(AnalyzerData);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        horizontalLayout->addLayout(verticalLayout);


        retranslateUi(AnalyzerData);
        QObject::connect(buttonBox, SIGNAL(accepted()), AnalyzerData, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AnalyzerData, SLOT(reject()));

        QMetaObject::connectSlotsByName(AnalyzerData);
    } // setupUi

    void retranslateUi(QDialog *AnalyzerData)
    {
        AnalyzerData->setWindowTitle(QApplication::translate("AnalyzerData", "Dialog", 0));
    } // retranslateUi

};

namespace Ui {
    class AnalyzerData: public Ui_AnalyzerData {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ANALYZERDATA_H

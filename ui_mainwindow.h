/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_5;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *settingsBtn;
    QPushButton *exportBtn;
    QPushButton *importBtn;
    QPushButton *printBtn;
    QPushButton *screenshot;
    QPushButton *screenshotAA;
    QPushButton *analyzerDataBtn;
    QVBoxLayout *verticalLayout_10;
    QLabel *logo_label;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_9;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_8;
    QPushButton *limitsBtn;
    QPushButton *rangeBtn;
    QHBoxLayout *horizontalLayout_2;
    QLabel *startLabel;
    QLineEdit *lineEdit_fqFrom;
    QHBoxLayout *horizontalLayout_4;
    QLabel *stopLabel;
    QLineEdit *lineEdit_fqTo;
    QHBoxLayout *horizontalLayout;
    QPushButton *fqSettingsBtn;
    QSpacerItem *horizontalSpacer;
    QPushButton *continuousStartBtn;
    QPushButton *singleStart;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_6;
    QVBoxLayout *verticalLayout_3;
    QTableWidget *tableWidget_presets;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *presetsAddBtn;
    QPushButton *presetsDeleteBtn;
    QPushButton *pressetsUpBtn;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *verticalLayout_5;
    QTableWidget *tableWidget_measurments;
    QHBoxLayout *horizontalLayout_7;
    QPushButton *measurementsOpenBtn;
    QPushButton *measurmentsSaveBtn;
    QPushButton *measurmentsDeleteBtn;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1293, 700);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(700, 700));
        MainWindow->setMaximumSize(QSize(6000, 3000));
        MainWindow->setWindowTitle(QStringLiteral("AntScope2"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        centralWidget->setStyleSheet(QStringLiteral(""));
        horizontalLayout_5 = new QHBoxLayout(centralWidget);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(4);
        sizePolicy1.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy1);
        tabWidget->setMouseTracking(true);
        tabWidget->setLayoutDirection(Qt::LeftToRight);
        tabWidget->setStyleSheet(QStringLiteral(""));
        tabWidget->setTabPosition(QTabWidget::North);
        tabWidget->setTabShape(QTabWidget::Triangular);
        tabWidget->setDocumentMode(false);
        tabWidget->setTabsClosable(false);
        tabWidget->setMovable(true);

        verticalLayout->addWidget(tabWidget);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        settingsBtn = new QPushButton(centralWidget);
        settingsBtn->setObjectName(QStringLiteral("settingsBtn"));
        sizePolicy.setHeightForWidth(settingsBtn->sizePolicy().hasHeightForWidth());
        settingsBtn->setSizePolicy(sizePolicy);
        settingsBtn->setStyleSheet(QStringLiteral(""));

        horizontalLayout_3->addWidget(settingsBtn);

        exportBtn = new QPushButton(centralWidget);
        exportBtn->setObjectName(QStringLiteral("exportBtn"));
        sizePolicy.setHeightForWidth(exportBtn->sizePolicy().hasHeightForWidth());
        exportBtn->setSizePolicy(sizePolicy);
        exportBtn->setStyleSheet(QStringLiteral(""));

        horizontalLayout_3->addWidget(exportBtn);

        importBtn = new QPushButton(centralWidget);
        importBtn->setObjectName(QStringLiteral("importBtn"));
        sizePolicy.setHeightForWidth(importBtn->sizePolicy().hasHeightForWidth());
        importBtn->setSizePolicy(sizePolicy);

        horizontalLayout_3->addWidget(importBtn);

        printBtn = new QPushButton(centralWidget);
        printBtn->setObjectName(QStringLiteral("printBtn"));
        sizePolicy.setHeightForWidth(printBtn->sizePolicy().hasHeightForWidth());
        printBtn->setSizePolicy(sizePolicy);
        printBtn->setStyleSheet(QStringLiteral(""));

        horizontalLayout_3->addWidget(printBtn);

        screenshot = new QPushButton(centralWidget);
        screenshot->setObjectName(QStringLiteral("screenshot"));
        sizePolicy.setHeightForWidth(screenshot->sizePolicy().hasHeightForWidth());
        screenshot->setSizePolicy(sizePolicy);
        screenshot->setStyleSheet(QStringLiteral(""));

        horizontalLayout_3->addWidget(screenshot);

        screenshotAA = new QPushButton(centralWidget);
        screenshotAA->setObjectName(QStringLiteral("screenshotAA"));
        sizePolicy.setHeightForWidth(screenshotAA->sizePolicy().hasHeightForWidth());
        screenshotAA->setSizePolicy(sizePolicy);
        screenshotAA->setStyleSheet(QStringLiteral(""));

        horizontalLayout_3->addWidget(screenshotAA);

        analyzerDataBtn = new QPushButton(centralWidget);
        analyzerDataBtn->setObjectName(QStringLiteral("analyzerDataBtn"));
        sizePolicy.setHeightForWidth(analyzerDataBtn->sizePolicy().hasHeightForWidth());
        analyzerDataBtn->setSizePolicy(sizePolicy);
        analyzerDataBtn->setStyleSheet(QStringLiteral(""));

        horizontalLayout_3->addWidget(analyzerDataBtn);


        verticalLayout->addLayout(horizontalLayout_3);


        horizontalLayout_5->addLayout(verticalLayout);

        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setSpacing(6);
        verticalLayout_10->setObjectName(QStringLiteral("verticalLayout_10"));
        logo_label = new QLabel(centralWidget);
        logo_label->setObjectName(QStringLiteral("logo_label"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Maximum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(logo_label->sizePolicy().hasHeightForWidth());
        logo_label->setSizePolicy(sizePolicy2);
        logo_label->setMinimumSize(QSize(270, 50));
        logo_label->setMaximumSize(QSize(270, 50));
        logo_label->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/rig_logo.png")));
        logo_label->setScaledContents(true);
        logo_label->setIndent(0);

        verticalLayout_10->addWidget(logo_label);

        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        QSizePolicy sizePolicy3(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy3);
        groupBox->setMinimumSize(QSize(230, 165));
        groupBox->setMaximumSize(QSize(270, 150));
        horizontalLayout_9 = new QHBoxLayout(groupBox);
        horizontalLayout_9->setSpacing(6);
        horizontalLayout_9->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        limitsBtn = new QPushButton(groupBox);
        limitsBtn->setObjectName(QStringLiteral("limitsBtn"));
        limitsBtn->setStyleSheet(QLatin1String("limitsBtn:pressed\n"
"{\n"
"	background-color: rgb(1,178,255);\n"
"}"));
        limitsBtn->setCheckable(true);
        limitsBtn->setChecked(true);

        horizontalLayout_8->addWidget(limitsBtn);

        rangeBtn = new QPushButton(groupBox);
        rangeBtn->setObjectName(QStringLiteral("rangeBtn"));
        rangeBtn->setCheckable(true);

        horizontalLayout_8->addWidget(rangeBtn);


        verticalLayout_2->addLayout(horizontalLayout_8);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        startLabel = new QLabel(groupBox);
        startLabel->setObjectName(QStringLiteral("startLabel"));

        horizontalLayout_2->addWidget(startLabel);

        lineEdit_fqFrom = new QLineEdit(groupBox);
        lineEdit_fqFrom->setObjectName(QStringLiteral("lineEdit_fqFrom"));
        QFont font;
        font.setPointSize(10);
        lineEdit_fqFrom->setFont(font);
        lineEdit_fqFrom->setText(QStringLiteral(""));
        lineEdit_fqFrom->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(lineEdit_fqFrom);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        stopLabel = new QLabel(groupBox);
        stopLabel->setObjectName(QStringLiteral("stopLabel"));

        horizontalLayout_4->addWidget(stopLabel);

        lineEdit_fqTo = new QLineEdit(groupBox);
        lineEdit_fqTo->setObjectName(QStringLiteral("lineEdit_fqTo"));
        lineEdit_fqTo->setFont(font);
        lineEdit_fqTo->setText(QStringLiteral(""));
        lineEdit_fqTo->setFrame(true);
        lineEdit_fqTo->setAlignment(Qt::AlignCenter);
        lineEdit_fqTo->setClearButtonEnabled(false);

        horizontalLayout_4->addWidget(lineEdit_fqTo);


        verticalLayout_2->addLayout(horizontalLayout_4);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        fqSettingsBtn = new QPushButton(groupBox);
        fqSettingsBtn->setObjectName(QStringLiteral("fqSettingsBtn"));
        QSizePolicy sizePolicy4(QSizePolicy::Maximum, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(1);
        sizePolicy4.setVerticalStretch(1);
        sizePolicy4.setHeightForWidth(fqSettingsBtn->sizePolicy().hasHeightForWidth());
        fqSettingsBtn->setSizePolicy(sizePolicy4);
        fqSettingsBtn->setMaximumSize(QSize(20, 20));

        horizontalLayout->addWidget(fqSettingsBtn);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        continuousStartBtn = new QPushButton(groupBox);
        continuousStartBtn->setObjectName(QStringLiteral("continuousStartBtn"));
        QSizePolicy sizePolicy5(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy5.setHorizontalStretch(2);
        sizePolicy5.setVerticalStretch(2);
        sizePolicy5.setHeightForWidth(continuousStartBtn->sizePolicy().hasHeightForWidth());
        continuousStartBtn->setSizePolicy(sizePolicy5);
        continuousStartBtn->setMinimumSize(QSize(65, 0));
        continuousStartBtn->setMaximumSize(QSize(16777215, 20));
        continuousStartBtn->setCheckable(true);

        horizontalLayout->addWidget(continuousStartBtn);

        singleStart = new QPushButton(groupBox);
        singleStart->setObjectName(QStringLiteral("singleStart"));
        sizePolicy5.setHeightForWidth(singleStart->sizePolicy().hasHeightForWidth());
        singleStart->setSizePolicy(sizePolicy5);
        singleStart->setMaximumSize(QSize(16777215, 20));

        horizontalLayout->addWidget(singleStart);


        verticalLayout_2->addLayout(horizontalLayout);


        horizontalLayout_9->addLayout(verticalLayout_2);


        verticalLayout_10->addWidget(groupBox);

        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        sizePolicy3.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy3);
        groupBox_2->setMinimumSize(QSize(230, 201));
        groupBox_2->setMaximumSize(QSize(270, 16777215));
        verticalLayout_6 = new QVBoxLayout(groupBox_2);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        tableWidget_presets = new QTableWidget(groupBox_2);
        if (tableWidget_presets->columnCount() < 2)
            tableWidget_presets->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget_presets->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget_presets->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        tableWidget_presets->setObjectName(QStringLiteral("tableWidget_presets"));
        QSizePolicy sizePolicy6(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(tableWidget_presets->sizePolicy().hasHeightForWidth());
        tableWidget_presets->setSizePolicy(sizePolicy6);
        tableWidget_presets->setMaximumSize(QSize(2000, 1000));
        tableWidget_presets->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget_presets->setDragEnabled(true);
        tableWidget_presets->setDragDropOverwriteMode(true);
        tableWidget_presets->setDragDropMode(QAbstractItemView::DropOnly);
        tableWidget_presets->setDefaultDropAction(Qt::MoveAction);
        tableWidget_presets->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidget_presets->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableWidget_presets->setRowCount(0);
        tableWidget_presets->setColumnCount(2);
        tableWidget_presets->horizontalHeader()->setVisible(false);
        tableWidget_presets->horizontalHeader()->setCascadingSectionResizes(false);
        tableWidget_presets->horizontalHeader()->setDefaultSectionSize(120);
        tableWidget_presets->horizontalHeader()->setMinimumSectionSize(85);
        tableWidget_presets->verticalHeader()->setVisible(false);
        tableWidget_presets->verticalHeader()->setMinimumSectionSize(30);

        verticalLayout_3->addWidget(tableWidget_presets);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        presetsAddBtn = new QPushButton(groupBox_2);
        presetsAddBtn->setObjectName(QStringLiteral("presetsAddBtn"));

        horizontalLayout_6->addWidget(presetsAddBtn);

        presetsDeleteBtn = new QPushButton(groupBox_2);
        presetsDeleteBtn->setObjectName(QStringLiteral("presetsDeleteBtn"));

        horizontalLayout_6->addWidget(presetsDeleteBtn);

        pressetsUpBtn = new QPushButton(groupBox_2);
        pressetsUpBtn->setObjectName(QStringLiteral("pressetsUpBtn"));

        horizontalLayout_6->addWidget(pressetsUpBtn);


        verticalLayout_3->addLayout(horizontalLayout_6);


        verticalLayout_6->addLayout(verticalLayout_3);


        verticalLayout_10->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(centralWidget);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        sizePolicy3.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy3);
        groupBox_3->setMinimumSize(QSize(230, 250));
        groupBox_3->setMaximumSize(QSize(270, 16777215));
        verticalLayout_4 = new QVBoxLayout(groupBox_3);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        tableWidget_measurments = new QTableWidget(groupBox_3);
        if (tableWidget_measurments->columnCount() < 1)
            tableWidget_measurments->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget_measurments->setHorizontalHeaderItem(0, __qtablewidgetitem2);
        tableWidget_measurments->setObjectName(QStringLiteral("tableWidget_measurments"));
        QSizePolicy sizePolicy7(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(1);
        sizePolicy7.setHeightForWidth(tableWidget_measurments->sizePolicy().hasHeightForWidth());
        tableWidget_measurments->setSizePolicy(sizePolicy7);
        tableWidget_measurments->setMaximumSize(QSize(2000, 1000));
        tableWidget_measurments->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableWidget_measurments->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tableWidget_measurments->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
        tableWidget_measurments->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget_measurments->setDragEnabled(false);
        tableWidget_measurments->setDragDropOverwriteMode(true);
        tableWidget_measurments->setDragDropMode(QAbstractItemView::NoDragDrop);
        tableWidget_measurments->setDefaultDropAction(Qt::MoveAction);
        tableWidget_measurments->setAlternatingRowColors(true);
        tableWidget_measurments->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidget_measurments->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
        tableWidget_measurments->setShowGrid(true);
        tableWidget_measurments->setGridStyle(Qt::DotLine);
        tableWidget_measurments->setSortingEnabled(false);
        tableWidget_measurments->setWordWrap(true);
        tableWidget_measurments->setCornerButtonEnabled(true);
        tableWidget_measurments->setRowCount(0);
        tableWidget_measurments->setColumnCount(1);
        tableWidget_measurments->horizontalHeader()->setCascadingSectionResizes(true);
        tableWidget_measurments->horizontalHeader()->setDefaultSectionSize(245);
        tableWidget_measurments->horizontalHeader()->setHighlightSections(true);
        tableWidget_measurments->horizontalHeader()->setMinimumSectionSize(175);
        tableWidget_measurments->verticalHeader()->setVisible(false);

        verticalLayout_5->addWidget(tableWidget_measurments);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        measurementsOpenBtn = new QPushButton(groupBox_3);
        measurementsOpenBtn->setObjectName(QStringLiteral("measurementsOpenBtn"));
        sizePolicy.setHeightForWidth(measurementsOpenBtn->sizePolicy().hasHeightForWidth());
        measurementsOpenBtn->setSizePolicy(sizePolicy);

        horizontalLayout_7->addWidget(measurementsOpenBtn);

        measurmentsSaveBtn = new QPushButton(groupBox_3);
        measurmentsSaveBtn->setObjectName(QStringLiteral("measurmentsSaveBtn"));
        sizePolicy.setHeightForWidth(measurmentsSaveBtn->sizePolicy().hasHeightForWidth());
        measurmentsSaveBtn->setSizePolicy(sizePolicy);

        horizontalLayout_7->addWidget(measurmentsSaveBtn);

        measurmentsDeleteBtn = new QPushButton(groupBox_3);
        measurmentsDeleteBtn->setObjectName(QStringLiteral("measurmentsDeleteBtn"));
        sizePolicy.setHeightForWidth(measurmentsDeleteBtn->sizePolicy().hasHeightForWidth());
        measurmentsDeleteBtn->setSizePolicy(sizePolicy);

        horizontalLayout_7->addWidget(measurmentsDeleteBtn);


        verticalLayout_5->addLayout(horizontalLayout_7);


        verticalLayout_4->addLayout(verticalLayout_5);


        verticalLayout_10->addWidget(groupBox_3);


        horizontalLayout_5->addLayout(verticalLayout_10);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        settingsBtn->setText(QApplication::translate("MainWindow", "Settings", 0));
        exportBtn->setText(QApplication::translate("MainWindow", "Export", 0));
        importBtn->setText(QApplication::translate("MainWindow", "Import", 0));
        printBtn->setText(QApplication::translate("MainWindow", "Print", 0));
        screenshot->setText(QApplication::translate("MainWindow", "Screenshot", 0));
        screenshotAA->setText(QApplication::translate("MainWindow", "Screenshot from AA", 0));
        analyzerDataBtn->setText(QApplication::translate("MainWindow", "Data from AA", 0));
        logo_label->setText(QString());
        groupBox->setTitle(QApplication::translate("MainWindow", "Frequency(kHz)", 0));
        limitsBtn->setText(QApplication::translate("MainWindow", "Limits", 0));
        rangeBtn->setText(QApplication::translate("MainWindow", "Center, Range", 0));
        startLabel->setText(QApplication::translate("MainWindow", "Start", 0));
        stopLabel->setText(QApplication::translate("MainWindow", "Stop", 0));
        fqSettingsBtn->setText(QApplication::translate("MainWindow", "S", 0));
        continuousStartBtn->setText(QApplication::translate("MainWindow", "Continuous", 0));
        singleStart->setText(QApplication::translate("MainWindow", "Single", 0));
        groupBox_2->setTitle(QApplication::translate("MainWindow", "Presets(kHz)", 0));
        QTableWidgetItem *___qtablewidgetitem = tableWidget_presets->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("MainWindow", "Start", 0));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget_presets->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("MainWindow", "Stop", 0));
        presetsAddBtn->setText(QApplication::translate("MainWindow", "Add", 0));
        presetsDeleteBtn->setText(QApplication::translate("MainWindow", "Delete", 0));
        pressetsUpBtn->setText(QApplication::translate("MainWindow", "Move up", 0));
        groupBox_3->setTitle(QApplication::translate("MainWindow", "Measurements", 0));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget_measurments->horizontalHeaderItem(0);
        ___qtablewidgetitem2->setText(QApplication::translate("MainWindow", "Name", 0));
        measurementsOpenBtn->setText(QApplication::translate("MainWindow", "Open", 0));
        measurmentsSaveBtn->setText(QApplication::translate("MainWindow", "Save", 0));
        measurmentsDeleteBtn->setText(QApplication::translate("MainWindow", "Delete", 0));
        Q_UNUSED(MainWindow);
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

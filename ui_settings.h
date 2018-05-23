/********************************************************************************
** Form generated from reading UI file 'settings.ui'
**
** Created by: Qt User Interface Compiler version 5.4.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Settings
{
public:
    QHBoxLayout *horizontalLayout;
    QTabWidget *tabWidget;
    QWidget *General;
    QCheckBox *graphHintCheckBox;
    QCheckBox *markersHintCheckBox;
    QGroupBox *groupBox_7;
    QCheckBox *measureSystemMetric;
    QCheckBox *measureSystemAmerican;
    QLineEdit *lineEdit_systemImpedance;
    QLabel *label_4;
    QLabel *label_6;
    QCheckBox *graphBriefHintCheckBox;
    QGroupBox *groupBox_12;
    QComboBox *serialPortComboBox;
    QLabel *label_15;
    QCheckBox *autoDetect;
    QCheckBox *manualDetect;
    QLabel *languageLabel;
    QComboBox *languageComboBox;
    QWidget *Updates;
    QVBoxLayout *verticalLayout_4;
    QGroupBox *groupBox_10;
    QHBoxLayout *horizontalLayout_11;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_9;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_5;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLabel *analyzerModelLabel;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QLabel *versionLabel;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_5;
    QLabel *serialLabel;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_8;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_6;
    QLineEdit *browseLine;
    QPushButton *browseBtn;
    QHBoxLayout *horizontalLayout_7;
    QProgressBar *updateProgressBar;
    QPushButton *updateBtn;
    QHBoxLayout *horizontalLayout_10;
    QCheckBox *autoUpdatesCheckBox;
    QPushButton *checkUpdatesBtn;
    QGroupBox *groupBox_11;
    QHBoxLayout *horizontalLayout_15;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_14;
    QLabel *label_16;
    QLabel *antScopeVersion;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_17;
    QCheckBox *checkBox_AntScopeAutoUpdate;
    QWidget *Calibration;
    QGroupBox *groupBox_3;
    QPushButton *openCalibBtn;
    QLabel *labelOpenState;
    QPushButton *openOpenFileBtn;
    QProgressBar *openProgressBar;
    QGroupBox *groupBox_4;
    QPushButton *shortCalibBtn;
    QPushButton *shortOpenFileBtn;
    QLabel *labelShortState;
    QProgressBar *shortProgressBar;
    QGroupBox *groupBox_5;
    QPushButton *loadCalibBtn;
    QPushButton *loadOpenFileBtn;
    QLabel *labelLoadState;
    QProgressBar *loadProgressBar;
    QGroupBox *groupBox_6;
    QPushButton *calibWizard;
    QLabel *labelWizardStatus;
    QLabel *label_3;
    QPushButton *turnOnOffBtn;
    QWidget *Cable;
    QLabel *label_7;
    QLineEdit *velocityFactor;
    QLabel *label_8;
    QLineEdit *cableR0;
    QLabel *label_9;
    QGroupBox *groupBox_8;
    QLabel *label_10;
    QLineEdit *conductiveLoss;
    QLabel *label_11;
    QLineEdit *dielectricLoss;
    QComboBox *cableLossComboBox;
    QLabel *label_12;
    QRadioButton *anyFq;
    QRadioButton *atFq;
    QLineEdit *atMHz;
    QLabel *label_13;
    QComboBox *cableComboBox;
    QLabel *label_14;
    QLineEdit *cableLen;
    QGroupBox *groupBox_9;
    QPushButton *doNothingBtn;
    QPushButton *subtractCableBtn;
    QPushButton *addCableBtn;
    QPushButton *updateGraphsBtn;

    void setupUi(QDialog *Settings)
    {
        if (Settings->objectName().isEmpty())
            Settings->setObjectName(QStringLiteral("Settings"));
        Settings->resize(550, 320);
        Settings->setMinimumSize(QSize(550, 320));
        Settings->setMaximumSize(QSize(550, 640));
        horizontalLayout = new QHBoxLayout(Settings);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        tabWidget = new QTabWidget(Settings);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy);
        General = new QWidget();
        General->setObjectName(QStringLiteral("General"));
        graphHintCheckBox = new QCheckBox(General);
        graphHintCheckBox->setObjectName(QStringLiteral("graphHintCheckBox"));
        graphHintCheckBox->setGeometry(QRect(280, 10, 241, 17));
        graphHintCheckBox->setChecked(true);
        markersHintCheckBox = new QCheckBox(General);
        markersHintCheckBox->setObjectName(QStringLiteral("markersHintCheckBox"));
        markersHintCheckBox->setGeometry(QRect(280, 30, 241, 17));
        markersHintCheckBox->setChecked(true);
        groupBox_7 = new QGroupBox(General);
        groupBox_7->setObjectName(QStringLiteral("groupBox_7"));
        groupBox_7->setGeometry(QRect(10, 10, 151, 71));
        measureSystemMetric = new QCheckBox(groupBox_7);
        measureSystemMetric->setObjectName(QStringLiteral("measureSystemMetric"));
        measureSystemMetric->setGeometry(QRect(10, 20, 131, 21));
        measureSystemAmerican = new QCheckBox(groupBox_7);
        measureSystemAmerican->setObjectName(QStringLiteral("measureSystemAmerican"));
        measureSystemAmerican->setGeometry(QRect(10, 40, 131, 20));
        lineEdit_systemImpedance = new QLineEdit(General);
        lineEdit_systemImpedance->setObjectName(QStringLiteral("lineEdit_systemImpedance"));
        lineEdit_systemImpedance->setGeometry(QRect(280, 100, 61, 20));
        lineEdit_systemImpedance->setMaxLength(4);
        label_4 = new QLabel(General);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(280, 80, 181, 16));
        label_6 = new QLabel(General);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(350, 100, 51, 21));
        graphBriefHintCheckBox = new QCheckBox(General);
        graphBriefHintCheckBox->setObjectName(QStringLiteral("graphBriefHintCheckBox"));
        graphBriefHintCheckBox->setGeometry(QRect(280, 50, 241, 17));
        graphBriefHintCheckBox->setChecked(true);
        groupBox_12 = new QGroupBox(General);
        groupBox_12->setObjectName(QStringLiteral("groupBox_12"));
        groupBox_12->setGeometry(QRect(10, 90, 151, 111));
        serialPortComboBox = new QComboBox(groupBox_12);
        serialPortComboBox->setObjectName(QStringLiteral("serialPortComboBox"));
        serialPortComboBox->setGeometry(QRect(30, 80, 111, 22));
        label_15 = new QLabel(groupBox_12);
        label_15->setObjectName(QStringLiteral("label_15"));
        label_15->setGeometry(QRect(30, 60, 111, 16));
        autoDetect = new QCheckBox(groupBox_12);
        autoDetect->setObjectName(QStringLiteral("autoDetect"));
        autoDetect->setGeometry(QRect(10, 20, 131, 17));
        manualDetect = new QCheckBox(groupBox_12);
        manualDetect->setObjectName(QStringLiteral("manualDetect"));
        manualDetect->setGeometry(QRect(10, 40, 131, 17));
        languageLabel = new QLabel(General);
        languageLabel->setObjectName(QStringLiteral("languageLabel"));
        languageLabel->setGeometry(QRect(10, 210, 151, 16));
        languageComboBox = new QComboBox(General);
        languageComboBox->setObjectName(QStringLiteral("languageComboBox"));
        languageComboBox->setGeometry(QRect(10, 230, 151, 22));
        tabWidget->addTab(General, QString());
        Updates = new QWidget();
        Updates->setObjectName(QStringLiteral("Updates"));
        verticalLayout_4 = new QVBoxLayout(Updates);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        groupBox_10 = new QGroupBox(Updates);
        groupBox_10->setObjectName(QStringLiteral("groupBox_10"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox_10->sizePolicy().hasHeightForWidth());
        groupBox_10->setSizePolicy(sizePolicy1);
        horizontalLayout_11 = new QHBoxLayout(groupBox_10);
        horizontalLayout_11->setObjectName(QStringLiteral("horizontalLayout_11"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        groupBox = new QGroupBox(groupBox_10);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        horizontalLayout_5 = new QHBoxLayout(groupBox);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout_2->addWidget(label);

        analyzerModelLabel = new QLabel(groupBox);
        analyzerModelLabel->setObjectName(QStringLiteral("analyzerModelLabel"));
        analyzerModelLabel->setText(QStringLiteral("none"));
        analyzerModelLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(analyzerModelLabel);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout_3->addWidget(label_2);

        versionLabel = new QLabel(groupBox);
        versionLabel->setObjectName(QStringLiteral("versionLabel"));
        versionLabel->setText(QStringLiteral("0"));
        versionLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(versionLabel);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        horizontalLayout_4->addWidget(label_5);

        serialLabel = new QLabel(groupBox);
        serialLabel->setObjectName(QStringLiteral("serialLabel"));
        serialLabel->setText(QStringLiteral("0"));
        serialLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(serialLabel);


        verticalLayout->addLayout(horizontalLayout_4);


        horizontalLayout_5->addLayout(verticalLayout);


        horizontalLayout_9->addWidget(groupBox);

        groupBox_2 = new QGroupBox(groupBox_10);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        horizontalLayout_8 = new QHBoxLayout(groupBox_2);
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        browseLine = new QLineEdit(groupBox_2);
        browseLine->setObjectName(QStringLiteral("browseLine"));

        horizontalLayout_6->addWidget(browseLine);

        browseBtn = new QPushButton(groupBox_2);
        browseBtn->setObjectName(QStringLiteral("browseBtn"));

        horizontalLayout_6->addWidget(browseBtn);


        verticalLayout_2->addLayout(horizontalLayout_6);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        updateProgressBar = new QProgressBar(groupBox_2);
        updateProgressBar->setObjectName(QStringLiteral("updateProgressBar"));
        updateProgressBar->setValue(0);

        horizontalLayout_7->addWidget(updateProgressBar);

        updateBtn = new QPushButton(groupBox_2);
        updateBtn->setObjectName(QStringLiteral("updateBtn"));
        updateBtn->setEnabled(false);

        horizontalLayout_7->addWidget(updateBtn);


        verticalLayout_2->addLayout(horizontalLayout_7);


        horizontalLayout_8->addLayout(verticalLayout_2);


        horizontalLayout_9->addWidget(groupBox_2);


        verticalLayout_3->addLayout(horizontalLayout_9);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QStringLiteral("horizontalLayout_10"));
        autoUpdatesCheckBox = new QCheckBox(groupBox_10);
        autoUpdatesCheckBox->setObjectName(QStringLiteral("autoUpdatesCheckBox"));
        autoUpdatesCheckBox->setEnabled(false);

        horizontalLayout_10->addWidget(autoUpdatesCheckBox);

        checkUpdatesBtn = new QPushButton(groupBox_10);
        checkUpdatesBtn->setObjectName(QStringLiteral("checkUpdatesBtn"));

        horizontalLayout_10->addWidget(checkUpdatesBtn);


        verticalLayout_3->addLayout(horizontalLayout_10);


        horizontalLayout_11->addLayout(verticalLayout_3);


        verticalLayout_4->addWidget(groupBox_10);

        groupBox_11 = new QGroupBox(Updates);
        groupBox_11->setObjectName(QStringLiteral("groupBox_11"));
        sizePolicy1.setHeightForWidth(groupBox_11->sizePolicy().hasHeightForWidth());
        groupBox_11->setSizePolicy(sizePolicy1);
        horizontalLayout_15 = new QHBoxLayout(groupBox_11);
        horizontalLayout_15->setObjectName(QStringLiteral("horizontalLayout_15"));
        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QStringLiteral("horizontalLayout_14"));
        label_16 = new QLabel(groupBox_11);
        label_16->setObjectName(QStringLiteral("label_16"));

        horizontalLayout_14->addWidget(label_16);

        antScopeVersion = new QLabel(groupBox_11);
        antScopeVersion->setObjectName(QStringLiteral("antScopeVersion"));
        antScopeVersion->setText(QStringLiteral("0"));
        antScopeVersion->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_14->addWidget(antScopeVersion);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_14->addItem(horizontalSpacer);


        verticalLayout_5->addLayout(horizontalLayout_14);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QStringLiteral("horizontalLayout_17"));
        checkBox_AntScopeAutoUpdate = new QCheckBox(groupBox_11);
        checkBox_AntScopeAutoUpdate->setObjectName(QStringLiteral("checkBox_AntScopeAutoUpdate"));

        horizontalLayout_17->addWidget(checkBox_AntScopeAutoUpdate);


        verticalLayout_5->addLayout(horizontalLayout_17);


        horizontalLayout_15->addLayout(verticalLayout_5);


        verticalLayout_4->addWidget(groupBox_11);

        tabWidget->addTab(Updates, QString());
        Calibration = new QWidget();
        Calibration->setObjectName(QStringLiteral("Calibration"));
        groupBox_3 = new QGroupBox(Calibration);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(10, 70, 511, 51));
        openCalibBtn = new QPushButton(groupBox_3);
        openCalibBtn->setObjectName(QStringLiteral("openCalibBtn"));
        openCalibBtn->setGeometry(QRect(350, 20, 151, 23));
        labelOpenState = new QLabel(groupBox_3);
        labelOpenState->setObjectName(QStringLiteral("labelOpenState"));
        labelOpenState->setGeometry(QRect(10, 20, 101, 16));
        openOpenFileBtn = new QPushButton(groupBox_3);
        openOpenFileBtn->setObjectName(QStringLiteral("openOpenFileBtn"));
        openOpenFileBtn->setGeometry(QRect(280, 20, 61, 21));
        openProgressBar = new QProgressBar(groupBox_3);
        openProgressBar->setObjectName(QStringLiteral("openProgressBar"));
        openProgressBar->setEnabled(true);
        openProgressBar->setGeometry(QRect(120, 20, 161, 23));
        openProgressBar->setValue(0);
        groupBox_4 = new QGroupBox(Calibration);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setGeometry(QRect(10, 130, 511, 51));
        shortCalibBtn = new QPushButton(groupBox_4);
        shortCalibBtn->setObjectName(QStringLiteral("shortCalibBtn"));
        shortCalibBtn->setGeometry(QRect(350, 20, 151, 23));
        shortOpenFileBtn = new QPushButton(groupBox_4);
        shortOpenFileBtn->setObjectName(QStringLiteral("shortOpenFileBtn"));
        shortOpenFileBtn->setGeometry(QRect(280, 20, 61, 21));
        labelShortState = new QLabel(groupBox_4);
        labelShortState->setObjectName(QStringLiteral("labelShortState"));
        labelShortState->setGeometry(QRect(10, 20, 101, 16));
        shortProgressBar = new QProgressBar(groupBox_4);
        shortProgressBar->setObjectName(QStringLiteral("shortProgressBar"));
        shortProgressBar->setGeometry(QRect(120, 20, 161, 23));
        shortProgressBar->setValue(0);
        groupBox_5 = new QGroupBox(Calibration);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        groupBox_5->setGeometry(QRect(10, 190, 511, 61));
        loadCalibBtn = new QPushButton(groupBox_5);
        loadCalibBtn->setObjectName(QStringLiteral("loadCalibBtn"));
        loadCalibBtn->setGeometry(QRect(350, 20, 151, 23));
        loadOpenFileBtn = new QPushButton(groupBox_5);
        loadOpenFileBtn->setObjectName(QStringLiteral("loadOpenFileBtn"));
        loadOpenFileBtn->setGeometry(QRect(280, 20, 61, 21));
        labelLoadState = new QLabel(groupBox_5);
        labelLoadState->setObjectName(QStringLiteral("labelLoadState"));
        labelLoadState->setGeometry(QRect(10, 20, 101, 16));
        loadProgressBar = new QProgressBar(groupBox_5);
        loadProgressBar->setObjectName(QStringLiteral("loadProgressBar"));
        loadProgressBar->setGeometry(QRect(120, 20, 161, 23));
        loadProgressBar->setValue(0);
        groupBox_6 = new QGroupBox(Calibration);
        groupBox_6->setObjectName(QStringLiteral("groupBox_6"));
        groupBox_6->setGeometry(QRect(10, 0, 511, 61));
        calibWizard = new QPushButton(groupBox_6);
        calibWizard->setObjectName(QStringLiteral("calibWizard"));
        calibWizard->setGeometry(QRect(360, 20, 141, 31));
        labelWizardStatus = new QLabel(groupBox_6);
        labelWizardStatus->setObjectName(QStringLiteral("labelWizardStatus"));
        labelWizardStatus->setGeometry(QRect(50, 20, 150, 20));
        label_3 = new QLabel(groupBox_6);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(10, 20, 36, 21));
        turnOnOffBtn = new QPushButton(groupBox_6);
        turnOnOffBtn->setObjectName(QStringLiteral("turnOnOffBtn"));
        turnOnOffBtn->setGeometry(QRect(210, 20, 71, 20));
        tabWidget->addTab(Calibration, QString());
        Cable = new QWidget();
        Cable->setObjectName(QStringLiteral("Cable"));
        label_7 = new QLabel(Cable);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(10, 10, 171, 16));
        velocityFactor = new QLineEdit(Cable);
        velocityFactor->setObjectName(QStringLiteral("velocityFactor"));
        velocityFactor->setGeometry(QRect(190, 10, 51, 20));
        velocityFactor->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_8 = new QLabel(Cable);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setGeometry(QRect(10, 40, 171, 16));
        cableR0 = new QLineEdit(Cable);
        cableR0->setObjectName(QStringLiteral("cableR0"));
        cableR0->setGeometry(QRect(190, 40, 51, 20));
        cableR0->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_9 = new QLabel(Cable);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setGeometry(QRect(250, 40, 51, 16));
        groupBox_8 = new QGroupBox(Cable);
        groupBox_8->setObjectName(QStringLiteral("groupBox_8"));
        groupBox_8->setGeometry(QRect(10, 70, 511, 91));
        label_10 = new QLabel(groupBox_8);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setGeometry(QRect(10, 30, 101, 16));
        conductiveLoss = new QLineEdit(groupBox_8);
        conductiveLoss->setObjectName(QStringLiteral("conductiveLoss"));
        conductiveLoss->setGeometry(QRect(120, 30, 51, 20));
        conductiveLoss->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_11 = new QLabel(groupBox_8);
        label_11->setObjectName(QStringLiteral("label_11"));
        label_11->setGeometry(QRect(10, 60, 101, 16));
        dielectricLoss = new QLineEdit(groupBox_8);
        dielectricLoss->setObjectName(QStringLiteral("dielectricLoss"));
        dielectricLoss->setGeometry(QRect(120, 60, 51, 20));
        dielectricLoss->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        cableLossComboBox = new QComboBox(groupBox_8);
        cableLossComboBox->setObjectName(QStringLiteral("cableLossComboBox"));
        cableLossComboBox->setGeometry(QRect(180, 40, 101, 22));
        label_12 = new QLabel(groupBox_8);
        label_12->setObjectName(QStringLiteral("label_12"));
        label_12->setGeometry(QRect(300, 40, 16, 16));
        anyFq = new QRadioButton(groupBox_8);
        anyFq->setObjectName(QStringLiteral("anyFq"));
        anyFq->setGeometry(QRect(330, 30, 171, 17));
        anyFq->setChecked(true);
        atFq = new QRadioButton(groupBox_8);
        atFq->setObjectName(QStringLiteral("atFq"));
        atFq->setGeometry(QRect(330, 60, 21, 17));
        atMHz = new QLineEdit(groupBox_8);
        atMHz->setObjectName(QStringLiteral("atMHz"));
        atMHz->setEnabled(false);
        atMHz->setGeometry(QRect(350, 60, 61, 20));
        atMHz->setAlignment(Qt::AlignCenter);
        label_13 = new QLabel(groupBox_8);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setGeometry(QRect(420, 60, 31, 16));
        cableComboBox = new QComboBox(Cable);
        cableComboBox->setObjectName(QStringLiteral("cableComboBox"));
        cableComboBox->setGeometry(QRect(10, 170, 511, 22));
        label_14 = new QLabel(Cable);
        label_14->setObjectName(QStringLiteral("label_14"));
        label_14->setGeometry(QRect(10, 200, 111, 16));
        cableLen = new QLineEdit(Cable);
        cableLen->setObjectName(QStringLiteral("cableLen"));
        cableLen->setGeometry(QRect(130, 200, 51, 20));
        cableLen->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        groupBox_9 = new QGroupBox(Cable);
        groupBox_9->setObjectName(QStringLiteral("groupBox_9"));
        groupBox_9->setGeometry(QRect(10, 220, 511, 61));
        doNothingBtn = new QPushButton(groupBox_9);
        doNothingBtn->setObjectName(QStringLiteral("doNothingBtn"));
        doNothingBtn->setGeometry(QRect(10, 20, 151, 23));
        doNothingBtn->setCheckable(true);
        subtractCableBtn = new QPushButton(groupBox_9);
        subtractCableBtn->setObjectName(QStringLiteral("subtractCableBtn"));
        subtractCableBtn->setGeometry(QRect(184, 20, 151, 23));
        subtractCableBtn->setCheckable(true);
        addCableBtn = new QPushButton(groupBox_9);
        addCableBtn->setObjectName(QStringLiteral("addCableBtn"));
        addCableBtn->setGeometry(QRect(350, 20, 151, 23));
        addCableBtn->setCheckable(true);
        updateGraphsBtn = new QPushButton(Cable);
        updateGraphsBtn->setObjectName(QStringLiteral("updateGraphsBtn"));
        updateGraphsBtn->setGeometry(QRect(360, 200, 161, 23));
        tabWidget->addTab(Cable, QString());

        horizontalLayout->addWidget(tabWidget);


        retranslateUi(Settings);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(Settings);
    } // setupUi

    void retranslateUi(QDialog *Settings)
    {
        Settings->setWindowTitle(QApplication::translate("Settings", "Settings", 0));
        graphHintCheckBox->setText(QApplication::translate("Settings", "Show Graph Hint", 0));
        markersHintCheckBox->setText(QApplication::translate("Settings", "Show Markers Hint", 0));
        groupBox_7->setTitle(QApplication::translate("Settings", "Measure system", 0));
        measureSystemMetric->setText(QApplication::translate("Settings", "Metric", 0));
        measureSystemAmerican->setText(QApplication::translate("Settings", "American", 0));
        label_4->setText(QApplication::translate("Settings", "System impedance:", 0));
        label_6->setText(QApplication::translate("Settings", "Ohm", 0));
        graphBriefHintCheckBox->setText(QApplication::translate("Settings", "Show Brief params under cursor", 0));
        groupBox_12->setTitle(QApplication::translate("Settings", "Analyzer detection", 0));
        label_15->setText(QApplication::translate("Settings", "Serial port:", 0));
        autoDetect->setText(QApplication::translate("Settings", "Auto", 0));
        manualDetect->setText(QApplication::translate("Settings", "Manual", 0));
        languageLabel->setText(QApplication::translate("Settings", "Language:", 0));
        tabWidget->setTabText(tabWidget->indexOf(General), QApplication::translate("Settings", "General", 0));
        groupBox_10->setTitle(QApplication::translate("Settings", "Analyzer", 0));
        groupBox->setTitle(QApplication::translate("Settings", "Info", 0));
        label->setText(QApplication::translate("Settings", "Analyzer model:", 0));
        label_2->setText(QApplication::translate("Settings", "Version of firmware:", 0));
        label_5->setText(QApplication::translate("Settings", "Serial number:", 0));
        groupBox_2->setTitle(QApplication::translate("Settings", "Update from file", 0));
        browseBtn->setText(QApplication::translate("Settings", "Browse", 0));
        updateBtn->setText(QApplication::translate("Settings", "Update", 0));
        autoUpdatesCheckBox->setText(QApplication::translate("Settings", "Auto check updates", 0));
        checkUpdatesBtn->setText(QApplication::translate("Settings", "Check Updates", 0));
        groupBox_11->setTitle(QApplication::translate("Settings", "AntScope2", 0));
        label_16->setText(QApplication::translate("Settings", "Version:", 0));
        checkBox_AntScopeAutoUpdate->setText(QApplication::translate("Settings", "Auto check updates", 0));
        tabWidget->setTabText(tabWidget->indexOf(Updates), QApplication::translate("Settings", "Updates", 0));
        groupBox_3->setTitle(QApplication::translate("Settings", "Open", 0));
        openCalibBtn->setText(QApplication::translate("Settings", "Start Open Calibration", 0));
        labelOpenState->setText(QApplication::translate("Settings", "Not choosed", 0));
        openOpenFileBtn->setText(QApplication::translate("Settings", "Open file", 0));
        groupBox_4->setTitle(QApplication::translate("Settings", "Short", 0));
        shortCalibBtn->setText(QApplication::translate("Settings", "Start Short Calibration", 0));
        shortOpenFileBtn->setText(QApplication::translate("Settings", "Open file", 0));
        labelShortState->setText(QApplication::translate("Settings", "Not choosed", 0));
        groupBox_5->setTitle(QApplication::translate("Settings", "Load", 0));
        loadCalibBtn->setText(QApplication::translate("Settings", "Start Load Calibration", 0));
        loadOpenFileBtn->setText(QApplication::translate("Settings", "Open file", 0));
        labelLoadState->setText(QApplication::translate("Settings", "Not choosed", 0));
        groupBox_6->setTitle(QApplication::translate("Settings", "Calibration Wizard", 0));
        calibWizard->setText(QApplication::translate("Settings", "Start", 0));
        labelWizardStatus->setText(QApplication::translate("Settings", "Calibration is off", 0));
        label_3->setText(QApplication::translate("Settings", "Status:", 0));
        turnOnOffBtn->setText(QApplication::translate("Settings", "Turn On", 0));
        tabWidget->setTabText(tabWidget->indexOf(Calibration), QApplication::translate("Settings", "OSL Calibration", 0));
        label_7->setText(QApplication::translate("Settings", "Velocity factor", 0));
        label_8->setText(QApplication::translate("Settings", "Cable R0", 0));
        label_9->setText(QApplication::translate("Settings", "Ohm", 0));
        groupBox_8->setTitle(QApplication::translate("Settings", "Cable loss", 0));
        label_10->setText(QApplication::translate("Settings", "Conductive loss", 0));
        label_11->setText(QApplication::translate("Settings", "Dielectric loss", 0));
        cableLossComboBox->clear();
        cableLossComboBox->insertItems(0, QStringList()
         << QApplication::translate("Settings", "dB/100feet", 0)
         << QApplication::translate("Settings", "dB/feet", 0)
         << QApplication::translate("Settings", "dB/100m", 0)
         << QApplication::translate("Settings", "dB/m", 0)
        );
        label_12->setText(QApplication::translate("Settings", "at", 0));
        anyFq->setText(QApplication::translate("Settings", "any frequency", 0));
        atFq->setText(QString());
        label_13->setText(QApplication::translate("Settings", "MHz", 0));
        label_14->setText(QApplication::translate("Settings", "Cable length", 0));
        groupBox_9->setTitle(QApplication::translate("Settings", "Transmission line options", 0));
        doNothingBtn->setText(QApplication::translate("Settings", "Do nothing", 0));
        subtractCableBtn->setText(QApplication::translate("Settings", "Subtract cable", 0));
        addCableBtn->setText(QApplication::translate("Settings", "Add cable", 0));
        updateGraphsBtn->setText(QApplication::translate("Settings", "Update graphs", 0));
        tabWidget->setTabText(tabWidget->indexOf(Cable), QApplication::translate("Settings", "Cable", 0));
    } // retranslateUi

};

namespace Ui {
    class Settings: public Ui_Settings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGS_H

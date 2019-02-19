#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "popupindicator.h"
#include "analyzer/customanalyzer.h"

extern QString appendSpaces(const QString& number);
extern bool g_noRestrictScale; // see main.cpp

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_fqSettings(NULL),
    m_measurements(NULL),
    m_markers(NULL),
    m_settings(NULL),
    m_settingsDialog(NULL),
    m_exportDialog(NULL),
    m_screenshot(NULL),
    m_calibration(NULL),
    m_isContinuos(false),
    m_dotsNumber(50),
    m_updater(NULL),
    m_updateDialog(NULL),
    m_deferredUpdate(false),
    m_swrZoomState(10),
    m_phaseZoomState(10),
    m_rsZoomState(10),
    m_rpZoomState(10),
    m_rlZoomState(10),
    m_tdrZoomState(10),
    m_smithZoomState(10),
    m_autoDetectMode(true),
    m_languageNumber(0),
    m_addingMarker(false),
    m_bInterrupted(false)
{
    ui->setupUi(this);
    m_qtLanguageTranslator = new QTranslator();

    QString path = Settings::setIniFile();
    m_settings = new QSettings(path, QSettings::IniFormat);
    m_settings->beginGroup("MainWindow");
    createTabs(m_settings->value("tabSequence","0123456").toString());
    ui->tabWidget->setCurrentIndex(m_settings->value("currentTab",0).toInt());

    m_swrZoomState = m_settings->value("swrZoomState", 10).toInt();
    m_phaseZoomState = m_settings->value("phaseZoomState", 10).toInt();
    m_rsZoomState = m_settings->value("rsZoomState", 10).toInt();
    m_rpZoomState = m_settings->value("rpZoomState", 10).toInt();
    m_rlZoomState = m_settings->value("rlZoomState", 10).toInt();
    m_tdrZoomState = m_settings->value("tdrZoomState", 10).toInt();
    m_smithZoomState = m_settings->value("smithZoomState", 10).toInt();

    m_settings->endGroup();

    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint |
                                   Qt::WindowMinimizeButtonHint |
                                   Qt::WindowMaximizeButtonHint |
                                   Qt::WindowCloseButtonHint);

    ui->singleStart->setEnabled(false);
    ui->continuousStartBtn->setEnabled(false);
    ui->analyzerDataBtn->setEnabled(false);
    ui->screenshotAA->setEnabled(false);
    ui->measurmentsSaveBtn->setEnabled(false);    
    ui->measurmentsDeleteBtn->setEnabled(false);
    ui->measurmentsClearBtn->setEnabled(false);
    ui->exportBtn->setEnabled(false);


    qApp->setStyle(QStyleFactory::create("fusion"));
    QPalette asPalette;
    asPalette.setColor(QPalette::Foreground, QColor(1,178,255));
    asPalette.setColor(QPalette::Button, QColor(89, 89, 89));
    asPalette.setColor(QPalette::ButtonText, QColor(255,255,255));
    asPalette.setColor(QPalette::Highlight, QColor(1,178,255));
    asPalette.setColor(QPalette::Background, QColor(79, 79, 79));
    asPalette.setColor(QPalette::Dark, QColor(1,178,255));
    asPalette.setColor(QPalette::Light, QColor(1,178,255));

    qApp->setPalette(asPalette);

    QString style = "QPushButton:checked{"
    "background-color: rgb(1, 178, 255);}";

    ui->limitsBtn->setStyleSheet(style);
    ui->rangeBtn->setStyleSheet(style);

    style = "QPushButton:disabled{"
            "background-color: rgb(59, 59, 59);"
            "color: rgb(119, 119, 119);}";

    ui->exportBtn->setStyleSheet(style);
    ui->printBtn->setStyleSheet(style);
    ui->screenshotAA->setStyleSheet(style);
    ui->analyzerDataBtn->setStyleSheet(style);

    ui->measurmentsSaveBtn->setStyleSheet(style);
    ui->measurmentsDeleteBtn->setStyleSheet(style);

    ui->presetsDeleteBtn->setStyleSheet(style);
    ui->measurmentsClearBtn->setStyleSheet(style);


    style = "QPushButton:disabled{"
            "background-color: rgb(59, 59, 59);"
            "color: rgb(119, 119, 119);}"
            "QPushButton:checked{"
            "background-color: rgb(1, 178, 255);}";
    ui->singleStart->setStyleSheet(style);
    ui->continuousStartBtn->setStyleSheet(style);

    ui->tableWidget_measurments->setColumnCount(1);
    ui->tableWidget_measurments->setSelectionBehavior(QAbstractItemView::SelectRows );

    style = "QGroupBox {border: 2px solid rgb(1, 178, 255); margin-top: 1ex;}"
            "QGroupBox::title {"
            "subcontrol-origin: margin;"
            "subcontrol-position: top center;" /* position at the top center */
            "padding: 0 3px;}"
            "QGroupBox::title {color: white;}";

    ui->groupBox_Run->setStyleSheet(style);

    setWidgetsSettings();

    foreach (QCustomPlot *plot, m_mapWidgets) {
        plot->setMouseTracking(true);
        plot->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(plot,
                SIGNAL(customContextMenuRequested(const QPoint&)),
                SLOT(onCustomContextMenuRequested(const QPoint&)));
        connect(plot,SIGNAL(mouseDoubleClick(QMouseEvent*)),this, SLOT(on_mouseDoubleClick(QMouseEvent*)));
    }
    connect(m_swrWidget,SIGNAL(mouseWheel(QWheelEvent*)),this, SLOT(on_mouseWheel_swr(QWheelEvent*)));
    connect(m_swrWidget,SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(on_mouseMove_swr(QMouseEvent*)));

    connect(m_phaseWidget,SIGNAL(mouseWheel(QWheelEvent*)),this, SLOT(on_mouseWheel_phase(QWheelEvent*)));
    connect(m_phaseWidget,SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(on_mouseMove_phase(QMouseEvent*)));

    connect(m_rsWidget,SIGNAL(mouseWheel(QWheelEvent*)),this, SLOT(on_mouseWheel_rs(QWheelEvent*)));
    connect(m_rsWidget,SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(on_mouseMove_rs(QMouseEvent*)));

    connect(m_rpWidget,SIGNAL(mouseWheel(QWheelEvent*)),this, SLOT(on_mouseWheel_rp(QWheelEvent*)));
    connect(m_rpWidget,SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(on_mouseMove_rp(QMouseEvent*)));

    connect(m_rlWidget,SIGNAL(mouseWheel(QWheelEvent*)),this, SLOT(on_mouseWheel_rl(QWheelEvent*)));
    connect(m_rlWidget,SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(on_mouseMove_rl(QMouseEvent*)));

    connect(m_tdrWidget,SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(on_mouseMove_tdr(QMouseEvent*)));
    //connect(m_tdrWidget,SIGNAL(mouseWheel(QWheelEvent*)),this, SLOT(on_mouseWheel_tdr(QWheelEvent*)));

    connect(m_smithWidget,SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(on_mouseMove_smith(QMouseEvent*)));

    m_analyzer = new Analyzer(this);
    connect(m_analyzer,SIGNAL(analyzerFound(QString)),this,SLOT(on_analyzerFound(QString)));
    connect(m_analyzer,SIGNAL(analyzerDisconnected()),this,SLOT(on_analyzerDisconnected()));
    connect(this,SIGNAL(measure(qint64,qint64,int)),m_analyzer,SLOT(on_measure(qint64,qint64,int)));
    connect(this,SIGNAL(measureContinuous(qint64,qint64,int)),m_analyzer,SLOT(on_measureContinuous(qint64,qint64,int)));
    connect(m_analyzer,SIGNAL(measurementComplete()),this,SLOT(on_measurementComplete()), Qt::QueuedConnection);
    connect(this,SIGNAL(stopMeasure()), m_analyzer, SLOT(on_stopMeasure()));

    QShortcut *shortF1 = new QShortcut(QKeySequence("F1"),this);
    connect(shortF1,SIGNAL(activated()),this,SLOT(on_pressF1()));

    QShortcut *shortF2 = new QShortcut(QKeySequence("F2"),this);
    connect(shortF2,SIGNAL(activated()),this,SLOT(on_pressF2()));

    QShortcut *shortF3 = new QShortcut(QKeySequence("F3"),this);
    connect(shortF3,SIGNAL(activated()),this,SLOT(on_pressF3()));

    QShortcut *shortF4 = new QShortcut(QKeySequence("F4"),this);
    connect(shortF4,SIGNAL(activated()),this,SLOT(on_pressF4()));

    QShortcut *shortF5 = new QShortcut(QKeySequence("F5"),this);
    connect(shortF5,SIGNAL(activated()),this,SLOT(on_pressF5()));

    QShortcut *shortF6 = new QShortcut(QKeySequence("F6"),this);
    connect(shortF6,SIGNAL(activated()),this,SLOT(on_pressF6()));

    QShortcut *shortF7 = new QShortcut(QKeySequence("F7"),this);
    connect(shortF7,SIGNAL(activated()),this,SLOT(on_pressF7()));

    QShortcut *shortEsc = new QShortcut(QKeySequence("Esc"),this);
    connect(shortEsc,SIGNAL(activated()),this,SLOT(on_pressEsc()));

    QShortcut *shortF9 = new QShortcut(QKeySequence("F9"),this);
    connect(shortF9,SIGNAL(activated()),this,SLOT(on_pressF9()));

    QShortcut *shortF10 = new QShortcut(QKeySequence("F10"),this);
    connect(shortF10,SIGNAL(activated()),this,SLOT(on_pressF10()));

    QShortcut *shortDelete = new QShortcut(QKeySequence("Delete"),this);
    connect(shortDelete,SIGNAL(activated()),this,SLOT(on_pressDelete()));

    QShortcut *shortPlus = new QShortcut(QKeySequence("+"),this);
    connect(shortPlus,SIGNAL(activated()),this,SLOT(on_pressPlus()));
    QShortcut *shortEqual = new QShortcut(QKeySequence("="),this);
    connect(shortEqual,SIGNAL(activated()),this,SLOT(on_pressPlus()));
    QShortcut *shortMinus = new QShortcut(QKeySequence("-"),this);
    connect(shortMinus,SIGNAL(activated()),this,SLOT(on_pressMinus()));    
    QShortcut *shortUp = new QShortcut(QKeySequence(Qt::Key_Up),this);
    connect(shortUp,SIGNAL(activated()),this,SLOT(on_pressPlus()));
    QShortcut *shortDoun = new QShortcut(QKeySequence(Qt::Key_Down),this);
    connect(shortDoun,SIGNAL(activated()),this,SLOT(on_pressMinus()));

    QShortcut *shortCtrlPlus = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus),this);
    connect(shortCtrlPlus,SIGNAL(activated()),this,SLOT(on_pressCtrlPlus()));
    QShortcut *shortCtrlEqual = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Equal),this);
    connect(shortCtrlEqual,SIGNAL(activated()),this,SLOT(on_pressCtrlPlus()));
    QShortcut *shortCtrlUp = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Up),this);
    connect(shortCtrlUp,SIGNAL(activated()),this,SLOT(on_pressCtrlPlus()));
    QShortcut *shortCtrlMinus = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus),this);
    connect(shortCtrlMinus,SIGNAL(activated()),this,SLOT(on_pressCtrlMinus()));
    QShortcut *shortCtrlDoun = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down),this);
    connect(shortCtrlDoun,SIGNAL(activated()),this,SLOT(on_pressCtrlMinus()));
    QShortcut *shortCtrlZero = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Insert),this);
    connect(shortCtrlZero,SIGNAL(activated()),this,SLOT(on_pressCtrlZero()));

    QShortcut *shortLeft = new QShortcut(QKeySequence(Qt::Key_Left),this);
    connect(shortLeft,SIGNAL(activated()),this,SLOT(on_pressLeft()));

    QShortcut *shortRight = new QShortcut(QKeySequence(Qt::Key_Right),this);
    connect(shortRight,SIGNAL(activated()),this,SLOT(on_pressRight()));

    QShortcut *shortCtrlC = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C),this);
    connect(shortCtrlC,SIGNAL(activated()),this,SLOT(on_pressCtrlC()));

    m_presets = new Presets(this);
    connect(this, SIGNAL(isRangeChanged(bool)), m_presets, SLOT(on_isRangeChanged(bool)));
    m_presets->setTable(ui->tableWidget_presets);

    m_measurements = new Measurements (this);
    m_measurements->setWidgets(m_swrWidget,
                               m_phaseWidget,
                               m_rsWidget,
                               m_rpWidget,
                               m_rlWidget,
                               m_tdrWidget,
                               m_smithWidget,
                               ui->tableWidget_measurments);
    connect(m_analyzer, SIGNAL(newData(rawData)), m_measurements, SLOT(on_newData(rawData)));
    connect(m_analyzer, SIGNAL(newMeasurement(QString)), m_measurements, SLOT(on_newMeasurement(QString)));
    connect(m_analyzer, SIGNAL(continueMeasurement(qint64, qint64, qint32)), m_measurements, SLOT(on_continueMeasurement(qint64, qint64, qint32)));
    connect(this, SIGNAL(currentTab(QString)), m_measurements, SLOT(on_currentTab(QString)));
    connect(this, SIGNAL(focus(bool)), m_measurements,SLOT(on_focus(bool)));
    connect(this, SIGNAL(newCursorFq(double, int, int, int)), m_measurements,SLOT(on_newCursorFq(double, int, int, int)));
    connect(this, SIGNAL(newCursorSmithPos(double, double, int)), m_measurements,SLOT(on_newCursorSmithPos(double, double, int)));
    connect(this, SIGNAL(mainWindowPos(int,int)), m_measurements,SLOT(on_mainWindowPos(int,int)));
    connect(m_measurements, SIGNAL(calibrationChanged()), this,SLOT(on_calibrationChanged()));
    connect(m_measurements, &Measurements::import_finished, this, &MainWindow::on_importFinished);

    QString name = "AntScope2 v." + QString(ANTSCOPE2VER);
    name += tr(" - Analyzer not connected");
    setWindowTitle(name);


    if(m_markers == NULL)
    {
        m_markers = new Markers(this);
        m_markers->setWidgets(m_swrWidget,
                              m_phaseWidget,
                              m_rsWidget,
                              m_rpWidget,
                              m_rlWidget,
                              m_tdrWidget,
                              m_smithWidget);
        m_markers->setMeasurements(m_measurements);
        connect(this, SIGNAL(focus(bool)), m_markers,SLOT(on_focus(bool)));
        connect(this, SIGNAL(mainWindowPos(int,int)), m_markers,SLOT(on_mainWindowPos(int,int)));
        connect(this, SIGNAL(currentTab(QString)), m_markers, SLOT(on_currentTab(QString)));
        connect(this, SIGNAL(rescale()), m_markers, SLOT(rescale()));
        connect(m_analyzer, SIGNAL(newMeasurement(QString)), m_markers, SLOT(on_newMeasurement(QString)));
        connect(m_analyzer, SIGNAL(measurementComplete()), m_markers, SLOT(on_measurementComplete()));
    }


    m_calibration = new Calibration();
    m_calibration->setAnalyzer(m_analyzer);
    m_calibration->start();
    connect(m_calibration,SIGNAL(setCalibrationMode(bool)),
            m_analyzer,SLOT(setCalibrationMode(bool)));
    connect(m_calibration,SIGNAL(setCalibrationMode(bool)),
            m_measurements,SLOT(setCalibrationMode(bool)));
    m_measurements->setCalibration(m_calibration);

    style = "QCheckBox:disabled{"
            "color: rgb(119, 119, 119);}";

    ui->checkBoxCalibration->blockSignals(true);
    ui->checkBoxCalibration->setStyleSheet(style);
    ui->checkBoxCalibration->setEnabled(m_calibration->isCalibrationPerformed());
    ui->checkBoxCalibration->setChecked(m_calibration->getCalibrationEnabled());
    connect(ui->checkBoxCalibration, &QCheckBox::toggled, this, &MainWindow::calibrationToggled);

//    QWidget* w = ui->checkBoxCalibration;
//    QTimer::singleShot(1000, [w]() {
//        // to avoid calibration message due to ui->checkBoxCalibration->setChecked
//        // at load time
//        w->blockSignals(false);
//    });

    connect(ui->measurmentsClearBtn, &QPushButton::clicked, this, &MainWindow::on_measurementsClearBtn_clicked);

    QSettings settings1 ("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
    settings1.setValue (".asd/.", "AntScope2.file");
    settings1.setValue ("AntScope2.file/.", tr("File of AntScope2"));
    settings1.setValue ("AntScope2.file/shell/open/command/.",
                        "\"" + QDir::toNativeSeparators (QCoreApplication::applicationFilePath()) + "\"" + " \"%1\"");


    m_settings->beginGroup("MainWindow");
    m_lastOpenPath = m_settings->value("lastOpenPath", "").toString();
    m_lastSavePath = m_settings->value("lastSavePath", "").toString();
    if( m_settings->value("fullScreen", this->isFullScreen()).toBool())
    {
        this->showMaximized();
    }else
    {
        QRect rect = m_settings->value("geometry", 0).toRect();
        if(rect.x() != 0)
        {
            this->setGeometry(rect);
        }else
        {
            this->setGeometry(177, 131, 1230, 700);
        }
    }
    m_dotsNumber = m_settings->value("dotsNumber", 50).toInt();
    m_measureSystemMetric = m_settings->value("measureSystemMetric", true).toBool();
    m_Z0 = m_settings->value("systemImpedance", 50).toDouble();
    m_calibration->setZ0(m_Z0);
    m_measurements->setZ0(m_Z0);
    m_measurements->on_dotsNumberChanged(m_dotsNumber);
    m_measurements->on_changeMeasureSystemMetric(m_measureSystemMetric);
    QCPRange range(m_settings->value("rangeLower",0).toDouble(), m_settings->value("rangeUpper",1400000).toDouble());
    m_swrWidget->xAxis->setRange(range);
    m_phaseWidget->xAxis->setRange(range);
    m_rsWidget->xAxis->setRange(range);
    m_rpWidget->xAxis->setRange(range);
    m_rlWidget->xAxis->setRange(range);
    m_isRange = m_settings->value("isRange", false).toBool();
    if(!m_isRange)
    {
        ui->limitsBtn->setChecked(true);
        on_limitsBtn_clicked(true);
//        ui->rangeBtn->setChecked(false);
        setFqFrom(range.lower);
        setFqTo(range.upper);
    }else
    {
        ui->rangeBtn->setChecked(true);
        on_rangeBtn_clicked(true);
//        ui->rangeBtn->setChecked(true);
//        ui->limitsBtn->setChecked(false);
        setFqFrom((range.upper + range.lower)/2);
        setFqTo((range.upper - range.lower)/2);
    }
    ui->spinBoxPoints->setValue(m_dotsNumber);
    connect(ui->spinBoxPoints, SIGNAL(valueChanged(int)), this, SLOT(onSpinChanged(int)));
    connect(ui->fullBtn, &QPushButton::clicked, this, &MainWindow::onFullRange);

    m_autoFirmwareUpdateEnabled = m_settings->value("autoFirmwareUpdate", true).toBool();
    m_autoUpdateEnabled = m_settings->value("autoUpdate", true).toBool();
    m_autoDetectMode = m_settings->value("autoDetectMode",true).toBool();
    m_serialPort = m_settings->value("serialPort","").toString();

    m_analyzer->on_changedAutoDetectMode(m_autoDetectMode);
    m_analyzer->on_changedSerialPort(m_serialPort);

    m_swrZoomState = m_settings->value("swrZoomState", 10).toInt();
    m_phaseZoomState = m_settings->value("phaseZoomState", 10).toInt();
    m_rsZoomState = m_settings->value("rsZoomState", 10).toInt();
    m_rpZoomState = m_settings->value("rpZoomState", 10).toInt();
    m_rlZoomState = m_settings->value("rlZoomState", 10).toInt();
    m_tdrZoomState = m_settings->value("tdrZoomState", 10).toInt();
    m_smithZoomState = m_settings->value("smithZoomState", 10).toInt();

    m_languageNumber = m_settings->value("languageNumber",0).toInt();

    m_settings->endGroup();

    m_settings->beginGroup("Cable");
    m_cableVelFactor = m_settings->value("VelFactor",0.66 ).toDouble();
    m_cableResistance = m_settings->value("R0",50 ).toDouble();
    m_cableLossConductive = m_settings->value("ConductiveLoss",0 ).toDouble();
    m_cableLossDielectric = m_settings->value("DielectricLoss",0 ).toDouble();
    m_cableLossFqMHz = m_settings->value("LossFrequencyMHz",1 ).toDouble();
    m_cableLossUnits = m_settings->value("LossUnits",0 ).toInt();
    m_cableLossAtAnyFq = m_settings->value("LossAtAnyFrequency",0 ).toInt();
    m_cableLength = m_settings->value("Length",0 ).toDouble();
    m_farEndMeasurement = m_settings->value("FarEndMeasurement", 0 ).toInt();
    m_cableIndex = m_settings->value("CableIndex",0).toInt();
    m_settings->endGroup();

    m_measurements->setCableVelFactor(m_cableVelFactor);
    m_measurements->setCableResistance(m_cableResistance);
    m_measurements->setCableLossConductive(m_cableLossConductive);
    m_measurements->setCableLossDielectric(m_cableLossDielectric);
    m_measurements->setCableLossFqMHz(m_cableLossFqMHz);
    m_measurements->setCableLossUnits(m_cableLossUnits);
    m_measurements->setCableLossAtAnyFq(m_cableLossAtAnyFq);
    m_measurements->setCableLength(m_cableLength);
    m_measurements->setCableFarEndMeasurement(m_farEndMeasurement);

    for (int i=0; i<ui->tabWidget->count(); i++)
    {
        QString tooltip = QString(tr("Press F%1")).arg(i+1);
        ui->tabWidget->setTabToolTip(i, tooltip);
    }

    QString str = ui->tabWidget->currentWidget()->objectName();
    emit currentTab (str);
    QTimer::singleShot(100, this, SLOT(updateGraph()));

//#ifdef Q_OS_WIN
    m_updater = new Updater();
    connect(m_updater,SIGNAL(newVersionAvailable()), this, SLOT(on_newVersionAvailable()));
    connect(m_updater,SIGNAL(progress(int)), this, SIGNAL(updateProgress(int)));
    if(m_autoUpdateEnabled)
    {
        QTimer::singleShot(1000, m_updater, SLOT(on_checkUpdates()));
    }
//#endif

    m_1secTimer = new QTimer(this);
    connect(m_1secTimer, SIGNAL(timeout()), this, SLOT(on_1secTimerTick()));
    m_1secTimer->start(100);

    loadLanguage(languages_small[m_languageNumber]);
    ui->tableWidget_presets->horizontalHeader()->show();
    if(!m_isRange)
    {
        ui->groupBox_Presets->setTitle(tr("Presets (limits), kHz"));
        ui->tableWidget_presets->horizontalHeaderItem(0)->setText(tr("Start"));
        ui->tableWidget_presets->horizontalHeaderItem(1)->setText(tr("Stop"));
//        ui->startLabel->setText(tr("Start"));
//        ui->stopLabel->setText(tr("Stop"));
//        ui->startLabel->update();
//        ui->stopLabel->update();
    }else
    {
        ui->groupBox_Presets->setTitle(tr("Presets (center, range), kHz"));
        ui->tableWidget_presets->horizontalHeaderItem(0)->setText(tr("Center"));
        ui->tableWidget_presets->horizontalHeaderItem(1)->setText(tr("Range(+/-)"));
//        ui->startLabel->setText(tr("Center"));
//        ui->stopLabel->setText(tr("Range (+/-)"));
//        ui->startLabel->update();
//        ui->stopLabel->update();
    }
    //PopUpIndicator::hideIndicator(ui->tabWidget);
    PopUpIndicator::hideIndicator(m_swrWidget);

    QWidget* w = ui->checkBoxCalibration;
    QTimer::singleShot(1000, [w]() {
        // to avoid calibration message due to ui->checkBoxCalibration->setChecked
        // at load time
        w->blockSignals(false);
    });

    QTimer::singleShot(100, [this]() {
        // force labels' text changing
        if(this->m_isRange) {
            this->ui->limitsBtn->setChecked(false);
            this->ui->startLabel->setText(tr("Center"));
            this->ui->stopLabel->setText(tr("Range (+/-)"));
            this->ui->groupBox_Presets->setTitle(tr("Presets (center, range), kHz"));
            this->ui->tableWidget_presets->horizontalHeaderItem(0)->setText(tr("Center"));
            this->ui->tableWidget_presets->horizontalHeaderItem(1)->setText(tr("Range(+/-)"));
        } else {
            this->ui->rangeBtn->setChecked(false);
            this->ui->startLabel->setText(tr("Start"));
            this->ui->stopLabel->setText(tr("Stop"));
            this->ui->groupBox_Presets->setTitle(tr("Presets (limits), kHz"));
            this->ui->tableWidget_presets->horizontalHeaderItem(0)->setText(tr("Start"));
            this->ui->tableWidget_presets->horizontalHeaderItem(1)->setText(tr("Stop"));
        }
    });

}

MainWindow::~MainWindow()
{
    QList<QStringList*> values = m_BandsMap.values();
    while (!values.isEmpty()) {
        QStringList* lst = values.takeLast();
        delete lst;
    }
    m_BandsMap.clear();

    if(m_calibration)
    {
        delete m_calibration;
    }
    if(m_updater)
    {
        delete m_updater;
    }
    if(m_updateDialog)
    {
        delete m_updateDialog;
    }
    m_settings->beginGroup("MainWindow");
    m_settings->setValue("geometry", this->geometry());
    m_settings->setValue("fullScreen", this->isMaximized());
    m_settings->setValue("dotsNumber", this->m_dotsNumber);

    m_settings->setValue("lastOpenPath", m_lastOpenPath);
    m_settings->setValue("lastSavePath", m_lastSavePath);
    m_settings->setValue("measureSystemMetric", m_measureSystemMetric);
    m_settings->setValue("isRange", m_isRange);

    QString str;
    str.append(QString::number(ui->tabWidget->indexOf(m_tab_1)));
    str.append(QString::number(ui->tabWidget->indexOf(m_tab_2)));
    str.append(QString::number(ui->tabWidget->indexOf(m_tab_3)));
    str.append(QString::number(ui->tabWidget->indexOf(m_tab_4)));
    str.append(QString::number(ui->tabWidget->indexOf(m_tab_5)));
    str.append(QString::number(ui->tabWidget->indexOf(m_tab_6)));
    str.append(QString::number(ui->tabWidget->indexOf(m_tab_7)));
    m_settings->setValue("tabSequence",str);
    m_settings->setValue("currentTab",ui->tabWidget->currentIndex());
    m_settings->setValue("systemImpedance", m_Z0);
    m_settings->setValue("rangeLower", m_swrWidget->xAxis->range().lower);
    m_settings->setValue("rangeUpper", m_swrWidget->xAxis->range().upper);
    m_settings->setValue("autoFirmwareUpdate", m_autoFirmwareUpdateEnabled);
    m_settings->setValue("autoUpdate", m_autoUpdateEnabled);
    m_settings->setValue("autoDetectMode", m_autoDetectMode);
    m_settings->setValue("serialPort",m_serialPort);

    m_settings->setValue("swrZoomState", m_swrZoomState);
    m_settings->setValue("phaseZoomState", m_phaseZoomState);
    m_settings->setValue("rsZoomState", m_rsZoomState);
    m_settings->setValue("rpZoomState", m_rpZoomState);
    m_settings->setValue("rlZoomState", m_rlZoomState);
    m_settings->setValue("tdrZoomState", m_tdrZoomState);
    m_settings->setValue("smithZoomState", m_smithZoomState);

    m_settings->setValue("languageNumber", m_languageNumber);

    m_settings->endGroup();

    m_settings->beginGroup("Cable");
    m_settings->setValue("VelFactor",m_cableVelFactor );
    m_settings->setValue("R0", m_cableResistance);
    m_settings->setValue("ConductiveLoss", m_cableLossConductive);
    m_settings->setValue("DielectricLoss", m_cableLossDielectric);
    m_settings->setValue("LossFrequencyMHz", m_cableLossFqMHz);
    m_settings->setValue("LossUnits", m_cableLossUnits);
    m_settings->setValue("LossAtAnyFrequency", m_cableLossAtAnyFq);
    m_settings->setValue("Length", m_cableLength);
    m_settings->setValue("FarEndMeasurement", m_farEndMeasurement);
    m_settings->setValue("CableIndex", m_cableIndex);
    m_settings->endGroup();

    if(m_qtLanguageTranslator)
    {
        delete m_qtLanguageTranslator;
    }
    delete ui;
}

void MainWindow::changeEvent(QEvent* event)
{
    if(0 != event) {
        switch(event->type()) {
        // this event is send if a translator is loaded
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;

            // this event is send, if the system, language changes
        case QEvent::LocaleChange:
        {
            QString locale = QLocale::system().name();
            locale.truncate(locale.lastIndexOf('_'));
            loadLanguage(locale);
        }
            break;
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(m_deferredUpdate)
    {
        if(m_updateDialog == NULL)
        {
            m_updateDialog = new AntScopeUpdateDialog();
        }
        m_updateDialog->setAsAfterClosing();
        m_updateDialog->show();
    }else
    {
        QMainWindow::closeEvent(event);
    }
}

bool MainWindow::event(QEvent * e)
{
    if(e->type() == QEvent::WindowActivate)
    {
        emit focus(true);
    }else if (e->type() == QEvent::WindowDeactivate)
    {
        emit focus(false);
    }else if (e->type() == QEvent::WindowStateChange)
    {
        updateGraph();
    }
    return QMainWindow::event(e) ;
}

void MainWindow::setWidgetsSettings()
{
    QPen pen;
    pen.setColor(QColor(0, 0, 0, 150));
    pen.setWidthF(INACTIVE_GRAPH_PEN_WIDTH);

    QFont fontTickLabel = m_swrWidget->xAxis->tickLabelFont();
    QFont fontLabel = fontTickLabel;
    fontTickLabel.setPointSize(11);
    fontLabel.setPointSize(12);

    bool bands_loaded = loadBands();
    QStringList* bands = nullptr;
    if (bands_loaded)
    {
        m_settings->beginGroup("Settings");
        QString band = m_settings->value("current_band", "ITU Region 1 - Europe, Africa").toString();
        m_settings->endGroup();
        if (m_BandsMap.contains(band))
        {
            bands = m_BandsMap[band];
        }
    }

    //-------SWR Widget---------------------------------------------
    m_swrWidget->addGraph();//graph(0) - SWR
    setBands(m_swrWidget, bands, 1, MAX_SWR);

    m_swrWidget->graph(0)->setPen(pen);
    m_swrWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_swrWidget->yAxis->setLabel(tr("SWR"));
    m_swrWidget->xAxis->setRange(0,1400000);
    m_swrWidget->yAxis->setRangeMin(1);
    m_swrWidget->yAxis->setRangeMax(MAX_SWR);
    m_swrWidget->yAxis->setRange(1, m_swrZoomState+0.2);

     //| Qt::Vertical | Qt::Horizontal
    m_swrWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_swrWidget->axisRect()->setRangeZoom(Qt::Horizontal);// | Qt::Vertical);
    m_swrWidget->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_swrWidget->xAxis->setTickLabelFont(fontTickLabel);
    m_swrWidget->yAxis->setTickLabelFont(fontTickLabel);
    m_swrWidget->xAxis->setLabelFont(fontLabel);
    m_swrWidget->yAxis->setLabelFont(fontLabel);
    m_swrWidget->replot();

    //-------Phase Widget---------------------------------------------
    m_phaseWidget->addGraph();//graph(0)
    setBands(m_phaseWidget, bands, -180, 180);
    m_phaseWidget->graph(0)->setPen(pen);
    //ui->swr_widget->axisRect()->setBackground(Qt::black);
    m_phaseWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_phaseWidget->yAxis->setLabel(tr("Angle"));
    m_phaseWidget->xAxis->setRange(0,1400000);
    m_phaseWidget->yAxis->setRangeMin(-180);
    m_phaseWidget->yAxis->setRangeMax(180);
    m_phaseWidget->yAxis->setRange(-180, 180);
    m_phaseWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_phaseWidget->axisRect()->setRangeZoom(Qt::Horizontal);
    m_phaseWidget->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_phaseWidget->xAxis->setTickLabelFont(fontTickLabel);
    m_phaseWidget->yAxis->setTickLabelFont(fontTickLabel);
    m_phaseWidget->xAxis->setLabelFont(fontLabel);
    m_phaseWidget->yAxis->setLabelFont(fontLabel);
    m_phaseWidget->replot();

    //-------RSeries Widget------------------------------------------------
    m_rsWidget->setAutoAddPlottableToLegend(false);
    m_rsWidget->addGraph();//graph(0)
    setBands(m_rsWidget, bands, -2000, 2000);
    m_rsWidget->graph(0)->setPen(pen);
    m_rsWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_rsWidget->yAxis->setLabel(tr("Rs, Ohm"));
    m_rsWidget->xAxis->setRange(0,1400000);
    m_rsWidget->yAxis->setRangeMin(-2000);
    m_rsWidget->yAxis->setRangeMax(2000);
    m_rsWidget->yAxis->setRange(-m_rsZoomState*80,m_rsZoomState*80);
    m_rsWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_rsWidget->axisRect()->setRangeZoom(Qt::Horizontal);
    m_rsWidget->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_rsWidget->xAxis->setTickLabelFont(fontTickLabel);
    m_rsWidget->yAxis->setTickLabelFont(fontTickLabel);
    m_rsWidget->xAxis->setLabelFont(fontLabel);
    m_rsWidget->yAxis->setLabelFont(fontLabel);
    m_rsWidget->replot();

    //-------RParallel Widget------------------------------------------------
    m_rpWidget->setAutoAddPlottableToLegend(false);
    m_rpWidget->addGraph();//graph(0)
    setBands(m_rpWidget, bands, -2000, 2000);
    m_rpWidget->graph(0)->setPen(pen);
    m_rpWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_rpWidget->yAxis->setLabel(tr("Rp, Ohm"));
    m_rpWidget->xAxis->setRange(0,1400000);
    m_rpWidget->yAxis->setRangeMin(-2000);
    m_rpWidget->yAxis->setRangeMax(2000);
    m_rpWidget->yAxis->setRange(-m_rpZoomState*80,m_rpZoomState*80);
    m_rpWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_rpWidget->axisRect()->setRangeZoom(Qt::Horizontal);
    m_rpWidget->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_rpWidget->xAxis->setTickLabelFont(fontTickLabel);
    m_rpWidget->yAxis->setTickLabelFont(fontTickLabel);
    m_rpWidget->xAxis->setLabelFont(fontLabel);
    m_rpWidget->yAxis->setLabelFont(fontLabel);
    m_rpWidget->replot();

    //-------RL Widget---------------------------------------------
    m_rlWidget->addGraph();//graph(0)
    setBands(m_rlWidget, bands, 0, 50);
    m_rlWidget->graph(0)->setPen(pen);
    m_rlWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_rlWidget->yAxis->setLabel(tr("RL, dB"));
    m_rlWidget->xAxis->setRange(0,1400000);
    m_rlWidget->yAxis->setRangeMin(0);
    m_rlWidget->yAxis->setRangeMax(50);
    m_rlWidget->yAxis->setRange(0,m_rlZoomState*5);
    m_rlWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_rlWidget->axisRect()->setRangeZoom(Qt::Horizontal);
    m_rlWidget->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_rlWidget->xAxis->setTickLabelFont(fontTickLabel);
    m_rlWidget->yAxis->setTickLabelFont(fontTickLabel);
    m_rlWidget->xAxis->setLabelFont(fontLabel);
    m_rlWidget->yAxis->setLabelFont(fontLabel);
    m_rlWidget->replot();

    //-------TDR Widget------------------------------------------------
    m_tdrWidget->setAutoAddPlottableToLegend(false);
    m_tdrWidget->addGraph();//graph(0)
    m_tdrWidget->graph(0)->setPen(pen);
    m_tdrWidget->xAxis->setLabel(tr("Length, m"));
    m_tdrWidget->yAxis->setRangeMin(-1);
    m_tdrWidget->yAxis->setRangeMax(1);
    m_tdrWidget->yAxis->setRange(-1,1);
    m_tdrWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_tdrWidget->axisRect()->setRangeZoom(Qt::Horizontal);
    m_tdrWidget->axisRect()->setRangeDrag(Qt::Horizontal);
    m_tdrWidget->xAxis->setTickLabelFont(fontTickLabel);
    m_tdrWidget->yAxis->setTickLabelFont(fontTickLabel);
    m_tdrWidget->xAxis->setLabelFont(fontLabel);
    m_tdrWidget->yAxis->setLabelFont(fontLabel);
    m_tdrWidget->replot();

    //-------Smith Widget---------------------------------------------
    m_smithWidget->addGraph();//graph(0)
    m_smithWidget->xAxis->setRangeMin(-10);
    m_smithWidget->xAxis->setRangeMax(10);
    m_smithWidget->xAxis->setRange(-7,7);
    m_smithWidget->yAxis->setRangeMin(-10);
    m_smithWidget->yAxis->setRangeMax(10);
    m_smithWidget->yAxis->setRange(-7,7);
    m_tdrWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_smithWidget->replot();
}

void MainWindow::setBands(QCustomPlot * widget, double y1, double y2)
{
    addBand(widget,135.7, 137.8, y1, y2);
    addBand(widget,472, 479, y1, y2);
    addBand(widget,1800, 2000, y1, y2);
    addBand(widget,3500, 3800, y1, y2);
    addBand(widget,7000, 7300, y1, y2);
    addBand(widget,10100, 10150, y1, y2);
    addBand(widget,14000, 14350, y1, y2);
    addBand(widget,18068, 18168, y1, y2);
    addBand(widget,21000, 21450, y1, y2);
    addBand(widget,24890, 24990, y1, y2);
    addBand(widget,27075, 27295, y1, y2);
    addBand(widget,28000, 29700, y1, y2);
    addBand(widget,50000, 54000, y1, y2);
    addBand(widget,144000, 148000, y1, y2);
    addBand(widget,220000, 225000, y1, y2);
    addBand(widget,420000, 450000, y1, y2);
    addBand(widget,902000, 928000, y1, y2);
    addBand(widget,1240000, 1300000, y1, y2);
}

void MainWindow::setBands(QCustomPlot * widget, QStringList* bands, double y1, double y2)
{
    if (bands == nullptr)
    {
        setBands(widget, y1, y2);
        return;
    }
    foreach (QString str, *bands)
    {
        QStringList list = str.split(',');
        if (list.size() == 2)
        {
            addBand(widget, list[0].toDouble(), list[1].toDouble(), y1, y2);
        }
    }
}

void MainWindow::addBand (QCustomPlot * widget, double x1, double x2, double y1, double y2)
{
    QCPItemRect * xRectItem = new QCPItemRect( widget );
    m_itemRectList.append(xRectItem);

    xRectItem->setVisible          (true);
    xRectItem->setPen              (QPen(Qt::transparent));
    xRectItem->setBrush            (QBrush(QColor(50,50,150,50)));

    xRectItem->topLeft->setType(QCPItemPosition::ptPlotCoords);
    xRectItem->topLeft->setAxisRect( widget->xAxis->axisRect() );
    xRectItem->topLeft->setCoords( x1, y2 );

    xRectItem->bottomRight ->setType(QCPItemPosition::ptPlotCoords);
    xRectItem->bottomRight ->setAxisRect( widget->xAxis->axisRect() );
    xRectItem->bottomRight ->setCoords( x2, y1 );
}

void MainWindow::on_pressF1 ()
{
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::on_pressF2 ()
{
    ui->tabWidget->setCurrentIndex(1);
}

void MainWindow::on_pressF3 ()
{
    ui->tabWidget->setCurrentIndex(2);
}

void MainWindow::on_pressF4 ()
{
    ui->tabWidget->setCurrentIndex(3);
}

void MainWindow::on_pressF5 ()
{
    ui->tabWidget->setCurrentIndex(4);
}

void MainWindow::on_pressF6 ()
{
    ui->tabWidget->setCurrentIndex(5);
}

void MainWindow::on_pressF7 ()
{
    ui->tabWidget->setCurrentIndex(6);
}

void MainWindow::on_pressEsc ()
{
    ui->singleStart->setChecked(false);
    ui->continuousStartBtn->setChecked(false);
    m_bInterrupted = true;
    emit stopMeasure();
}

void MainWindow::on_pressF9 ()
{
    emit on_singleStart_clicked();
}

void MainWindow::on_pressF10 ()
{
    emit on_continuousStartBtn_clicked(!ui->continuousStartBtn->isChecked());
    ui->continuousStartBtn->setChecked(!ui->continuousStartBtn->isChecked());
}

void MainWindow::on_pressDelete ()
{
    emit on_measurmentsDeleteBtn_clicked();
}

void MainWindow::on_pressPlus ()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_1")
    {
        double from = m_swrWidget->xAxis->getRangeLower();
        double to = m_swrWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band -= band/10;
        double center = (from + to)/2;

        if((center - band) > ABSOLUTE_MIN_FQ)
        {
            from = center - band;
        }else
        {
            from = ABSOLUTE_MIN_FQ;
        }
        m_swrWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_swrWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_phaseWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_swrWidget->replot();
    }else if(str == "tab_2")
    {
        double from = m_phaseWidget->xAxis->getRangeLower();
        double to = m_phaseWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band -= band/10;
        double center = (from + to)/2;

        if((center - band) > ABSOLUTE_MIN_FQ)
        {
            from = center - band;
        }else
        {
            from = ABSOLUTE_MIN_FQ;
        }
        m_phaseWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_phaseWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_phaseWidget->replot();
    }else if(str == "tab_3")
    {
        double from = m_rsWidget->xAxis->getRangeLower();
        double to = m_rsWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band -= band/10;
        double center = (from + to)/2;

        if((center - band) > ABSOLUTE_MIN_FQ)
        {
            from = center - band;
        }else
        {
            from = ABSOLUTE_MIN_FQ;
        }
        m_rsWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_rsWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rsWidget->replot();
    }else if(str == "tab_4")
    {
        double from = m_rpWidget->xAxis->getRangeLower();
        double to = m_rpWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band -= band/10;
        double center = (from + to)/2;

        if((center - band) > ABSOLUTE_MIN_FQ)
        {
            from = center - band;
        }else
        {
            from = ABSOLUTE_MIN_FQ;
        }
        m_rpWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_rpWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rpWidget->replot();
    }else if(str == "tab_5")
    {
        double from = m_rlWidget->xAxis->getRangeLower();
        double to = m_rlWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band -= band/10;
        double center = (from + to)/2;

        if((center - band) > ABSOLUTE_MIN_FQ)
        {
            from = center - band;
        }else
        {
            from = ABSOLUTE_MIN_FQ;
        }
        m_rlWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_rlWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rlWidget->replot();
    }else if(str == "tab_6")
    {
        double from = m_tdrWidget->xAxis->getRangeLower();
        double to = m_tdrWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band -= band/10;
        double center = (from + to)/2;

        if((center - band) > 0)
        {
            m_tdrWidget->xAxis->setRangeLower(center - band);
        }
        if ((center + band) < 10000)
        {
            m_tdrWidget->xAxis->setRangeUpper(center + band);
        }
        m_tdrWidget->replot();
    }
}

void MainWindow::on_pressCtrlPlus ()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_1")
    {
        if(m_swrZoomState > 1)
        {
            --m_swrZoomState;
            m_swrWidget->yAxis->setRangeUpper(m_swrZoomState+0.2);
            m_swrWidget->yAxis->setRangeLower(1);
            m_swrWidget->replot();
            if(m_markers)
            {
                QTimer::singleShot(5, m_markers, SLOT(redraw()));
            }
            if(m_measurements)
            {
                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
            }
        }
    }else if(str == "tab_2")
    {
    }else if(str == "tab_3")
    {
        if(m_rsZoomState > 1)
        {
            --m_rsZoomState;
            int val = m_rsZoomState*80;
            m_rsWidget->yAxis->setRangeLower(-val);
            m_rsWidget->yAxis->setRangeUpper(val);
            m_rsWidget->replot();
            if(m_markers)
            {
                QTimer::singleShot(5, m_markers, SLOT(redraw()));
            }
            if(m_measurements)
            {
                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
            }
        }
    }else if(str == "tab_4")
    {
        if(m_rpZoomState > 1)
        {
            --m_rpZoomState;
            int val = m_rpZoomState*80;
            m_rpWidget->yAxis->setRangeLower(-val);
            m_rpWidget->yAxis->setRangeUpper(val);
            m_rpWidget->replot();
            if(m_markers)
            {
                QTimer::singleShot(5, m_markers, SLOT(redraw()));
            }
            if(m_measurements)
            {
                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
            }
        }
    }else if(str == "tab_5")
    {
        if(m_rlZoomState > 1)
        {
            --m_rlZoomState;
            m_rlWidget->yAxis->setRangeUpper(m_rlZoomState*5);
            m_rlWidget->yAxis->setRangeLower(0);
            if(m_markers)
            {
                QTimer::singleShot(5, m_markers, SLOT(redraw()));
            }
            if(m_measurements)
            {
                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
            }
        }
    }else if(str == "tab_6")
    {
        m_tdrWidget->replot();
    }
}

void MainWindow::on_pressMinus ()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_1")
    {
        double from = m_swrWidget->xAxis->getRangeLower();
        double to = m_swrWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band += band/10;
        double center = (from + to)/2;

        if((center - band) > ABSOLUTE_MIN_FQ)
        {
            from = center - band;
        }else
        {
            from = ABSOLUTE_MIN_FQ;
        }
        m_swrWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_swrWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_phaseWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_swrWidget->replot();
    }else if(str == "tab_2")
    {
        double from = m_phaseWidget->xAxis->getRangeLower();
        double to = m_phaseWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band += band/10;
        double center = (from + to)/2;

        if((center - band) > ABSOLUTE_MIN_FQ)
        {
            from = center - band;
        }else
        {
            from = ABSOLUTE_MIN_FQ;
        }
        m_phaseWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_phaseWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_phaseWidget->replot();
    }else if(str == "tab_3")
    {
        double from = m_rsWidget->xAxis->getRangeLower();
        double to = m_rsWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band += band/10;
        double center = (from + to)/2;

        if((center - band) > ABSOLUTE_MIN_FQ)
        {
            from = center - band;
        }else
        {
            from = ABSOLUTE_MIN_FQ;
        }
        m_rsWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_rsWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rsWidget->replot();
    }else if(str == "tab_4")
    {
        double from = m_rpWidget->xAxis->getRangeLower();
        double to = m_rpWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band += band/10;
        double center = (from + to)/2;

        if((center - band) > ABSOLUTE_MIN_FQ)
        {
            from = center - band;
        }else
        {
            from = ABSOLUTE_MIN_FQ;
        }
        m_rpWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_rpWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rpWidget->replot();
    }else if(str == "tab_5")
    {
        double from = m_rlWidget->xAxis->getRangeLower();
        double to = m_rlWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band += band/10;
        double center = (from + to)/2;

        if((center - band) > ABSOLUTE_MIN_FQ)
        {
            from = center - band;
        }else
        {
            from = ABSOLUTE_MIN_FQ;
        }
        m_rlWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_rlWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rlWidget->replot();
    }else if(str == "tab_6")
    {
        double from = m_tdrWidget->xAxis->getRangeLower();
        double to = m_tdrWidget->xAxis->getRangeUpper();
        double band = (to - from)/2;
        band += band/10;
        double center = (from + to)/2;

        if((center - band) > 0)
        {
            m_tdrWidget->xAxis->setRangeLower(center - band);
        }
        if ((center + band) < 10000)
        {
            m_tdrWidget->xAxis->setRangeUpper(center + band);
        }
        m_tdrWidget->replot();
    }
}

void MainWindow::on_pressCtrlMinus ()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_1")
    {
        if(m_swrZoomState <= 9)
        {
            ++m_swrZoomState;
            m_swrWidget->yAxis->setRangeUpper(m_swrZoomState+0.2);
            m_swrWidget->yAxis->setRangeLower(1);
            m_swrWidget->replot();
            if(m_markers)
            {
                QTimer::singleShot(5, m_markers, SLOT(redraw()));
            }
            if(m_measurements)
            {
                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
            }
        }
    }else if(str == "tab_2")
    {
//        m_phaseWidget->replot();
    }else if(str == "tab_3")
    {
        if(m_rsZoomState < 19)
        {
            ++m_rsZoomState;
            int val = m_rsZoomState*80;
            m_rsWidget->yAxis->setRangeLower(-val);
            m_rsWidget->yAxis->setRangeUpper(val);
            m_rsWidget->replot();
            if(m_markers)
            {
                QTimer::singleShot(5, m_markers, SLOT(redraw()));
            }
            if(m_measurements)
            {
                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
            }
        }
    }else if(str == "tab_4")
    {
        if(m_rpZoomState < 19)
        {
            ++m_rpZoomState;
            int val = m_rpZoomState*80;
            m_rpWidget->yAxis->setRangeLower(-val);
            m_rpWidget->yAxis->setRangeUpper(val);
            m_rpWidget->replot();
            if(m_markers)
            {
                QTimer::singleShot(5, m_markers, SLOT(redraw()));
            }
            if(m_measurements)
            {
                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
            }
        }
    }else if(str == "tab_5")
    {
        if(m_rlZoomState <= 9)
        {
            ++m_rlZoomState;
            m_rlWidget->yAxis->setRangeUpper(m_rlZoomState*5);
            m_rlWidget->yAxis->setRangeLower(0);
            m_rlWidget->replot();
            if(m_markers)
            {
                QTimer::singleShot(5, m_markers, SLOT(redraw()));
            }
            if(m_measurements)
            {
                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
            }
        }
    }else if(str == "tab_6")
    {
        m_tdrWidget->replot();
    }
}

void MainWindow::on_pressCtrlZero()
{
    m_swrZoomState = 1;
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_1")
    {
        m_swrWidget->yAxis->setRangeUpper(m_swrZoomState+0.2);
        m_swrWidget->yAxis->setRangeLower(1);
        m_swrWidget->replot();
        if(m_markers)
        {
            QTimer::singleShot(5, m_markers, SLOT(redraw()));
        }
        if(m_measurements)
        {
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
    }else if(str == "tab_2")
    {
//        m_phaseWidget->replot();
    }else if(str == "tab_3")
    {
        int val = m_rsZoomState*80;
        m_rsWidget->yAxis->setRangeLower(-val);
        m_rsWidget->yAxis->setRangeUpper(val);
        m_rsWidget->replot();
        if(m_markers)
        {
            QTimer::singleShot(5, m_markers, SLOT(redraw()));
        }
        if(m_measurements)
        {
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
    }else if(str == "tab_4")
    {
        int val = m_rpZoomState*80;
        m_rpWidget->yAxis->setRangeLower(-val);
        m_rpWidget->yAxis->setRangeUpper(val);
        m_rpWidget->replot();
        if(m_markers)
        {
            QTimer::singleShot(5, m_markers, SLOT(redraw()));
        }
        if(m_measurements)
        {
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
    }else if(str == "tab_5")
    {
        m_rlWidget->yAxis->setRangeUpper(m_rlZoomState*5);
        m_rlWidget->yAxis->setRangeLower(0);
        m_rlWidget->replot();
        if(m_markers)
        {
            QTimer::singleShot(5, m_markers, SLOT(redraw()));
        }
        if(m_measurements)
        {
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
    }else if(str == "tab_6")
    {
        m_tdrWidget->replot();
    }
}

void MainWindow::on_pressLeft()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_1")
    {
        double from = m_swrWidget->xAxis->getRangeLower();
        double to = m_swrWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((from - diff) >= ABSOLUTE_MIN_FQ)
        {
            from -= diff;
            to -= diff;
            m_swrWidget->xAxis->setRangeLower(from);
            m_swrWidget->xAxis->setRangeUpper(to);
        }else
        {
            from = ABSOLUTE_MIN_FQ;
            m_swrWidget->xAxis->setRangeLower(from);
        }

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_phaseWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_swrWidget->replot();
    }else if(str == "tab_2")
    {
        double from = m_phaseWidget->xAxis->getRangeLower();
        double to = m_phaseWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((from - diff) >= ABSOLUTE_MIN_FQ)
        {
            from -= diff;
            to -= diff;
            m_phaseWidget->xAxis->setRangeLower(from);
            m_phaseWidget->xAxis->setRangeUpper(to);
        }else
        {
            from = ABSOLUTE_MIN_FQ;
            m_phaseWidget->xAxis->setRangeLower(from);
        }

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_phaseWidget->replot();
    }else if(str == "tab_3")
    {
        double from = m_rsWidget->xAxis->getRangeLower();
        double to = m_rsWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((from - diff) >= ABSOLUTE_MIN_FQ)
        {
            from -= diff;
            to -= diff;
            m_rsWidget->xAxis->setRangeLower(from);
            m_rsWidget->xAxis->setRangeUpper(to);
        }else
        {
            from = ABSOLUTE_MIN_FQ;
            m_rsWidget->xAxis->setRangeLower(from);
        }

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rsWidget->replot();
    }else if(str == "tab_4")
    {
        double from = m_rpWidget->xAxis->getRangeLower();
        double to = m_rpWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((from - diff) >= ABSOLUTE_MIN_FQ)
        {
            from -= diff;
            to -= diff;
            m_rpWidget->xAxis->setRangeLower(from);
            m_rpWidget->xAxis->setRangeUpper(to);
        }else
        {
            from = ABSOLUTE_MIN_FQ;
            m_rpWidget->xAxis->setRangeLower(from);
        }

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rpWidget->replot();
    }else if(str == "tab_5")
    {
        double from = m_rlWidget->xAxis->getRangeLower();
        double to = m_rlWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((from - diff) >= ABSOLUTE_MIN_FQ)
        {
            from -= diff;
            to -= diff;
            m_rlWidget->xAxis->setRangeLower(from);
            m_rlWidget->xAxis->setRangeUpper(to);
        }else
        {
            from = ABSOLUTE_MIN_FQ;
            m_rlWidget->xAxis->setRangeLower(from);
        }

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rlWidget->replot();
    }else if(str == "tab_6")
    {
        double from = m_tdrWidget->xAxis->getRangeLower();
        double to = m_tdrWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((from - diff) >= 0)
        {
            m_tdrWidget->xAxis->setRangeLower(from - diff);
            m_tdrWidget->xAxis->setRangeUpper(to - diff);
        }else
        {
            m_tdrWidget->xAxis->setRangeLower(0);
        }
        m_tdrWidget->replot();
    }
}

void MainWindow::on_pressRight()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_1")
    {
        double from = m_swrWidget->xAxis->getRangeLower();
        double to = m_swrWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((to + diff) <= ABSOLUTE_MAX_FQ)
        {
            from += diff;
            to += diff;
            m_swrWidget->xAxis->setRangeLower(from);
            m_swrWidget->xAxis->setRangeUpper(to);
        }else
        {
            to = ABSOLUTE_MAX_FQ;
            m_swrWidget->xAxis->setRangeUpper(to);
        }

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_phaseWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_swrWidget->replot();
    }else if(str == "tab_2")
    {
        double from = m_phaseWidget->xAxis->getRangeLower();
        double to = m_phaseWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((to + diff) <= ABSOLUTE_MAX_FQ)
        {
            from += diff;
            to += diff;
            m_phaseWidget->xAxis->setRangeLower(from);
            m_phaseWidget->xAxis->setRangeUpper(to);
        }else
        {
            to = ABSOLUTE_MAX_FQ;
            m_phaseWidget->xAxis->setRangeUpper(to);
        }

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_phaseWidget->replot();
    }else if(str == "tab_3")
    {
        double from = m_rsWidget->xAxis->getRangeLower();
        double to = m_rsWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((to + diff) <= ABSOLUTE_MAX_FQ)
        {
            from += diff;
            to += diff;
            m_rsWidget->xAxis->setRangeLower(from);
            m_rsWidget->xAxis->setRangeUpper(to);
        }else
        {
            to = ABSOLUTE_MAX_FQ;
            m_rsWidget->xAxis->setRangeUpper(to);
        }

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rsWidget->replot();
    }else if(str == "tab_4")
    {
        double from = m_rpWidget->xAxis->getRangeLower();
        double to = m_rpWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((to + diff) <= ABSOLUTE_MAX_FQ)
        {
            from += diff;
            to += diff;
            m_rpWidget->xAxis->setRangeLower(from);
            m_rpWidget->xAxis->setRangeUpper(to);
        }else
        {
            to = ABSOLUTE_MAX_FQ;
            m_rpWidget->xAxis->setRangeUpper(to);
        }

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rpWidget->replot();
    }else if(str == "tab_5")
    {
        double from = m_rlWidget->xAxis->getRangeLower();
        double to = m_rlWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((to + diff) <= ABSOLUTE_MAX_FQ)
        {
            from += diff;
            to += diff;
            m_rlWidget->xAxis->setRangeLower(from);
            m_rlWidget->xAxis->setRangeUpper(to);
        }else
        {
            to = ABSOLUTE_MAX_FQ;
            m_rlWidget->xAxis->setRangeUpper(to);
        }

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rlWidget->replot();
    }else if(str == "tab_6")
    {
        double from = m_tdrWidget->xAxis->getRangeLower();
        double to = m_tdrWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((to + diff) <= 10000)
        {
            m_tdrWidget->xAxis->setRangeLower(from + diff);
            m_tdrWidget->xAxis->setRangeUpper(to + diff);
        }else
        {
            m_tdrWidget->xAxis->setRangeUpper(10000);
        }
        m_tdrWidget->replot();
    }
}

void MainWindow::on_pressCtrlC ()
{
    QCustomPlot* plot = nullptr;
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_1")
    {
        plot = m_swrWidget;
    }else if(str == "tab_2")
    {
        plot = m_phaseWidget;
    }else if(str == "tab_3")
    {
        plot = m_rsWidget;
    }else if(str == "tab_4")
    {
        plot = m_rpWidget;
    }else if(str == "tab_5")
    {
        plot = m_rlWidget;
    }else if(str == "tab_6")
    {
        plot = m_tdrWidget;
    }else if(str == "tab_7")
    {
        resizeWnd();
        plot = m_smithWidget;
    }

    QPixmap pixmap = plot->grab();
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->setPixmap(pixmap);
}

void MainWindow::on_analyzerFound(QString name)
{
    QString name1 = "AntScope2 v." + QString(ANTSCOPE2VER);
    setWindowTitle(name1 + " - " + name);
//    quint32 model = m_analyzer->getModel();

    ui->singleStart->setEnabled(true);
    ui->continuousStartBtn->setEnabled(true);
    ui->analyzerDataBtn->setEnabled(true);
    ui->screenshotAA->setEnabled(true);
}

void MainWindow::on_analyzerDisconnected()
{
    QString name = "AntScope2 v." + QString(ANTSCOPE2VER);
    name += tr(" - Analyzer not connected");
    setWindowTitle(name);
    ui->singleStart->setEnabled(false);
    ui->continuousStartBtn->setEnabled(false);
    ui->analyzerDataBtn->setEnabled(false);
    ui->screenshotAA->setEnabled(false);
}

void MainWindow::on_mouseWheel_swr(QWheelEvent * e)
{
    double from  = m_swrWidget->xAxis->getRangeLower();
    double to = m_swrWidget->xAxis->getRangeUpper();
    if(!m_isRange)
    {
        setFqFrom(from);
        setFqTo(to);
    }else
    {
        setFqFrom((to+from)/2);
        setFqTo((to-from)/2);
    }

    m_phaseWidget->xAxis->setRange(m_swrWidget->xAxis->range());
    m_rsWidget->xAxis->setRange(m_swrWidget->xAxis->range());
    m_rpWidget->xAxis->setRange(m_swrWidget->xAxis->range());
    m_rlWidget->xAxis->setRange(m_swrWidget->xAxis->range());

    if (e->modifiers() == Qt::ControlModifier)
    {
        if(m_measurements)
        {
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
        if(e->delta() < 0)
        {
            if(g_noRestrictScale || m_swrZoomState <= 9)
            {
                ++m_swrZoomState;
                m_swrWidget->yAxis->setRangeUpper(m_swrZoomState+0.2);
                m_swrWidget->yAxis->setRangeLower(1);
            }
        }else
        {
            if(m_swrZoomState > 1)
            {
                --m_swrZoomState;
                m_swrWidget->yAxis->setRangeUpper(m_swrZoomState+0.2);
                m_swrWidget->yAxis->setRangeLower(1);
            }
        }
        QTimer::singleShot(5, m_markers, SLOT(redraw()));
    }
    emit rescale();
}

void MainWindow::on_mouseMove_swr(QMouseEvent *e)
{
    m_isMouseClick = false;
    double x = m_swrWidget->xAxis->pixelToCoord(e->pos().x());
    double y = m_swrWidget->yAxis->pixelToCoord(e->pos().y());

    double from;
    double to;
    if(!m_isRange)
    {
        from = getFqFrom();
        to = getFqTo();
    }else
    {
        from = getFqFrom() - getFqTo();
        to = getFqFrom() + getFqTo();
    }
    if((x >= from) && (x <= to))
    {
        if(y >= m_swrWidget->yAxis->range().lower && y <= m_swrWidget->yAxis->range().upper)
        {
            QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();
            if(!list.isEmpty())
            {
                QTableWidgetItem * item = list.at(0);
                emit newCursorFq(x, item->row(), QCursor::pos().x(), QCursor::pos().y());
            }
        }
    }
    if (e->buttons() & Qt::LeftButton)
    {
        if(!m_isRange)
        {
            setFqFrom(getFqFrom());
            setFqTo(getFqTo());
        }else
        {
            setFqFrom((getFqTo() + getFqFrom())/2);
            setFqTo((getFqTo() - getFqFrom())/2);
        }

        m_phaseWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_swrWidget->xAxis->range());
    }
}

void MainWindow::on_mouseWheel_phase(QWheelEvent * e)
{
    Q_UNUSED(e);

    double from  = m_phaseWidget->xAxis->getRangeLower();
    double to = m_phaseWidget->xAxis->getRangeUpper();
    if(!m_isRange)
    {
        setFqFrom(from);
        setFqTo(to);
    }else
    {
        setFqFrom((from+to)/2);
        setFqTo((to-from)/2);
    }

    m_swrWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
    m_rsWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
    m_rpWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
    m_rlWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
    emit rescale();
}

void MainWindow::on_mouseMove_phase(QMouseEvent *e)
{
    m_isMouseClick = false;
    double x = m_phaseWidget->xAxis->pixelToCoord(e->pos().x());
    double y = m_phaseWidget->yAxis->pixelToCoord(e->pos().y());
    double from;
    double to;
    if(!m_isRange)
    {
        from = getFqFrom();
        to = getFqTo();
    }else
    {
        from = getFqFrom() - getFqTo();
        to = getFqFrom() + getFqTo();
    }
    if((x >= from) && (x <= to))
    {
        if(y >= m_phaseWidget->yAxis->range().lower && y <= m_phaseWidget->yAxis->range().upper)
        {
            QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();
            if(!list.isEmpty())
            {
                QTableWidgetItem * item = list.at(0);
                emit newCursorFq(x, item->row(), QCursor::pos().x(), QCursor::pos().y());
            }
        }
    }
    if (e->buttons() & Qt::LeftButton)
    {
        if(!m_isRange)
        {
            setFqFrom(getFqFrom());
            setFqTo(getFqTo());
        }else
        {
            setFqFrom((getFqTo() + getFqFrom())/2);
            setFqTo((getFqTo() - getFqFrom())/2);
        }
        m_swrWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
    }
}

void MainWindow::on_mouseWheel_rs(QWheelEvent * e)
{
    static int state = 1;
    double from  = m_rsWidget->xAxis->getRangeLower();
    double to = m_rsWidget->xAxis->getRangeUpper();
    if(!m_isRange)
    {
        setFqFrom(from);
        setFqTo(to);
    }else
    {
        setFqFrom((from+to)/2);
        setFqTo((to-from)/2);
    }

    m_swrWidget->xAxis->setRange(m_rsWidget->xAxis->range());
    m_phaseWidget->xAxis->setRange(m_rsWidget->xAxis->range());
    m_rpWidget->xAxis->setRange(m_rsWidget->xAxis->range());
    m_rlWidget->xAxis->setRange(m_rsWidget->xAxis->range());

    if (e->modifiers() == Qt::ControlModifier)
    {
        int val;
        if(e->delta() < 0)
        {
            if(g_noRestrictScale || state <= 19)
            {
                ++state;
                val = state*80;
                m_rsWidget->yAxis->setRangeLower(-val);
                m_rsWidget->yAxis->setRangeUpper(val);
                m_rsWidget->replot();
                if(m_markers)
                {
                    QTimer::singleShot(5, m_markers, SLOT(redraw()));
                }
                if(m_measurements)
                {
                    QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
                }
            }
        }else
        {
            if(state > 1)
            {
                --state;
                val = state*80;
                m_rsWidget->yAxis->setRangeLower(-val);
                m_rsWidget->yAxis->setRangeUpper(val);
                m_rsWidget->replot();
                if(m_markers)
                {
                    QTimer::singleShot(5, m_markers, SLOT(redraw()));
                }
                if(m_measurements)
                {
                    QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
                }
            }
        }
    }
    emit rescale();
}

void MainWindow::on_mouseMove_rs(QMouseEvent *e)
{
    m_isMouseClick = false;
    double x = m_rsWidget->xAxis->pixelToCoord(e->pos().x());
    double y = m_rsWidget->yAxis->pixelToCoord(e->pos().y());
    int from;
    int to;
    if(!m_isRange)
    {
        from = getFqFrom();
        to = getFqTo();
    }else
    {
        from = getFqFrom() - getFqTo();
        to = getFqFrom() + getFqTo();
    }
    if((x >= from) && (x <= to))
    {
        if(y >= m_rsWidget->yAxis->range().lower && y <= m_rsWidget->yAxis->range().upper)
        {
            QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();
            if(!list.isEmpty())
            {
                QTableWidgetItem * item = list.at(0);
                emit newCursorFq(x, item->row(), QCursor::pos().x(), QCursor::pos().y());
            }
        }
    }
    if (e->buttons() & Qt::LeftButton)
    {
        if(!m_isRange)
        {
            setFqFrom(getFqFrom());
            setFqTo(getFqTo());
        }else
        {
            setFqFrom((getFqTo() + getFqFrom())/2);
            setFqTo((getFqTo() - getFqFrom())/2);
        }
        m_swrWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rsWidget->xAxis->range());
    }
}

void MainWindow::on_mouseWheel_rp(QWheelEvent *e)
{
    static quint32 state = 1;
    double from  = m_rpWidget->xAxis->getRangeLower();
    double to = m_rpWidget->xAxis->getRangeUpper();
    if(!m_isRange)
    {
        setFqFrom(from);
        setFqTo(to);
    }else
    {
        setFqFrom((from+to)/2);
        setFqTo((to-from)/2);
    }

    m_swrWidget->xAxis->setRange(m_rpWidget->xAxis->range());
    m_phaseWidget->xAxis->setRange(m_rpWidget->xAxis->range());
    m_rsWidget->xAxis->setRange(m_rpWidget->xAxis->range());
    m_rlWidget->xAxis->setRange(m_rpWidget->xAxis->range());

    if (e->modifiers() == Qt::ControlModifier)
    {
        if(e->delta() < 0)
        {
            if(g_noRestrictScale || state <= 19)
            {
                ++state;
                int val = state*80;
                m_rpWidget->yAxis->setRangeLower(-val);
                m_rpWidget->yAxis->setRangeUpper(val);
                m_rpWidget->replot();
                if(m_markers)
                {
                    QTimer::singleShot(5, m_markers, SLOT(redraw()));
                }
                if(m_measurements)
                {
                    QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
                }
            }
        }else
        {
            if(state > 1)
            {
                --state;
                int val = state*80;
                m_rpWidget->yAxis->setRangeLower(-val);
                m_rpWidget->yAxis->setRangeUpper(val);
                m_rpWidget->replot();
                if(m_markers)
                {
                    QTimer::singleShot(5, m_markers, SLOT(redraw()));
                }
                if(m_measurements)
                {
                    QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
                }
            }
        }
    }
    emit rescale();
}

void MainWindow::on_mouseMove_rp(QMouseEvent *e)
{
    m_isMouseClick = false;
    double x = m_rpWidget->xAxis->pixelToCoord(e->pos().x());
    double y = m_rpWidget->yAxis->pixelToCoord(e->pos().y());
    double from;
    double to;
    if(!m_isRange)
    {
        from = getFqFrom();
        to = getFqTo();
    }else
    {
        from = getFqFrom() - getFqTo();
        to = getFqFrom() + getFqTo();
    }
    if((x >= from) && (x <= to))
    {
        if(y >= m_rpWidget->yAxis->range().lower && y <= m_rpWidget->yAxis->range().upper)
        {
            QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();
            if(!list.isEmpty())
            {
                QTableWidgetItem * item = list.at(0);
                emit newCursorFq(x, item->row(), QCursor::pos().x(), QCursor::pos().y());
            }
        }
    }
    if (e->buttons() & Qt::LeftButton)
    {
        if(!m_isRange)
        {
            setFqFrom(getFqFrom());
            setFqTo(getFqTo());
        }else
        {
            setFqFrom((getFqTo() + getFqFrom())/2);
            setFqTo((getFqTo() - getFqFrom())/2);
        }
        m_swrWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rpWidget->xAxis->range());
    }
}

void MainWindow::on_mouseWheel_rl(QWheelEvent *e)
{
    static int state = 10;
    double from  = m_rlWidget->xAxis->getRangeLower();
    double to = m_rlWidget->xAxis->getRangeUpper();
    if(!m_isRange)
    {
        setFqFrom(from);
        setFqTo(to);
    }else
    {
        setFqFrom((from+to)/2);
        setFqTo((to-from)/2);
    }

    m_swrWidget->xAxis->setRange(m_rlWidget->xAxis->range());
    m_phaseWidget->xAxis->setRange(m_rlWidget->xAxis->range());
    m_rsWidget->xAxis->setRange(m_rlWidget->xAxis->range());
    m_rpWidget->xAxis->setRange(m_rlWidget->xAxis->range());

    if (e->modifiers() == Qt::ControlModifier)
    {
        if(m_measurements)
        {
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
        if(e->delta() < 0)
        {
            if(g_noRestrictScale || state <= 9)
            {
                ++state;
                m_rlWidget->yAxis->setRangeUpper(state*5);
                m_rlWidget->yAxis->setRangeLower(0);
                m_rlWidget->replot();
            }
        }else
        {
            if(state > 1)
            {
                --state;
                m_rlWidget->yAxis->setRangeUpper(state*5);
                m_rlWidget->yAxis->setRangeLower(0);
                m_rlWidget->replot();
            }
        }
        QTimer::singleShot(5, m_markers, SLOT(redraw()));
    }
    emit rescale();
}

void MainWindow::on_mouseMove_rl(QMouseEvent *e)
{
    m_isMouseClick = false;
    double x = m_rlWidget->xAxis->pixelToCoord(e->pos().x());
    double y = m_rlWidget->yAxis->pixelToCoord(e->pos().y());
    double from;
    double to;
    if(!m_isRange)
    {
        from = getFqFrom();
        to = getFqTo();
    }else
    {
        from = getFqFrom() - getFqTo();
        to = getFqFrom() + getFqTo();
    }
    if((x >= from) && (x <= to))
    {
        if(y >= m_rlWidget->yAxis->range().lower && y <= m_rlWidget->yAxis->range().upper)
        {
            QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();
            if(!list.isEmpty())
            {
                QTableWidgetItem * item = list.at(0);
                emit newCursorFq(x, item->row(), QCursor::pos().x(), QCursor::pos().y());
            }
        }
    }
    if (e->buttons() & Qt::LeftButton)
    {
        if(!m_isRange)
        {
            setFqFrom(getFqFrom());
            setFqTo(getFqTo());
        }else
        {
            setFqFrom((getFqTo() + getFqFrom())/2);
            setFqTo((getFqTo() - getFqFrom())/2);
        }
        m_swrWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_rlWidget->xAxis->range());
    }
}

void MainWindow::on_mouseWheel_tdr(QWheelEvent* e)
{
    static int state = 10;
    if (e->modifiers() == Qt::ControlModifier)
    {
        if(m_measurements)
        {
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
        double up = m_tdrWidget->yAxis->getRangeUpper();
        double lo = m_tdrWidget->yAxis->getRangeLower();

        double diff = (up - lo)*0.1;
        if(e->delta() < 0)
        {
            //if(state <= 9)
            {
                diff *= -1;
                ++state;
//                m_tdrWidget->yAxis->setRangeUpper(up + up*0.1 );
//                m_tdrWidget->yAxis->setRangeLower(lo - lo*0.1);
//                m_tdrWidget->replot();
            }
        } else {
            //if(state > 1)
            {
                --state;
//                m_tdrWidget->yAxis->setRangeUpper(up - up*0.1 );
//                m_tdrWidget->yAxis->setRangeLower(lo + lo*0.1);
//                m_tdrWidget->replot();
            }
        }
        m_tdrWidget->yAxis->moveRange(diff);
        m_tdrWidget->replot();
        QTimer::singleShot(5, m_markers, SLOT(redraw()));
    }
    emit rescale();
}

void MainWindow::on_mouseMove_tdr(QMouseEvent * e)
{
    double x = m_tdrWidget->xAxis->pixelToCoord(e->pos().x());
    if( (x >= m_tdrWidget->xAxis->range().lower) && (x <= m_tdrWidget->xAxis->range().upper))
    {
        QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();
        if(!list.isEmpty())
        {
            QTableWidgetItem * item = list.at(0);
            emit newCursorFq(x, item->row(), e->pos().x(), e->pos().y());
        }
    }
}

void MainWindow::on_mouseMove_smith(QMouseEvent * e)
{
    double x = m_smithWidget->xAxis->pixelToCoord(e->pos().x());
    double y = m_smithWidget->yAxis->pixelToCoord(e->pos().y());
    QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();
    if(!list.isEmpty())
    {
        QTableWidgetItem * item = list.at(0);
        emit newCursorSmithPos( x, y, item->row());
    }
}

void MainWindow::createTabs (QString sequence)
{
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    for(quint32 i = 0; i < 7; ++i)
    {
        int byte = sequence.indexOf(QString::number(i));
        switch(byte)
        {
        case 0:
            m_tab_1 = new QWidget();
            m_tab_1->setObjectName(QStringLiteral("tab_1"));
            m_horizontalLayout_1 = new QHBoxLayout(m_tab_1);
            m_horizontalLayout_1->setSpacing(6);
            m_horizontalLayout_1->setContentsMargins(11, 11, 11, 11);
            m_horizontalLayout_1->setObjectName(QStringLiteral("horizontalLayout_1"));
            m_swrWidget = new QCustomPlot(m_tab_1);
            m_swrWidget->setObjectName(QStringLiteral("swr_widget"));
            //m_swrWidget->yAxis->setScaleType(QCPAxis::stLogarithmic);

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_swrWidget->sizePolicy().hasHeightForWidth());
            m_swrWidget->setSizePolicy(sizePolicy);
            m_horizontalLayout_1->addWidget(m_swrWidget);
            ui->tabWidget->addTab(m_tab_1, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_1), QApplication::translate("MainWindow", "SWR", 0));
            m_mapWidgets.insert(QStringLiteral("swr_widget"), m_swrWidget);
            break;
        case 1:
            m_tab_2 = new QWidget();
            m_tab_2->setObjectName(QStringLiteral("tab_2"));

            m_horizontalLayout_2 = new QHBoxLayout(m_tab_2);
            m_horizontalLayout_2->setSpacing(6);
            m_horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
            m_horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
            m_phaseWidget = new QCustomPlot(m_tab_2);
            m_phaseWidget->setObjectName(QStringLiteral("phase_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_phaseWidget->sizePolicy().hasHeightForWidth());
            m_phaseWidget->setSizePolicy(sizePolicy);
            m_horizontalLayout_2->addWidget(m_phaseWidget);

            ui->tabWidget->addTab(m_tab_2, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_2), QApplication::translate("MainWindow", "Phase", 0));
            m_mapWidgets.insert(QStringLiteral("phase_widget"), m_phaseWidget);
            break;
        case 2:
            m_tab_3 = new QWidget();
            m_tab_3->setObjectName(QStringLiteral("tab_3"));

            m_horizontalLayout_3 = new QHBoxLayout(m_tab_3);
            m_horizontalLayout_3->setSpacing(6);
            m_horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
            m_horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
            m_rsWidget = new QCustomPlot(m_tab_3);
            m_rsWidget->setObjectName(QStringLiteral("rs_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_rsWidget->sizePolicy().hasHeightForWidth());
            m_rsWidget->setSizePolicy(sizePolicy);
            m_horizontalLayout_3->addWidget(m_rsWidget);

            ui->tabWidget->addTab(m_tab_3, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_3), QApplication::translate("MainWindow", "Z=R+jX", 0));
            m_mapWidgets.insert(QStringLiteral("rs_widget"), m_rsWidget);
            break;
        case 3:
            m_tab_4 = new QWidget();
            m_tab_4->setObjectName(QStringLiteral("tab_4"));

            m_horizontalLayout_4 = new QHBoxLayout(m_tab_4);
            m_horizontalLayout_4->setSpacing(6);
            m_horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
            m_horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
            m_rpWidget = new QCustomPlot(m_tab_4);
            m_rpWidget->setObjectName(QStringLiteral("rp_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_rpWidget->sizePolicy().hasHeightForWidth());
            m_rpWidget->setSizePolicy(sizePolicy);
            m_horizontalLayout_4->addWidget(m_rpWidget);

            ui->tabWidget->addTab(m_tab_4, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_4), QApplication::translate("MainWindow", "Z=R||+jX", 0));
            m_mapWidgets.insert(QStringLiteral("rp_widget"), m_rpWidget);
            break;
        case 4:
            m_tab_5 = new QWidget();
            m_tab_5->setObjectName(QStringLiteral("tab_5"));
            m_horizontalLayout_5 = new QHBoxLayout(m_tab_5);
            m_horizontalLayout_5->setSpacing(6);
            m_horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
            m_horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_2"));
            m_rlWidget = new QCustomPlot(m_tab_5);
            m_rlWidget->setObjectName(QStringLiteral("rl_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_rlWidget->sizePolicy().hasHeightForWidth());
            m_rlWidget->setSizePolicy(sizePolicy);
            m_horizontalLayout_5->addWidget(m_rlWidget);
            ui->tabWidget->addTab(m_tab_5, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_5), QApplication::translate("MainWindow", "RL", 0));
            m_mapWidgets.insert(QStringLiteral("rl_widget"), m_rlWidget);
            break;
        case 5:
            m_tab_6 = new QWidget();
            m_tab_6->setObjectName(QStringLiteral("tab_6"));

            m_horizontalLayout_6 = new QHBoxLayout(m_tab_6);
            m_horizontalLayout_6->setSpacing(6);
            m_horizontalLayout_6->setContentsMargins(11, 11, 11, 11);
            m_horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
            m_tdrWidget = new QCustomPlot(m_tab_6);
            m_tdrWidget->setObjectName(QStringLiteral("tdr_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_tdrWidget->sizePolicy().hasHeightForWidth());
            m_tdrWidget->setSizePolicy(sizePolicy);
            m_horizontalLayout_6->addWidget(m_tdrWidget);

            ui->tabWidget->addTab(m_tab_6, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_6), QApplication::translate("MainWindow", "TDR", 0));
            m_mapWidgets.insert(QStringLiteral("tdr_widget"), m_tdrWidget);
            break;
        case 6:
            m_tab_7 = new QWidget();
            m_tab_7->setObjectName(QStringLiteral("tab_7"));

            m_horizontalLayout_7 = new QHBoxLayout(m_tab_7);
            m_horizontalLayout_7->setSpacing(6);
            m_horizontalLayout_7->setContentsMargins(11, 11, 11, 11);
            m_horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
            m_smithWidget = new QCustomPlot(m_tab_7);
            m_smithWidget->setObjectName(QStringLiteral("smith_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_smithWidget->sizePolicy().hasHeightForWidth());
            m_smithWidget->setSizePolicy(sizePolicy);
            m_horizontalLayout_7->addWidget(m_smithWidget);

            ui->tabWidget->addTab(m_tab_7, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_7), QApplication::translate("MainWindow", "Smith", 0));

            m_smithWidget->xAxis->setTicks(false);
            m_smithWidget->yAxis->setTicks(false);
            m_smithWidget->xAxis->setVisible(false);
            m_smithWidget->yAxis->setVisible(false);
            m_mapWidgets.insert(QStringLiteral("smith_widget"), m_smithWidget);
            break;
        default:
            break;
        }
    }
}

void MainWindow::on_fqSettingsBtn_clicked()
{
    m_fqSettings = new FqSettings(this);
    m_fqSettings->setAttribute(Qt::WA_DeleteOnClose);
    m_fqSettings->setWindowTitle(tr("Frequency settings"));
    m_fqSettings->setDotsNumber(m_dotsNumber);
    connect(m_fqSettings, SIGNAL(dotsNumber(int)), this, SLOT(on_dotsNumberChanged(int)));
    connect(m_fqSettings, SIGNAL(dotsNumber(int)), m_measurements, SLOT(on_dotsNumberChanged(int)));
    m_fqSettings->exec();
}

void MainWindow::on_presetsAddBtn_clicked()
{
    QString from = QString::number(getFqFrom(),'f',0);
    QString to = QString::number(getFqTo(),'f',0);
    m_presets->addNewRow(from, to, QString::number(m_dotsNumber));
}

void MainWindow::on_tableWidget_presets_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    QStringList list = m_presets->getRow(row);
    QCPRange range;
    range.lower = list.at(0).toDouble();
    range.upper = list.at(1).toDouble();
    m_dotsNumber = list.at(2).toInt();
    ui->spinBoxPoints->setValue(m_dotsNumber);

    if(!m_isRange)
    {
        setFqFrom(list.at(0));
        setFqTo(list.at(1));
    }else
    {
        setFqFrom((list.at(1).toDouble() + list.at(0).toDouble())/2);
        setFqTo((list.at(1).toDouble() - list.at(0).toDouble())/2);
    }
    m_swrWidget->xAxis->setRange(range);
    m_phaseWidget->xAxis->setRange(range);
    m_rsWidget->xAxis->setRange(range);
    m_rpWidget->xAxis->setRange(range);
    m_rlWidget->xAxis->setRange(range);
    updateGraph();
}

void MainWindow::on_presetsDeleteBtn_clicked()
{
    QList <QTableWidgetItem *> list = ui->tableWidget_presets->selectedItems();
    if(list.length() > 0)
    {
        QTableWidgetItem * item = list.at(0);
        int rowNumber = item->row();
        m_presets->deleteRow(rowNumber);
    }
}

void MainWindow::on_pressetsUpBtn_clicked()
{
    QList <QTableWidgetItem *> list = ui->tableWidget_presets->selectedItems();
    if(list.length() > 0)
    {
        QTableWidgetItem * item = list.at(0);
        int rowNumber = item->row();
        m_presets->moveRowUp(rowNumber);
    }
}

void MainWindow::on_exportBtn_clicked()
{
    QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();
    if(!list.isEmpty())
    {
        QTableWidgetItem * item = list.at(0);
        m_exportDialog = new Export(this);
        m_exportDialog->setAttribute(Qt::WA_DeleteOnClose);
        m_exportDialog->setWindowTitle(tr("Export"));
        m_exportDialog->setMeasurements(m_measurements, item->row());
        m_exportDialog->exec();
    }
}

void MainWindow::on_analyzerDataBtn_clicked()
{
    m_analyzerData = new AnalyzerData(m_analyzer->getModel(), this);
    m_analyzerData->setAttribute(Qt::WA_DeleteOnClose);
    m_analyzer->getAnalyzerData();
    connect(m_analyzer,SIGNAL(analyzerDataStringArrived(QString)),m_analyzerData,SLOT(on_analyzerDataStringArrived(QString)));
    connect(m_analyzerData,SIGNAL(itemDoubleClick(QString,QString,QString)),m_analyzer,SLOT(on_itemDoubleClick(QString,QString,QString)));
    connect(m_analyzerData,SIGNAL(signalSaveFile(QString)),this,SLOT(on_SaveFile(QString)));
    connect(m_analyzerData, &AnalyzerData::dataChanged, this, &MainWindow::on_dataChanged);
    //connect(m_analyzerData,SIGNAL(dialogClosed()),m_analyzer,SLOT(on_dialogClosed()));
    m_analyzerData->exec();
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index)
    QString str = ui->tabWidget->currentWidget()->objectName();
    updateGraph();
    emit currentTab (str);    
    QTimer::singleShot(20, m_markers, SLOT(redraw()));
}

void MainWindow::on_screenshotAA_clicked()
{
    int wd = lcdWidth[m_analyzer->getModel()];
    int ht = lcdHeight[m_analyzer->getModel()];
    if (CustomAnalyzer::customized()) {
        CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
        if (ca != nullptr) {
            wd = ca->width();
            ht = ca->height();
        }
    }

    m_screenshot = new Screenshot(this, m_analyzer->getModel(), ht, wd);
    m_screenshot->setAttribute(Qt::WA_DeleteOnClose);
    m_screenshot->setWindowTitle("Screenshot");

    connect(m_analyzer,SIGNAL(analyzerScreenshotDataArrived(QByteArray)),m_screenshot,SLOT(on_newData(QByteArray)));
    connect(m_screenshot,SIGNAL(screenshotComplete()),m_analyzer,SLOT(on_screenshotComplete()));
    connect(m_screenshot,SIGNAL(newScreenshot()),m_analyzer,SLOT(makeScreenshot()));

    m_analyzer->makeScreenshot();

    m_screenshot->exec();
}

void MainWindow::on_singleStart_clicked()
{
    hidAnalyzer* hid = m_analyzer->getHidAnalyzer();

    bool use_min_max = isMeasuring();
    if (isMeasuring())
    {
        m_bInterrupted = true;
        emit stopMeasure();
        //ui->singleStart->setChecked(false);
        ui->singleStart->setChecked(false);
        ui->continuousStartBtn->setChecked(false);
        return;
    }

    ui->continuousStartBtn->setChecked(false);

    double start;
    double stop;

    qint64 minFreq = minFq[m_analyzer->getAnalyzerModel()].toULongLong();
    qint64 maxFreq = maxFq[m_analyzer->getAnalyzerModel()].toULongLong();

    if (CustomAnalyzer::customized()) {
        CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
        if (ca != nullptr) {
            minFreq = ca->minFq().toULongLong();
            maxFreq = ca->maxFq().toULongLong();
        }
    }

    if (use_min_max)
    {
        start = minFreq;
        stop = maxFreq;
    }
    else
    {
        start = getFqFrom();
        stop = getFqTo();
    }

    if(stop > static_cast<double>(maxFreq))
    {
        stop = maxFreq;
        if(!m_isRange)
        {
            setFqFrom(start);
            setFqTo(stop);
        }else
        {
            setFqTo((stop-start)/2);
            setFqFrom((stop+start)/2);
        }
    }
    if((start > static_cast<double>(maxFreq)) || (start < static_cast<double>(minFreq)))
    {
        start = minFreq;
        if(!m_isRange)
        {
            setFqFrom(start);
        }else
        {
            setFqTo((stop-start)/2);
            setFqFrom((stop+start)/2);
        }
    }
    QCPRange range(start, stop);
    m_swrWidget->xAxis->setRange(range);
    m_phaseWidget->xAxis->setRange(range);
    m_rsWidget->xAxis->setRange(range);
    m_rpWidget->xAxis->setRange(range);
    m_rlWidget->xAxis->setRange(range);

    m_settings->beginGroup("MainWindow");
    if (!m_isRange) {
        m_settings->setValue("rangeLower", start);
        m_settings->setValue("rangeUpper", stop);
    } else {
        m_settings->setValue("rangeLower", (stop-start)/2);
        m_settings->setValue("rangeUpper", (stop+start)/2);
    }
    m_settings->setValue("dotsNumber", m_dotsNumber);
    m_settings->endGroup();

    if(ui->tabWidget->currentWidget()->objectName() == "tab_6")
    {
        qint64 minFq_ = minFq[m_analyzer->getAnalyzerModel()].toULongLong()*1000;
        qint64 maxFq_ = maxFq[m_analyzer->getAnalyzerModel()].toULongLong()*1000;
        if (CustomAnalyzer::customized()) {
            CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
            if (ca != nullptr) {
                minFq_ = ca->minFq().toULongLong();
                maxFq_ = ca->maxFq().toULongLong();
            }
        }

        emit measure(minFq_, maxFq_, qMax(m_dotsNumber, 200));
    }
    else
    {
        emit measure(start*1000, stop*1000, m_dotsNumber);
    }
    ui->measurmentsSaveBtn->setEnabled(true);
    ui->exportBtn->setEnabled(true);
    ui->measurmentsDeleteBtn->setEnabled(!m_analyzer->isMeasuring());
    ui->measurmentsClearBtn->setEnabled(!m_analyzer->isMeasuring());
}

void MainWindow::on_continuousStartBtn_clicked(bool checked)
{
    hidAnalyzer* hid = m_analyzer->getHidAnalyzer();
    if (isMeasuring())
    {
        m_bInterrupted = true;
        emit stopMeasure();
        ui->singleStart->setChecked(false);
        ui->continuousStartBtn->setChecked(false);
        return;
    }

    ui->singleStart->setChecked(false);
    m_isContinuos = checked;
    m_analyzer->setContinuos(m_isContinuos);
    if(m_isContinuos)
    {
        m_bInterrupted = false;
        double start;
        double stop;

        start = getFqFrom();
        stop = getFqTo();

        qint64 minFreq = minFq[m_analyzer->getAnalyzerModel()].toULongLong();
        qint64 maxFreq = maxFq[m_analyzer->getAnalyzerModel()].toULongLong();
        if (CustomAnalyzer::customized()) {
            CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
            if (ca != nullptr) {
                minFreq = ca->minFq().toULongLong();
                maxFreq = ca->maxFq().toULongLong();
            }
        }

        if(stop > static_cast<double>(maxFreq))
        {
            stop = maxFreq;
            if(!m_isRange)
            {
                setFqTo(maxFreq);
            }else
            {
                setFqTo((stop-start)/2);
            }
        }
        if((start > static_cast<double>(maxFreq)) || (start < static_cast<double>(minFreq)))
        {
            start = minFreq;
            if(!m_isRange)
            {
                setFqFrom(start);
            }else
            {
                setFqFrom((stop+start)/2);
            }
        }
        QCPRange range(start, stop);
        m_swrWidget->xAxis->setRange(range);
        m_phaseWidget->xAxis->setRange(range);
        m_rsWidget->xAxis->setRange(range);
        m_rpWidget->xAxis->setRange(range);
        m_rlWidget->xAxis->setRange(range);

        m_settings->beginGroup("MainWindow");
        if (!m_isRange) {
            m_settings->setValue("rangeLower", start);
            m_settings->setValue("rangeUpper", stop);
        } else {
            m_settings->setValue("rangeLower", (stop-start)/2);
            m_settings->setValue("rangeUpper", (stop+start)/2);
        }
        m_settings->setValue("dotsNumber", m_dotsNumber);
        m_settings->endGroup();

        emit measure(start*1000, stop*1000, m_dotsNumber);
        ui->measurmentsSaveBtn->setEnabled(true);
        ui->exportBtn->setEnabled(true);
        ui->measurmentsDeleteBtn->setEnabled(false);
        ui->measurmentsClearBtn->setEnabled(false);
    }else
    {
        m_bInterrupted = true;
        m_analyzer->setContinuos(false);
    }
}

void MainWindow::on_measurementComplete()
{    
    QTimer::singleShot(5, m_markers, SLOT(redraw()));
    if(m_isContinuos)
    {
        double start;
        double stop;

        start = getFqFrom();
        stop = getFqTo();

        qint64 minFreq = minFq[m_analyzer->getAnalyzerModel()].toULongLong();
        qint64 maxFreq = maxFq[m_analyzer->getAnalyzerModel()].toULongLong();
        if (CustomAnalyzer::customized()) {
            CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
            if (ca != nullptr) {
                minFreq = ca->minFq().toULongLong();
                maxFreq = ca->maxFq().toULongLong();
            }
        }
        if(stop > static_cast<double>(maxFreq))
        {
            stop = maxFreq;
            if(!m_isRange)
            {
                setFqTo(maxFreq);
            }else
            {
                setFqTo((stop-start)/2);
            }
        }
        if((start > static_cast<double>(maxFreq)) || (start < static_cast<double>(minFreq)))
        {
            start = minFreq;
            if(!m_isRange)
            {
                setFqFrom(start);
            }else
            {
                setFqFrom((stop+start)/2);
            }
        }
        QCPRange range(start, stop);
        m_swrWidget->xAxis->setRange(range);
        m_phaseWidget->xAxis->setRange(range);
        m_rsWidget->xAxis->setRange(range);
        m_rpWidget->xAxis->setRange(range);
        m_rlWidget->xAxis->setRange(range);
        if (!m_bInterrupted)
        {
            emit measureContinuous(start*1000, stop*1000, m_dotsNumber);
        } else {
            m_bInterrupted = true;
            ui->measurmentsDeleteBtn->setEnabled(true);
            ui->measurmentsClearBtn->setEnabled(true);
            m_analyzer->setContinuos(false);
            m_analyzer->setIsMeasuring(false);
            PopUpIndicator::setIndicatorVisible(false);
        }
    } else {
        m_bInterrupted = true;
        ui->singleStart->setChecked(false);
        ui->measurmentsDeleteBtn->setEnabled(true);
        ui->measurmentsClearBtn->setEnabled(true);
        ui->exportBtn->setEnabled(true);
        ui->measurmentsSaveBtn->setEnabled(true);
        m_analyzer->setContinuos(false);
        m_analyzer->setIsMeasuring(false);
        PopUpIndicator::setIndicatorVisible(false);
    }
}

void MainWindow::moveEvent(QMoveEvent *)
{
    emit mainWindowPos(this->x(), this->y());
}

void MainWindow::resizeEvent(QResizeEvent * e)
{
    resizeWnd();
    QMainWindow::resizeEvent(e);
}

void MainWindow::resizeWnd(void)
{
    int width = m_smithWidget->width();
    int height = m_smithWidget->height();
    if(width > height)
    {
        double alfa = (double)width/height;
        double range = 14 * alfa;
        m_smithWidget->xAxis->setRangeLower((-1)*range/2);
        m_smithWidget->xAxis->setRangeUpper(range/2);
    }else
    {
        double alfa = (double)height/width;
        double range = 14 * alfa;
        m_smithWidget->yAxis->setRangeLower((-1)*range/2);
        m_smithWidget->yAxis->setRangeUpper(range/2);
    }
}

void MainWindow::on_settingsBtn_clicked()
{
    m_analyzer->setIsMeasuring(true);
    m_settingsDialog = new Settings(this);
    m_settingsDialog->setAttribute(Qt::WA_DeleteOnClose);
    m_settingsDialog->setWindowTitle(tr("Settings"));
    m_settingsDialog->setAnalyzer(m_analyzer);
    m_settingsDialog->setCalibration(m_calibration);
    m_settingsDialog->setMeasureSystemMetric(m_measureSystemMetric);
    m_settingsDialog->setZ0(m_Z0);
    m_settingsDialog->setCableVelFactor(m_cableVelFactor);
    m_settingsDialog->setCableResistance(m_cableResistance);

    m_settingsDialog->setCableLossConductive(m_cableLossConductive);
    m_settingsDialog->setCableLossDielectric(m_cableLossDielectric);
    m_settingsDialog->setCableLossFqMHz(m_cableLossFqMHz);
    m_settingsDialog->setCableLossUnits(m_cableLossUnits);
    m_settingsDialog->setCableLossAtAnyFq(m_cableLossAtAnyFq);
    m_settingsDialog->setCableLength(m_cableLength);
    m_settingsDialog->setCableFarEndMeasurement(m_farEndMeasurement);
    m_settingsDialog->setCableIndex(m_cableIndex);
    m_settingsDialog->setFirmwareAutoUpdate(m_autoFirmwareUpdateEnabled);
    m_settingsDialog->setAntScopeAutoUpdate(m_autoUpdateEnabled);
    m_settingsDialog->setAntScopeVersion(ANTSCOPE2VER);
    m_settingsDialog->setAutoDetectMode(m_autoDetectMode, m_serialPort);

    QStringList list;
    for(int i = 0; i < LANGUAGES_QUANTITY; ++i)
    {
        list << languages[i];
    }
    m_settingsDialog->setLanguages(list, m_languageNumber);

    m_settingsDialog->setBands(m_BandsMap.keys());

    if(m_measurements)
    {
        connect(m_settingsDialog, SIGNAL(graphHintChecked(bool)),
                m_measurements,SLOT(setGraphHintEnabled(bool)));
        connect(m_settingsDialog, SIGNAL(graphBriefHintChecked(bool)),
                m_measurements,SLOT(setGraphBriefHintEnabled(bool)));
    }

    if(m_markers)
    {
        //m_settingsDialog->setMarkersHintChecked(m_markers->getMarkersHintEnabled());
        connect(m_settingsDialog, SIGNAL(markersHintChecked(bool)),
                m_markers,SLOT(setMarkersHintEnabled(bool)));
    }
    connect(m_settingsDialog, SIGNAL(checkUpdatesBtn()),
            m_analyzer,SLOT(on_checkUpdatesBtn_clicked()));
    connect(m_settingsDialog, SIGNAL(autoUpdatesCheckBox(bool)),
            m_analyzer,SLOT(setAutoCheckUpdate(bool)));
    connect(m_settingsDialog, SIGNAL(updateBtn(QString)),
            m_analyzer,SLOT(readFile(QString)));

    connect(m_settingsDialog,SIGNAL(startCalibration()),
            m_calibration,SLOT(on_startCalibration()));
    connect(m_settingsDialog,SIGNAL(startCalibrationOpen()),
            m_calibration,SLOT(on_startCalibrationOpen()));
    connect(m_settingsDialog,SIGNAL(startCalibrationShort()),
            m_calibration,SLOT(on_startCalibrationShort()));
    connect(m_settingsDialog,SIGNAL(startCalibrationLoad()),
            m_calibration,SLOT(on_startCalibrationLoad()));

    connect(m_settingsDialog,SIGNAL(openOpenFile(QString)),
            m_calibration,SLOT(on_openOpenFile(QString)));
    connect(m_settingsDialog,SIGNAL(shortOpenFile(QString)),
            m_calibration,SLOT(on_shortOpenFile(QString)));
    connect(m_settingsDialog,SIGNAL(loadOpenFile(QString)),
            m_calibration,SLOT(on_loadOpenFile(QString)));

    connect(m_settingsDialog,SIGNAL(calibrationEnabled(bool)),
            m_calibration,SLOT(on_enableOSLCalibration(bool)));
    connect(m_settingsDialog,SIGNAL(calibrationEnabled(bool)),
            m_measurements,SLOT(on_calibrationEnabled(bool)));

    connect(m_calibration,SIGNAL(progress(int, int)),
            m_settingsDialog,SLOT(on_percentCalibrationChanged(qint32,qint32)));

    connect(m_analyzer, SIGNAL(updatePercentChanged(qint32)),
            m_settingsDialog,SLOT(on_percentChanged(qint32)));

    connect(m_settingsDialog,SIGNAL(changeMeasureSystemMetric(bool)),
            this,SLOT(on_changeMeasureSystemMetric(bool)));
    connect(m_settingsDialog,SIGNAL(changeMeasureSystemMetric(bool)),
            m_measurements,SLOT(on_changeMeasureSystemMetric(bool)));

    connect(m_settingsDialog, SIGNAL(Z0Changed(double)),
            this, SLOT(on_Z0Changed(double)));

    connect(m_settingsDialog, SIGNAL(paramsChanged()),
            this, SLOT(on_settingsParamsChanged()));

    connect(m_analyzer, SIGNAL(aa30bootFound()),
            m_settingsDialog, SLOT(on_aa30bootFound()));
    connect(m_analyzer, SIGNAL(aa30updateComplete()),
            m_settingsDialog, SLOT(on_aa30updateComplete()));

    connect(m_settingsDialog,SIGNAL(firmwareAutoUpdateStateChanged(bool)),this, SLOT(on_firmwareAutoUpdateStateChanged(bool)));

    connect(m_settingsDialog,SIGNAL(antScopeAutoUpdateStateChanged(bool)),this, SLOT(on_antScopeAutoUpdateStateChanged(bool)));

    connect(m_settingsDialog, SIGNAL(changedAutoDetectMode(bool)), this, SLOT(on_changedAutoDetectMode(bool)));

    connect(m_settingsDialog, SIGNAL(changedSerialPort(QString)), this, SLOT(on_changedSerialPort(QString)));

    connect(m_settingsDialog, SIGNAL(languageChanged(int)), this, SLOT(on_translate(int)));

    connect(m_settingsDialog, SIGNAL(bandChanged(QString)), this, SLOT(on_bandChanged(QString)));

    m_settingsDialog->exec();

    ui->checkBoxCalibration->setEnabled(m_calibration->isCalibrationPerformed());
    ui->checkBoxCalibration->setChecked(m_calibration->getCalibrationEnabled());
}

void MainWindow::on_dotsNumberChanged(int number)
{
    m_dotsNumber = number;
}

void MainWindow::on_measurmentsDeleteBtn_clicked()
{
    if(m_analyzer->isMeasuring())
    {
        return;
    }
    QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();
    for(int i = 0; i < list.length(); ++i)
    {
        QTableWidgetItem * item = list.at(i);
        int rowNumber = item->row();
        m_measurements->deleteRow(rowNumber);
    }

    if(ui->tableWidget_measurments->rowCount() == 0)
    {
//        m_settings->beginGroup("MainWindow");
//        qint64 from = m_settings->value("rangeLower",0).toULongLong();
//        qint64 to =  m_settings->value("rangeUpper",1400000).toULongLong();
//        qint32 dots = m_settings->value("dotsNumber", 50).toInt();
//        m_settings->endGroup();

//        qint64 range = (to - from);
//        on_dataChanged(from + range/2, range, dots);
        onFullRange(true);

        ui->measurmentsSaveBtn->setEnabled(false);
        ui->measurmentsDeleteBtn->setEnabled(false);
        ui->measurmentsClearBtn->setEnabled(false);
        ui->exportBtn->setEnabled(false);
    }
    else
    {
        on_tableWidget_measurments_cellClicked(ui->tableWidget_measurments->rowCount()-1, 0);
    }
    if(m_markers)
    {
        m_markers->repaint();
        m_markers->redraw();
    }
}


void MainWindow::on_measurementsClearBtn_clicked(bool)
{
    if(m_analyzer->isMeasuring())
    {
        return;
    }

    while(ui->tableWidget_measurments->rowCount() != 0)
    {
        QTableWidgetItem * item = ui->tableWidget_measurments->item(0, 0);
        int rowNumber = item->row();
        m_measurements->deleteRow(rowNumber);
    }

//    m_settings->beginGroup("MainWindow");
//    qint64 from = m_settings->value("rangeLower",0).toULongLong();
//    qint64 to =  m_settings->value("rangeUpper",1400000).toULongLong();
//    qint32 dots = m_settings->value("dotsNumber", 50).toInt();
//    m_settings->endGroup();

//    qint64 range = (to - from);
//    on_dataChanged(from + range/2, range, dots);
    onFullRange(true);

    if(ui->tableWidget_measurments->rowCount() == 0)
    {
        ui->measurmentsSaveBtn->setEnabled(false);
        ui->measurmentsDeleteBtn->setEnabled(false);
        ui->measurmentsClearBtn->setEnabled(false);
        ui->exportBtn->setEnabled(false);
    }
    if(m_markers)
    {
        m_markers->repaint();
        m_markers->redraw();
    }
}

#if 0
void MainWindow::on_tableWidget_measurments_cellClicked(int row, int column)
{
    Q_UNUSED(column)
    int count = m_swrWidget->graphCount();
    if(count > 0)
    {
        for(int i = 1; i < count; ++i)
        {
            int j = (i-1)*3 + 1;
            if((i-1) == row)
            {
                QPen pen = m_swrWidget->graph(i)->pen();
                pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
                m_swrWidget->graph(i)->setPen(pen);
                m_phaseWidget->graph(i)->setPen(pen);                
                m_rlWidget->graph(i)->setPen(pen);
                m_measurements->getMeasurement(count - 2 - (i-1))->smithCurve->setPen(pen);

                pen = m_rpWidget->graph(j+0)->pen();
                pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
                m_rpWidget->graph(j+0)->setPen(pen);

                pen = m_rpWidget->graph(j+1)->pen();
                pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
                m_rpWidget->graph(j+1)->setPen(pen);

                pen = m_rpWidget->graph(j+2)->pen();
                pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
                m_rpWidget->graph(j+2)->setPen(pen);


                pen = m_rsWidget->graph(j+0)->pen();
                pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
                m_rsWidget->graph(j+0)->setPen(pen);

                pen = m_rsWidget->graph(j+1)->pen();
                pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
                m_rsWidget->graph(j+1)->setPen(pen);

                pen = m_rsWidget->graph(j+2)->pen();
                pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
                m_rsWidget->graph(j+2)->setPen(pen);
            } else {
                QPen pen = m_swrWidget->graph(i)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_swrWidget->graph(i)->setPen(pen);
                m_phaseWidget->graph(i)->setPen(pen);
                m_rlWidget->graph(i)->setPen(pen);
                m_measurements->getMeasurement(count - 2 - (i-1))->smithCurve->setPen(pen);

                pen = m_rpWidget->graph(j+0)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rpWidget->graph(j+0)->setPen(pen);

                pen = m_rpWidget->graph(j+1)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rpWidget->graph(j+1)->setPen(pen);

                pen = m_rpWidget->graph(j+2)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rpWidget->graph(j+2)->setPen(pen);


                pen = m_rsWidget->graph(j+0)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rsWidget->graph(j+0)->setPen(pen);

                pen = m_rsWidget->graph(j+1)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rsWidget->graph(j+1)->setPen(pen);

                pen = m_rsWidget->graph(j+2)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rsWidget->graph(j+2)->setPen(pen);
            }
        }
        updateGraph();
    }
}
#endif
void MainWindow::on_tableWidget_measurments_cellClicked(int row, int column)
{
    Q_UNUSED(column)
    int count = m_swrWidget->graphCount();
    if(count > 0)
    {
        for(int i = 1; i < count; ++i)
        {
            int j = (i-1)*3 + 1;
//            if((i-1) != row)
            {
                QPen pen = m_swrWidget->graph(i)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_swrWidget->graph(i)->setPen(pen);
                m_phaseWidget->graph(i)->setPen(pen);
                m_rlWidget->graph(i)->setPen(pen);
                m_measurements->getMeasurement(count - 2 - (i-1))->smithCurve->setPen(pen);

                pen = m_rpWidget->graph(j+0)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rpWidget->graph(j+0)->setPen(pen);

                pen = m_rpWidget->graph(j+1)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rpWidget->graph(j+1)->setPen(pen);

                pen = m_rpWidget->graph(j+2)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rpWidget->graph(j+2)->setPen(pen);


                pen = m_rsWidget->graph(j+0)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rsWidget->graph(j+0)->setPen(pen);

                pen = m_rsWidget->graph(j+1)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rsWidget->graph(j+1)->setPen(pen);

                pen = m_rsWidget->graph(j+2)->pen();
                pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                m_rsWidget->graph(j+2)->setPen(pen);
            }
        }

        int i = row+1;
        int j = (i-1)*3 + 1;

        QPen pen = m_swrWidget->graph(i)->pen();
        pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
        m_swrWidget->graph(i)->setPen(pen);
        m_phaseWidget->graph(i)->setPen(pen);
        m_rlWidget->graph(i)->setPen(pen);
        m_measurements->getMeasurement(count - 2 - (i-1))->smithCurve->setPen(pen);

        pen = m_rpWidget->graph(j+0)->pen();
        pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
        m_rpWidget->graph(j+0)->setPen(pen);

        pen = m_rpWidget->graph(j+1)->pen();
        pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
        m_rpWidget->graph(j+1)->setPen(pen);

        pen = m_rpWidget->graph(j+2)->pen();
        pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
        m_rpWidget->graph(j+2)->setPen(pen);


        pen = m_rsWidget->graph(j+0)->pen();
        pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
        m_rsWidget->graph(j+0)->setPen(pen);

        pen = m_rsWidget->graph(j+1)->pen();
        pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
        m_rsWidget->graph(j+1)->setPen(pen);

        pen = m_rsWidget->graph(j+2)->pen();
        pen.setWidth(ACTIVE_GRAPH_PEN_WIDTH);
        m_rsWidget->graph(j+2)->setPen(pen);

        updateGraph();
    }
}


/*
void MainWindow::on_screenshot_clicked()
{
    QDateTime datetime = QDateTime::currentDateTime();
    QString path = "Images/" + datetime.toString("dd.MM.yyyy_hh.mm.ss");
    QString str = QFileDialog::getSaveFileName(this, "Export BMP", path, "*.bmp");
    if(str.isEmpty())
    {
        return;
    }
    if(str.indexOf(".bmp") == -1)
    {
        str += ".bmp";
    }
    QCoreApplication::processEvents();
    Sleep(300);
    QScreen *screen = QGuiApplication::primaryScreen();
    if (const QWindow *window = windowHandle())
        screen = window->screen();
    if (!screen)
    {
        return;
    }
    QRect rect = this->frameGeometry();
    QPixmap originalPixmap = screen->grabWindow(0,this->x(),this->y(),rect.width(),rect.height());
    QPixmap pix = originalPixmap.scaled(5000,3000,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    pix.save(str,"BMP",100);
    QApplication::clipboard()->setImage(pix.toImage(),QClipboard::Clipboard);
}
*/
void MainWindow::on_screenshot_clicked()
{
    QDateTime datetime = QDateTime::currentDateTime();
    QString path = "Images/" + datetime.toString("dd.MM.yyyy_hh.mm.ss");
    QString str = QFileDialog::getSaveFileName(this, "Export PNG", path, "*.png");
    if(str.isEmpty())
    {
        return;
    }
    if(str.indexOf(".png") == -1)
    {
        str += ".png";
    }

    on_pressCtrlC();
    QPixmap pixmap = QApplication::clipboard()->pixmap();
    if (!pixmap.isNull()) {
        pixmap.save(str, "PNG", 80);
    }
}

void MainWindow::on_printBtn_clicked()
{
    m_print = new Print();
    m_print->setAttribute(Qt::WA_DeleteOnClose);
    QList <QStringList> lst = m_markers->getMarkersHintList();
    for(int i = 0; i < lst.length(); ++i)
    {
        m_print->addRowText(lst.at(i));
    }

    QString model = CustomAnalyzer::customized() ?
                CustomAnalyzer::currentPrototype() : names[m_analyzer->getModel()];
    QString name = ui->tabWidget->currentWidget()->objectName();
    QString string;
    string += model + ", ";
    QDateTime datetime = QDateTime::currentDateTime();
    string += datetime.toString("dd.MM.yyyy-hh:mm, ");

    if(name == "tab_1")
    {
        string += "SWR graph";
        m_print->drawBands( 1, MAX_SWR);
        m_print->setRange(m_swrWidget->xAxis->range(),m_swrWidget->yAxis->range());
        m_print->setLabel(m_swrWidget->xAxis->label(), m_swrWidget->yAxis->label());
        for(int i = 1; i < m_swrWidget->graphCount(); ++i)
        {
            QModelIndex myIndex = ui->tableWidget_measurments->model()->
                    index( m_swrWidget->graphCount()-i-1, 0, QModelIndex());

            m_print->setData(m_swrWidget->graph(i)->data(), m_swrWidget->graph(i)->pen(), myIndex.data().toString());
        }
    }else if(name == "tab_2")
    {
        string += "Phase graph";
        m_print->drawBands( m_phaseWidget->yAxis->range().lower, m_phaseWidget->yAxis->range().upper);
        m_print->setRange(m_phaseWidget->xAxis->range(),m_phaseWidget->yAxis->range());
        m_print->setLabel(m_phaseWidget->xAxis->label(), m_phaseWidget->yAxis->label());
        for(int i = 1; i < m_phaseWidget->graphCount(); ++i)
        {
            QModelIndex myIndex = ui->tableWidget_measurments->model()->
                    index( m_swrWidget->graphCount()-i-1, 0, QModelIndex());

            m_print->setData(m_phaseWidget->graph(i)->data(), m_phaseWidget->graph(i)->pen(), myIndex.data().toString());
        }
    }else if(name == "tab_3")
    {
        string += "RXZ graph";
        m_print->drawBands( m_rsWidget->yAxis->range().lower, m_rsWidget->yAxis->range().upper);
        m_print->setRange(m_rsWidget->xAxis->range(),m_rsWidget->yAxis->range());
        m_print->setLabel(m_rsWidget->xAxis->label(), m_rsWidget->yAxis->label());
        for(int i = 1; i < m_rsWidget->graphCount(); ++i)
        {
            m_print->setData(m_rsWidget->graph(i)->data(), m_rsWidget->graph(i)->pen(), m_rsWidget->graph(i)->name());
        }
    }else if(name == "tab_4")
    {
        string += "RXZ parallel graph";
        m_print->drawBands( m_rpWidget->yAxis->range().lower, m_rpWidget->yAxis->range().upper);
        m_print->setRange(m_rpWidget->xAxis->range(),m_rpWidget->yAxis->range());
        m_print->setLabel(m_rpWidget->xAxis->label(), m_rpWidget->yAxis->label());
        for(int i = 1; i < m_rpWidget->graphCount(); ++i)
        {
            m_print->setData(m_rpWidget->graph(i)->data(), m_rpWidget->graph(i)->pen(), m_rpWidget->graph(i)->name());
        }
    }else if(name == "tab_5")
    {
        string += "RL graph";
        m_print->drawBands( m_rlWidget->yAxis->range().lower, m_rlWidget->yAxis->range().upper);
        m_print->setRange(m_rlWidget->xAxis->range(),m_rlWidget->yAxis->range());
        m_print->setLabel(m_rlWidget->xAxis->label(), m_rlWidget->yAxis->label());
        for(int i = 1; i < m_rlWidget->graphCount(); ++i)
        {
            QModelIndex myIndex = ui->tableWidget_measurments->model()->
                    index( m_swrWidget->graphCount()-i-1, 0, QModelIndex());

            m_print->setData(m_rlWidget->graph(i)->data(), m_rlWidget->graph(i)->pen(), myIndex.data().toString());
        }
    }else if(name == "tab_6")
    {
        string += "TDR graph";
        m_print->drawBands( m_tdrWidget->yAxis->range().lower, m_tdrWidget->yAxis->range().upper);
        m_print->setRange(m_tdrWidget->xAxis->range(),m_tdrWidget->yAxis->range());
        m_print->setLabel(m_tdrWidget->xAxis->label(), m_tdrWidget->yAxis->label());
        for(int i = 1; i < m_tdrWidget->graphCount(); ++i)
        {
            m_print->setData(m_tdrWidget->graph(i)->data(), m_tdrWidget->graph(i)->pen(), m_tdrWidget->graph(i)->name());
        }
    }else if(name == "tab_7")
    {
        string += "Smith graph";
        m_print->drawSmithImage();
        m_print->setRange(m_smithWidget->xAxis->range(),m_smithWidget->yAxis->range());
        m_print->setLabel(m_smithWidget->xAxis->label(), m_smithWidget->yAxis->label());

        for(int i = 0; i < m_measurements->getMeasurementLength(); ++i)
        {
            QModelIndex myIndex = ui->tableWidget_measurments->model()->
                                index( m_smithWidget->graphCount()-i-1, 0, QModelIndex());
            m_print->setSmithData(&m_measurements->getMeasurement(i)->smithGraph,
                                  m_measurements->getMeasurement(i)->smithCurve->pen(),//m_smithWidget->graph(i)->pen(),
                                  myIndex.data().toString());
        }
    }

    if(name != "tab_7")
    {
        qint32 markersCount = m_markers->getMarkersCount();
        for(int i = 0; i < markersCount; ++i)
        {
            m_print->addMarker(m_markers->getMarker(i).frequency, i+1);
        }
    }

    m_print->setHead(string);

    m_print->exec();
}

void MainWindow::on_measurmentsSaveBtn_clicked()
{
    QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();

    if(!list.isEmpty())
    {
        if(m_lastSavePath.indexOf('.') >= 0)
        {
            m_lastSavePath.remove(m_lastSavePath.indexOf('.'),4);
            m_lastSavePath.append(".asd");
        }
        QString path = QFileDialog::getSaveFileName(this, "Save file", m_lastSavePath, "AntScope2 (*.asd )");
        if(!path.isEmpty())
        {
            m_lastSavePath = path;
            QTableWidgetItem * item = list.at(0);
            m_measurements->saveData(item->row(), path);
        }
    }
}

void MainWindow::on_measurementsOpenBtn_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Open file", m_lastOpenPath, "AntScope2 (*.asd )");
    if(!path.isEmpty())
    {
        m_lastSavePath = path;

        m_measurements->loadData( path );
        ui->measurmentsSaveBtn->setEnabled(true);
        ui->exportBtn->setEnabled(true);
        ui->measurmentsDeleteBtn->setEnabled(true);
        ui->measurmentsClearBtn->setEnabled(true);
    }
}

void MainWindow::openFile(QString path)
{
    m_measurements->loadData(path);
    ui->measurmentsSaveBtn->setEnabled(true);
    ui->exportBtn->setEnabled(true);
    ui->measurmentsDeleteBtn->setEnabled(true);
    ui->measurmentsClearBtn->setEnabled(true);
}

void MainWindow::on_importBtn_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Open file", m_lastOpenPath,  "S1p (*.s1p);;"
                                                                                    "Csv (*.csv);;"
                                                                                    "Nwl (*.nwl);;"
                                                                                    "AntScope1 (*.antdata);;"
                                                                                    "AntScope2 (*.asd )");
    m_measurements->loadData(path);
    ui->measurmentsSaveBtn->setEnabled(true);
    ui->exportBtn->setEnabled(true);
    ui->measurmentsDeleteBtn->setEnabled(true);
    ui->measurmentsClearBtn->setEnabled(true);
}

void MainWindow::on_changeMeasureSystemMetric (bool state)
{
    m_measureSystemMetric = state;
}

void MainWindow::on_Z0Changed(double _Z0)
{
    m_Z0 = _Z0;
    m_calibration->setZ0(m_Z0);
    m_measurements->setZ0(m_Z0);
}

void MainWindow::updateGraph ()
{
    QCustomPlot* plot = nullptr;
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_1")
    {
        plot = m_swrWidget;
        m_swrWidget->replot();
    }else if(str == "tab_2")
    {
        plot = m_phaseWidget;
        m_phaseWidget->replot();
    }else if(str == "tab_3")
    {
        plot = m_rsWidget;

//        int count = m_rsWidget->legend->itemCount();
//        for (int i=4; i<count; i++) {
//            m_rsWidget->legend->removeItem(m_rsWidget->legend->itemCount()-1);
//        }
//        qDebug() << "Legend: " << count;

        m_rsWidget->replot();
    }else if(str == "tab_4")
    {
        plot = m_rpWidget;
        m_rpWidget->replot();
    }else if(str == "tab_5")
    {
        plot = m_rlWidget;
        m_rlWidget->replot();
    }else if(str == "tab_6")
    {
        plot = m_tdrWidget;
        m_tdrWidget->replot();
    }else if(str == "tab_7")
    {
        resizeWnd();
        plot = m_smithWidget;
        m_smithWidget->replot();
    }
}

void MainWindow::on_settingsParamsChanged()
{
    if(m_settingsDialog != NULL)
    {
        m_cableVelFactor = m_settingsDialog->getCableVelFactor();
        m_cableResistance = m_settingsDialog->getCableResistance();
        m_cableLossConductive = m_settingsDialog->getCableLossConductive();
        m_cableLossDielectric = m_settingsDialog->getCableLossDielectric();
        m_cableLossFqMHz = m_settingsDialog->getCableLossFqMHz();
        m_cableLossUnits = m_settingsDialog->getCableLossUnits();
        m_cableLossAtAnyFq = m_settingsDialog->getCableLossAtAnyFq();
        m_cableLength = m_settingsDialog->getCableLength();
        m_farEndMeasurement = m_settingsDialog->getCableFarEndMeasurement();
        m_cableIndex = m_settingsDialog->getCableIndex();

        if(m_measurements != NULL)
        {
            m_measurements->setCableVelFactor(m_cableVelFactor);
            m_measurements->setCableResistance(m_cableResistance);
            m_measurements->setCableLossConductive(m_cableLossConductive);
            m_measurements->setCableLossDielectric(m_cableLossDielectric);
            m_measurements->setCableLossFqMHz(m_cableLossFqMHz);
            m_measurements->setCableLossUnits(m_cableLossUnits);
            m_measurements->setCableLossAtAnyFq(m_cableLossAtAnyFq);
            m_measurements->setCableLength(m_cableLength);
            m_measurements->setCableFarEndMeasurement(m_farEndMeasurement);            
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
    }
}

void MainWindow::on_limitsBtn_clicked(bool checked)
{
    if( !checked )
    {
        if(!m_isRange)
        {
            ui->limitsBtn->setChecked(true);
        }
    }else
    {
        m_isRange = false;
        ui->rangeBtn->setChecked(false);
        ui->startLabel->setText(tr("Start"));
        ui->stopLabel->setText(tr("Stop"));
        ui->groupBox_Presets->setTitle(tr("Presets (limits), kHz"));
        ui->tableWidget_presets->horizontalHeaderItem(0)->setText(tr("Start"));
        ui->tableWidget_presets->horizontalHeaderItem(1)->setText(tr("Stop"));
        double center = ui->lineEdit_fqFrom->text().remove(' ').toDouble();
        double range = ui->lineEdit_fqTo->text().remove(' ').toDouble();

        setFqFrom(center - range);
        setFqTo(center + range);
        emit isRangeChanged(m_isRange);
    }
}

void MainWindow::on_rangeBtn_clicked(bool checked)
{
    if( !checked )
    {
        if(m_isRange)
        {
            ui->rangeBtn->setChecked(true);
        }
    }else
    {
        m_isRange = true;
        ui->limitsBtn->setChecked(false);
        ui->startLabel->setText(tr("Center"));
        ui->stopLabel->setText(tr("Range (+/-)"));
        ui->groupBox_Presets->setTitle(tr("Presets (center, range), kHz"));
        ui->tableWidget_presets->horizontalHeaderItem(0)->setText(tr("Center"));
        ui->tableWidget_presets->horizontalHeaderItem(1)->setText(tr("Range(+/-)"));
        double from = getFqFrom();
        double to = getFqTo();
        setFqFrom((to + from)/2);
        setFqTo((to - from)/2);
        emit isRangeChanged(m_isRange);
    }
}


void MainWindow::setFqFrom(QString from)
{
    from.remove(' ');
//    if(from.length() > 6)
//    {
//        from.insert(from.length()-6,' ');
//    }
//    if(from.length() > 3)
//    {
//        from.insert(from.length()-3,' ');
//    }
    from = appendSpaces(from);
    ui->lineEdit_fqFrom->setText(from);
}

void MainWindow::setFqFrom(double from)
{
    QString sFrom = QString::number(from,'f', 0);
//    if(sFrom.length() > 6)
//    {
//        sFrom.insert(sFrom.length()-6,' ');
//    }
//    if(sFrom.length() > 3)
//    {
//        sFrom.insert(sFrom.length()-3,' ');
//    }
    sFrom = appendSpaces(sFrom);
    ui->lineEdit_fqFrom->setText(sFrom);
}

void MainWindow::setFqTo(QString to)
{
    to.remove(' ');
//    if(to.length() > 6)
//    {
//        to.insert(to.length()-6,' ');
//    }
//    if(to.length() > 3)
//    {
//        to.insert(to.length()-3,' ');
//    }
    to = appendSpaces(to);
    ui->lineEdit_fqTo->setText(to);
}

void MainWindow::setFqTo(double to)
{
    QString sTo = QString::number(to,'f', 0);
//    if(sTo.length() > 6)
//    {
//        sTo.insert(sTo.length()-6,' ');
//    }
//    if(sTo.length() > 3)
//    {
//        sTo.insert(sTo.length()-3,' ');
//    }
    sTo = appendSpaces(sTo);
    ui->lineEdit_fqTo->setText(sTo);
}

double MainWindow::getFqFrom(void)
{
    return m_swrWidget->xAxis->range().lower;
}

double MainWindow::getFqTo(void)
{
    return m_swrWidget->xAxis->range().upper;
}

void MainWindow::on_lineEdit_fqFrom_editingFinished()
{
    setFqFrom(ui->lineEdit_fqFrom->text());
    double lower = 0;
    double upper = 0;
    if(!m_isRange)
    {
        lower = ui->lineEdit_fqFrom->text().remove(' ').toDouble();
        upper = ui->lineEdit_fqTo->text().remove(' ').toDouble();
        if(lower >= upper)
        {
            upper = lower + 1;
            m_swrWidget->xAxis->setRangeUpper(upper);
            m_phaseWidget->xAxis->setRangeUpper(upper);
            m_rsWidget->xAxis->setRangeUpper(upper);
            m_rpWidget->xAxis->setRangeUpper(upper);
            m_rlWidget->xAxis->setRangeUpper(upper);
        }
        if(lower < 0)
        {
            lower = 0;
        }

        m_swrWidget->xAxis->setRangeLower(lower);
        m_phaseWidget->xAxis->setRangeLower(lower);
        m_rsWidget->xAxis->setRangeLower(lower);
        m_rpWidget->xAxis->setRangeLower(lower);
        m_rlWidget->xAxis->setRangeLower(lower);
    }else
    {
        double from = ui->lineEdit_fqFrom->text().remove(' ').toDouble();
        double to = ui->lineEdit_fqTo->text().remove(' ').toDouble();
        lower = from - to;
        upper = from + to;
        if(lower >= upper)
        {
            upper = lower + 1;
            m_swrWidget->xAxis->setRangeUpper(upper);
            m_phaseWidget->xAxis->setRangeUpper(upper);
            m_rsWidget->xAxis->setRangeUpper(upper);
            m_rpWidget->xAxis->setRangeUpper(upper);
            m_rlWidget->xAxis->setRangeUpper(upper);
        }
        if(lower < 0)
        {
            lower = 0;
        }
        m_swrWidget->xAxis->setRangeUpper(lower+ui->lineEdit_fqTo->text().remove(' ').toDouble()*2);
        m_swrWidget->xAxis->setRangeLower(lower);

        m_phaseWidget->xAxis->setRangeUpper(lower+ui->lineEdit_fqTo->text().remove(' ').toDouble()*2);
        m_phaseWidget->xAxis->setRangeLower(lower);

        m_rsWidget->xAxis->setRangeUpper(lower+ui->lineEdit_fqTo->text().remove(' ').toDouble()*2);
        m_rsWidget->xAxis->setRangeLower(lower);

        m_rpWidget->xAxis->setRangeUpper(lower+ui->lineEdit_fqTo->text().remove(' ').toDouble()*2);
        m_rpWidget->xAxis->setRangeLower(lower);

        m_rlWidget->xAxis->setRangeUpper(lower+ui->lineEdit_fqTo->text().remove(' ').toDouble()*2);
        m_rlWidget->xAxis->setRangeLower(lower);
    }
    updateGraph();
    //ui->lineEdit_fqTo->setFocus();
    //ui->lineEdit_fqTo->selectAll();
}

void MainWindow::on_lineEdit_fqTo_editingFinished()
{
    setFqTo(ui->lineEdit_fqTo->text());
    double lower;
    double upper;
    if(!m_isRange)
    {
        lower = ui->lineEdit_fqFrom->text().remove(' ').toDouble();
        upper = ui->lineEdit_fqTo->text().remove(' ').toDouble();
        if(lower >= upper)
        {
            lower = upper - 1;
            m_swrWidget->xAxis->setRangeLower(lower);
            m_phaseWidget->xAxis->setRangeLower(lower);
            m_rsWidget->xAxis->setRangeLower(lower);
            m_rpWidget->xAxis->setRangeLower(lower);
            m_rlWidget->xAxis->setRangeLower(lower);
        }
        m_swrWidget->xAxis->setRangeUpper(upper);
        m_phaseWidget->xAxis->setRangeUpper(upper);
        m_rsWidget->xAxis->setRangeUpper(upper);
        m_rpWidget->xAxis->setRangeUpper(upper);
        m_rlWidget->xAxis->setRangeUpper(upper);
    }else
    {
        lower = ui->lineEdit_fqFrom->text().remove(' ').toDouble() - ui->lineEdit_fqTo->text().remove(' ').toDouble();
        upper = ui->lineEdit_fqFrom->text().remove(' ').toDouble() + ui->lineEdit_fqTo->text().remove(' ').toDouble();
        if(lower >= upper)
        {
            lower = upper - 1;
            m_swrWidget->xAxis->setRangeLower(lower);
            m_phaseWidget->xAxis->setRangeLower(lower);
            m_rsWidget->xAxis->setRangeLower(lower);
            m_rpWidget->xAxis->setRangeLower(lower);
            m_rlWidget->xAxis->setRangeLower(lower);
        }
        if(lower < 0)
        {
            lower = 0;
        }
        m_swrWidget->xAxis->setRangeUpper(upper);
        m_swrWidget->xAxis->setRangeLower(lower);

        m_phaseWidget->xAxis->setRangeUpper(upper);
        m_phaseWidget->xAxis->setRangeLower(lower);

        m_rsWidget->xAxis->setRangeUpper(upper);
        m_rsWidget->xAxis->setRangeLower(lower);

        m_rpWidget->xAxis->setRangeUpper(upper);
        m_rpWidget->xAxis->setRangeLower(lower);

        m_rlWidget->xAxis->setRangeUpper(upper);
        m_rlWidget->xAxis->setRangeLower(lower);
    }
    updateGraph();
    ui->singleStart->setFocus();
}

void MainWindow::on_newVersionAvailable()
{
#ifdef Q_OS_WIN
    if(m_updateDialog == NULL)
    {
        m_updateDialog = new AntScopeUpdateDialog();
        connect(m_updateDialog,SIGNAL(downloadAfterClosing()),this,SLOT(on_downloadAfterClosing()));
        connect(m_updateDialog,SIGNAL(downloadNow()),m_updater,SLOT(on_startDownload()));
        connect(m_updater,SIGNAL(progress(int)),m_updateDialog,SLOT(on_progress(int)));
    }
    m_updateDialog->setAsNewVersion();
    m_updateDialog->show();
#endif

#ifdef Q_OS_DARWIN
    if (m_updater != nullptr)
    {
        const Downloader* downloader = m_updater->downloader();
        if (downloader != nullptr)
        {
            QMessageBox msgBox;
            msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
            QString text = QString("<a href='%1' style='color:#01B2FF;'><br><br>%2<center>%3</center><br></a>")
                    .arg(downloader->downloadLink())
                    .arg(tr("New version of AntScope2 is available!"))
                    .arg(tr("Click to Download"));
            msgBox.setWindowTitle(tr("AntScope2 update"));
            msgBox.setText(text);
            msgBox.setStandardButtons(QMessageBox::Close);
            msgBox.exec();
        }
    }
#endif
}

void MainWindow::on_downloadAfterClosing()
{
    m_deferredUpdate = true;
}

void MainWindow::on_firmwareAutoUpdateStateChanged(bool state)
{
    m_autoFirmwareUpdateEnabled = state;
}

void MainWindow::on_antScopeAutoUpdateStateChanged(bool state)
{
    m_autoUpdateEnabled = state;
}

void MainWindow::on_1secTimerTick()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if(str == "tab_6" || str == "tab_7")
    {
        m_measurements->hideGraphBriefHint();
        return;
    }
    if(   (QCursor::pos().x() > (this->geometry().x() + ui->tabWidget->x() + m_swrWidget->x())) &&
          (QCursor::pos().x() < (this->geometry().x() + ui->tabWidget->x() + m_swrWidget->x() + m_swrWidget->width())) &&
          (QCursor::pos().y() > (this->geometry().y() + ui->tabWidget->y() + m_swrWidget->y())) &&
          (QCursor::pos().y() < (this->geometry().y() + ui->tabWidget->y() + m_swrWidget->y() + m_swrWidget->height()))
      )
    {
        m_measurements->showHideHints();
    }else
    {
        m_measurements->hideGraphBriefHint();
    }
}

void MainWindow::on_changedAutoDetectMode(bool state)
{
    m_autoDetectMode = state;
    m_analyzer->on_changedAutoDetectMode(state);
}

void MainWindow::on_changedSerialPort(QString portName)
{
    m_serialPort = portName;
    m_analyzer->on_changedSerialPort(portName);
}

bool MainWindow::loadLanguage(QString locale)
{ //locale: en, ukr, ru, ja, etc.
    QString title = windowTitle();
    bool res = m_qtLanguageTranslator->load("QtLanguage_" + locale, QCoreApplication::applicationDirPath());
    qApp->installTranslator(m_qtLanguageTranslator);
    ui->retranslateUi(this);

    m_swrWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_swrWidget->yAxis->setLabel(tr("SWR"));
    m_phaseWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_phaseWidget->yAxis->setLabel(tr("Angle"));
    m_rsWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_rsWidget->yAxis->setLabel(tr("Rs, Ohm"));
    m_rpWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_rpWidget->yAxis->setLabel(tr("Rp, Ohm"));
    m_rlWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_rlWidget->yAxis->setLabel(tr("RL, dB"));
    m_tdrWidget->xAxis->setLabel(tr("Length, m"));

    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_1), QApplication::translate("MainWindow", "SWR", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_2), QApplication::translate("MainWindow", "Phase", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_3), QApplication::translate("MainWindow", "Z=R+jX", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_4), QApplication::translate("MainWindow", "Z=R||+jX", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_5), QApplication::translate("MainWindow", "RL", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_6), QApplication::translate("MainWindow", "TDR", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_7), QApplication::translate("MainWindow", "Smith", 0));

    for (int i=0; i<ui->tabWidget->count(); i++)
    {
        QString tooltip = QString(tr("Press F%1")).arg(i+1);
        ui->tabWidget->setTabToolTip(i, tooltip);
    }

    if (m_settingsDialog != nullptr)
        m_settingsDialog->on_translate();
    if (m_measurements != nullptr)
        m_measurements->on_translate();
    if (m_markers != nullptr)
        m_markers->on_translate();
    if (m_fqSettings != nullptr)
        m_fqSettings->setWindowTitle(tr("Frequency settings"));
    if (m_exportDialog != nullptr)
        m_exportDialog->setWindowTitle(tr("Export"));
    if (m_screenshot != nullptr)
        m_screenshot->setWindowTitle(tr("Screenshot"));
    if (m_settingsDialog != nullptr)
        m_settingsDialog->setWindowTitle(tr("Settings"));

    setWindowTitle(title);

    return res;
}


void MainWindow::on_translate(int number)
{
    m_languageNumber = number;
    loadLanguage(languages_small[number]);
}

void MainWindow::on_calibrationChanged()
{
    if(m_markers)
    {
        m_markers->repaint();
        m_markers->redraw();
    }
}

void MainWindow::on_SaveFile(QString path)
{
    //int row = ui->tableWidget_measurments->rowCount() - 1;
    saveFile(0, path);
    ui->measurmentsSaveBtn->setEnabled(true);
}

void MainWindow::saveFile(int row, QString path)
{
    m_measurements->saveData(row, path);
}

void MainWindow::on_mouseDoubleClick(QMouseEvent* e)
{
    onCreateMarker(e->pos());
}

void MainWindow::onCreateMarker(QAction* action)
{
    onCreateMarker(action->data().toPoint());
}

void MainWindow::onCreateMarker(const QPoint& pos)
{
    //if (m_measurements->isEmpty())
      //  return;
    QCustomPlot* plot = getCurrentPlot();
    if (plot->objectName().contains("smith") || plot->objectName().contains("tdr"))
        return;
    double x = plot->xAxis->pixelToCoord(pos.x());
    m_addingMarker = true;
    m_markers->create(x);
    m_markers->setFq(x);
    m_markers->add();
}

void MainWindow::onCustomContextMenuRequested(const QPoint& pos)
{
    QMenu *menu=new QMenu(this);

    QCustomPlot* plot = getCurrentPlot();
    if (!plot->objectName().contains("smith") && !plot->objectName().contains("tdr"))
    {
        QAction* action = menu->addAction(QString("Create marker"));
        action->setData(pos);
        connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(onCreateMarker(QAction*)));
    }
    menu->popup(plot->mapToGlobal(pos));
}

QCustomPlot* MainWindow::getCurrentPlot()
{
    QWidget* w = ui->tabWidget->currentWidget();
    QList<QCustomPlot*> children = w->findChildren<QCustomPlot*>();
    if (children.isEmpty())
        return nullptr;
    return children[0];
}

bool MainWindow::loadBands()
{
    QString ituPath = Settings::programDataPath("itu-regions.txt");

    QFile file(ituPath);
    bool res = file.open(QFile::ReadOnly);
    if(!res)
        return false;
    QTextStream stream(&file);
    QString str;
    QStringList* list = nullptr;
    while(!stream.atEnd())
    {
        str = stream.readLine().trimmed();
        if (str.isEmpty())
            continue;
        if (str.indexOf('[') != -1)
        {
            int pos = str.indexOf(']');
            QString title = str.mid(1, pos-1);
            list = new QStringList();
            m_BandsMap.insert(title, list);
        } else {
            list->append(str);
        }
    }
    file.close();
    return true;
}

void MainWindow::on_bandChanged(QString band)
{
    if (m_BandsMap.contains(band))
    {
        while (!m_itemRectList.isEmpty()) {
            delete m_itemRectList.takeFirst();
        }

        QStringList* bands = m_BandsMap[band];
        setBands(m_swrWidget, bands, 1, MAX_SWR);
        setBands(m_phaseWidget, bands, -180, 180);
        setBands(m_rsWidget, bands, -2000, 2000);
        setBands(m_rpWidget, bands, -2000, 2000);
        setBands(m_rlWidget, bands, 0, 50);
    }
}


void MainWindow::onSpinChanged(int value)
{
    m_dotsNumber = value;
    m_measurements->on_dotsNumberChanged(value);
}

void MainWindow::calibrationToggled(bool checked)
{
    if(!m_calibration->getCalibrationPerformed())
    {
        QMessageBox::information(NULL, tr("!!!!Calibration not performed"),
                              tr("Calibration not performed."));

    }
    else
    {
        m_calibration->on_enableOSLCalibration(checked);
        m_measurements->on_calibrationEnabled(checked);
    }
}

void MainWindow::on_dataChanged(qint64 _center_khz, qint64 _range_khz, qint32 _dots)
{
    ui->spinBoxPoints->setValue(_dots);
    if (m_isRange) {
        ui->lineEdit_fqFrom->setText(QString::number(_center_khz));
        ui->lineEdit_fqTo->setText(QString::number(_range_khz/2));
    } else {
        ui->lineEdit_fqFrom->setText(QString::number(_center_khz - _range_khz/2));
        ui->lineEdit_fqTo->setText(QString::number(_center_khz + _range_khz/2));
    }
    on_lineEdit_fqTo_editingFinished();
    on_lineEdit_fqFrom_editingFinished();
}


void MainWindow::on_importFinished(double _fqMin_khz, double _fqMax_khz)
{
    qint64 _range = (qint64)(_fqMax_khz - _fqMin_khz);
    qint64 _center = (qint64)(_fqMin_khz + _range / 2);
    on_dataChanged(_center, _range, ui->spinBoxPoints->value());

    ui->measurmentsSaveBtn->setEnabled(true);
    ui->exportBtn->setEnabled(true);
    ui->measurmentsDeleteBtn->setEnabled(true);
    ui->measurmentsClearBtn->setEnabled(true);

}

QString appendSpaces(const QString& str) {
    QString tmp;
    int len = str.length();
    for (int idx=0; idx<len; idx++) {
        if (idx != 0 && (idx % 3) == 0)
            tmp.insert(0, ' ');
        tmp.insert(0, str[len - 1 - idx]);
    }
    return tmp;
}

void MainWindow::onFullRange(bool)
{
    int model = m_analyzer->getModel();

    qint64 from = minFq[model].toULongLong();
    qint64 to = maxFq[model].toULongLong();
    qint64 range = to - from;
    on_dataChanged(from + range/2, range, m_dotsNumber);
}



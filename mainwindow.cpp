#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "popupindicator.h"
#include "analyzer/customanalyzer.h"
#include "Notification.h"
#include "glwidget.h"
#include "CustomPlot.h"
#include "selectdevicedialog.h"
#include "printmulti.h"

extern QString appendSpaces(const QString& number);
extern bool g_developerMode; // see main.cpp
extern int g_maxMeasurements; // see measurements.cpp
extern void setAbsoluteFqMaximum();
extern bool g_bAA55modeNewProtocol;
MainWindow* MainWindow::m_mainWindow = nullptr;
QMap<QString, QString> g_mapTabPlotNames;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_measurements(NULL),
    m_fqSettings(NULL),
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
    //m_swrZoomState(10),
    m_phaseZoomState(10),
    m_rsZoomState(10),
    m_rpZoomState(10),
    m_rlZoomState(10),
    m_tdrZoomState(10),
    m_s21ZoomState(10),
    m_smithZoomState(10),
    m_userZoomState(10),
    m_languageNumber(0),
    m_addingMarker(false),
    m_bInterrupted(false)
{
    m_mainWindow = this;

    ui->setupUi(this);

    qInfo() << "* 1 sslLibraryBuildVersion: " << QSslSocket::sslLibraryBuildVersionString();
    qInfo() << "* 2 supportsSsl: " << QSslSocket::supportsSsl();
    qInfo() << "* 3 sslLibraryVersion: " << QSslSocket::sslLibraryVersionString();
    qInfo() << "* 4 Qt version: " << qVersion();

    setAbsoluteFqMaximum();

    g_mapTabPlotNames["tab_swr"] = "swr_widget";
    g_mapTabPlotNames["tab_phase"] = "phase_widget";
    g_mapTabPlotNames["tab_rs"] = "rs_widget";
    g_mapTabPlotNames["tab_rp"] = "rp_widget";
    g_mapTabPlotNames["tab_rl"] = "rl_widget";
    g_mapTabPlotNames["tab_tdr"] = "tdr_widget";
    g_mapTabPlotNames["tab_s21"] = "s21_widget";
    g_mapTabPlotNames["tab_smith"] = "smith_widget";
    g_mapTabPlotNames["tab_user"] = "user_widget";

    if (g_developerMode) {
        ui->spinBoxPoints->setRange(1, 1000000);
    }

//    QRegExp re("^[\d\s]*$");
//    QRegExpValidator *validator = new QRegExpValidator(re, this);
//    ui->lineEdit_fqFrom->setValidator(validator);
//    ui->lineEdit_fqTo->setValidator(validator);
    connect(ui->lineEdit_fqFrom, &QLineEdit::editingFinished, this, [=] {
        changeFqFrom(true);
    });
    connect(ui->lineEdit_fqTo, &QLineEdit::editingFinished, this, [=] {
        changeFqTo(true);
    });


    m_qtLanguageTranslator = new QTranslator();

    QString path = Settings::setIniFile();
    m_settings = new QSettings(path, QSettings::IniFormat);
    m_settings->beginGroup("MainWindow");
    QString sequence = m_settings->value("tabOrder","tab_swr,tab_phase,tab_rs,tab_rp,tab_rl,tab_smith,tab_tdr,tab_multi,tab_user").toString();
    if (!sequence.contains("tab_s21")) {
        sequence += ",tab_s21";
    }

#ifndef NO_MULTITAB
    bool hide_multi = true;
    if (!sequence.contains("tab_multi")) {
        sequence += ",tab_multi";
        hide_multi = false;
    }
#endif

    QString multi_tab = m_settings->value("multiTab","").toString();
    int cur_index = m_settings->value("currentTab",0).toInt();

    createTabs(sequence);

#ifndef NO_MULTITAB
    if (hide_multi) {
        ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(m_tab_multi), false);
        ui->printBtn->setEnabled(true);
        ui->tabWidget->setCurrentWidget(m_tab_swr);
    }
#endif


#ifndef NO_MULTITAB
    restoreMultitab(multi_tab);

    if (!ui->tabWidget->widget(cur_index)->isVisible()) {
        for(int i=0; i<ui->tabWidget->count(); i++) {
            if (ui->tabWidget->widget(i)->isVisible()) {
                cur_index = i;
                break;
            }
        }
    }
#endif

    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(cur_index);
#ifndef NO_MULTITAB
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [=](int index) {
        if (ui->tabWidget->widget(index) == m_tab_multi) {
            ui->printBtn->setEnabled(false);
            ui->exportBtn->setEnabled(false);
        } else {
            ui->printBtn->setEnabled(true);
            ui->exportBtn->setEnabled(true);
        }
    });
#endif

    m_phaseZoomState = m_settings->value("phaseZoomState", 10).toInt();
    m_rsZoomState = m_settings->value("rsZoomState", 10).toInt();
    m_rpZoomState = m_settings->value("rpZoomState", 10).toInt();
    m_rlZoomState = m_settings->value("rlZoomState", 10).toInt();
    m_tdrZoomState = m_settings->value("tdrZoomState", 10).toInt();
    m_s21ZoomState = m_settings->value("s21ZoomState", 10).toInt();
    m_smithZoomState = m_settings->value("smithZoomState", 10).toInt();
    m_userZoomState = m_settings->value("userZoomState", 10).toInt();
    m_settings->endGroup();

    m_settings->beginGroup("Settings");
    m_fqRestrict = g_developerMode ? m_settings->value("restrictFq", true).toBool() : true;
    g_maxMeasurements = m_settings->value("maxMeasurements", MAX_MEASUREMENTS).toInt();
    m_darkColorTheme = m_settings->value("darkColorTheme", true).toBool();
    m_settings->endGroup();

    if (g_developerMode)
        CustomAnalyzer::load(m_settings);

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

    m_lightPalette = qApp->palette();

    ui->tableWidget_measurments->setColumnCount(MEASUREMENTS_TABLE_COLUMNS);
    ui->tableWidget_measurments->setSelectionBehavior(QAbstractItemView::SelectRows );
    //ui->tableWidget_measurments->setToolTip(tr("Double-click an item to rescale the chart.\nRight-click an item to change color"));
    ui->tableWidget_measurments->setToolTip("");
    ui->tableWidget_measurments->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget_measurments, &QTableWidget::customContextMenuRequested, this, &MainWindow::on_tableWidgetMeasurmentsContextMenu);
    connect(ui->tableWidget_measurments, &QTableWidget::itemChanged, this, [=] (QTableWidgetItem* item) {
        if (item == nullptr)
            return;
        if (item != nullptr && item->column() == COL_VISIBLE) {
            m_measurements->toggleVisibility(item->row(), item->checkState()==Qt::Checked);
        }
    });
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
    connect(m_tdrWidget,SIGNAL(mouseWheel(QWheelEvent*)),this, SLOT(on_mouseWheel_tdr(QWheelEvent*)));

    connect(m_s21Widget,SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(on_mouseMove_s21(QMouseEvent*)));
    connect(m_s21Widget,SIGNAL(mouseWheel(QWheelEvent*)),this, SLOT(on_mouseWheel_s21(QWheelEvent*)));

    connect(m_smithWidget,SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(on_mouseMove_smith(QMouseEvent*)));
    if (g_developerMode) {
        connect(m_userWidget,SIGNAL(mouseWheel(QWheelEvent*)),this, SLOT(on_mouseWheel_user(QWheelEvent*)));
        connect(m_userWidget,SIGNAL(mouseMove(QMouseEvent*)),this, SLOT(on_mouseMove_user(QMouseEvent*)));
    }
    m_analyzer = new AnalyzerPro(this);
    //connect(m_analyzer, &AnalyzerPro::analyzerFound, static_cast<void (MainWindow::*)(int)>(&MainWindow::on_analyzerFound));
    connect(m_analyzer, &AnalyzerPro::analyzerFound,this,&MainWindow::on_analyzerFound);
    connect(m_analyzer,&AnalyzerPro::deviceDisconnected,this, &MainWindow::on_deviceDisconnected);
    connect(this,SIGNAL(measure(qint64,qint64,int)),m_analyzer,SLOT(on_measure(qint64,qint64,int)));
    connect(this,SIGNAL(measureS21(qint64,qint64,int)),m_analyzer,SLOT(on_measureS21(qint64,qint64,int)));
    connect(this,SIGNAL(measureUser(qint64,qint64,int)),m_analyzer,SLOT(on_measureUser(qint64,qint64,int)));
    connect(this,SIGNAL(measureContinuous(qint64,qint64,int)),m_analyzer,SLOT(on_measureContinuous(qint64,qint64,int)));
    connect(m_analyzer,SIGNAL(measurementComplete()),this,SLOT(on_measurementComplete()));//, Qt::QueuedConnection);
    connect(m_analyzer,SIGNAL(measurementCompleteNano()),this,SLOT(on_measurementCompleteNano()));//, Qt::QueuedConnection);
    connect(this,SIGNAL(stopMeasure()), m_analyzer, SLOT(on_stopMeasure()));
    connect(this,&MainWindow::measureOneFq, m_analyzer,&AnalyzerPro::on_measureOneFq);
    connect(m_analyzer, &AnalyzerPro::signalMeasurementError, this, &MainWindow::onMeasurementError);
    connect(m_analyzer, &AnalyzerPro::showNotification, this, &MainWindow::on_showNotification);
    connect(m_analyzer, &AnalyzerPro::signalAnalyzerError, this, [=] (const QString& error) {
        QRect r = ui->tabWidget->rect();
        QRect rr = QRect(40, 50, r.width()-80, 40);
        Notification::showMessage(error, Qt::red, rr, 5000, m_mainWindow);
    });
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

    QShortcut *shortCtrlPlus = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus),this);
    connect(shortCtrlPlus,SIGNAL(activated()),this,SLOT(on_pressCtrlPlus()));
    QShortcut *shortCtrlEqual = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal),this);
    connect(shortCtrlEqual,SIGNAL(activated()),this,SLOT(on_pressCtrlPlus()));
    QShortcut *shortCtrlUp = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up),this);
    connect(shortCtrlUp,SIGNAL(activated()),this,SLOT(on_pressCtrlPlus()));
    QShortcut *shortCtrlMinus = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus),this);
    connect(shortCtrlMinus,SIGNAL(activated()),this,SLOT(on_pressCtrlMinus()));
    QShortcut *shortCtrlDoun = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down),this);
    connect(shortCtrlDoun,SIGNAL(activated()),this,SLOT(on_pressCtrlMinus()));
    QShortcut *shortCtrlIns = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Asterisk),this);
    connect(shortCtrlIns,SIGNAL(activated()),this,SLOT(on_pressCtrlZero()));
    QShortcut *shortCtrlZero = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_0),this);
    connect(shortCtrlZero,SIGNAL(activated()),this,SLOT(on_pressCtrlZero()));

    QShortcut *shortLeft = new QShortcut(QKeySequence(Qt::Key_Left),this);
    connect(shortLeft,SIGNAL(activated()),this,SLOT(on_pressLeft()));

    QShortcut *shortRight = new QShortcut(QKeySequence(Qt::Key_Right),this);
    connect(shortRight,SIGNAL(activated()),this,SLOT(on_pressRight()));

    QShortcut *shortCtrlC = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_C),this);
    connect(shortCtrlC,SIGNAL(activated()),this,SLOT(on_pressCtrlC()));

    QShortcut *shortCtrlAltShiftM = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::Key_M),this);
    connect(shortCtrlAltShiftM,SIGNAL(activated()),this,SLOT(on_presssCtrlAltShiftM()));

    QShortcut *shortCtrlAltShiftN = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::Key_N),this);
    connect(shortCtrlAltShiftN,SIGNAL(activated()),this,SLOT(on_presssCtrlAltShiftN()));

    m_presets = new Presets(this);
    connect(this, SIGNAL(isRangeChanged(bool)), m_presets, SLOT(on_isRangeChanged(bool)));
    m_presets->setTable(ui->tableWidget_presets);

    m_measurements = new Measurements (this);
    connect(this, SIGNAL(isRangeChanged(bool)), m_measurements, SLOT(on_isRangeChanged(bool)));
    m_measurements->setWidgets(m_swrWidget,
                               m_phaseWidget,
                               m_rsWidget,
                               m_rpWidget,
                               m_rlWidget,
                               m_tdrWidget,
                               m_s21Widget,
                               m_smithWidget,
                               ui->tableWidget_measurments);
    m_measurements->setUserWidget(m_userWidget);

    connect(m_analyzer, &AnalyzerPro::newData, m_measurements, &Measurements::on_newDataRedraw);
    connect(m_analyzer, &AnalyzerPro::newS21Data, m_measurements, &Measurements::on_newS21Data);
    connect(m_analyzer, &AnalyzerPro::newAnalyzerData, m_measurements, &Measurements::on_newAnalyzerData);
    connect(m_analyzer, &AnalyzerPro::newUserData, m_measurements, &Measurements::on_newUserData);
    connect(m_analyzer, &AnalyzerPro::newUserDataHeader, m_measurements, &Measurements::on_newUserDataHeader);
    connect(m_analyzer, SIGNAL(newMeasurement(QString)), m_measurements, SLOT(on_newMeasurement(QString)));
    connect(m_analyzer, SIGNAL(newMeasurement(QString, qint64, qint64, qint32)), m_measurements, SLOT(on_newMeasurement(QString, qint64, qint64, qint32)));
    connect(m_analyzer, &AnalyzerPro::continueMeasurement, m_measurements, &Measurements::on_continueMeasurement);
    connect(this, &MainWindow::currentTab, m_measurements, &Measurements::on_currentTab);
    connect(this, &MainWindow::focus, m_measurements, &Measurements::on_focus);
    connect(this, &MainWindow::newCursorFq, m_measurements, &Measurements::on_newCursorFq);
    connect(this, &MainWindow::newCursorSmithPos, m_measurements, &Measurements::on_newCursorSmithPos);
    connect(this, &MainWindow::mainWindowPos, m_measurements, &Measurements::on_mainWindowPos);
    connect(this, &MainWindow::measureOneFq, m_measurements, &Measurements::on_newMeasurementOneFq);
    connect(m_measurements, SIGNAL(calibrationChanged()), this,SLOT(on_calibrationChanged()));
    connect(m_measurements, &Measurements::import_finished, this, &MainWindow::on_importFinished);
    connect(m_measurements, &Measurements::measurementCanceled, this, &MainWindow::stopMeasure);
    connect(m_measurements, &Measurements::oneFqCanceled, this, &MainWindow::on_pressEsc);
    connect(m_measurements, &Measurements::selectMeasurement, this, &MainWindow::on_tableWidget_measurments_cellClicked);

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
                              m_s21Widget,
                              m_smithWidget);
        m_markers->setMeasurements(m_measurements);
        connect(this, SIGNAL(focus(bool)), m_markers,SLOT(on_focus(bool)));
        connect(this, SIGNAL(mainWindowPos(int,int)), m_markers,SLOT(on_mainWindowPos(int,int)));
        connect(this, SIGNAL(currentTab(QString)), m_markers, SLOT(on_currentTab(QString)));
        connect(this, SIGNAL(rescale()), m_markers, SLOT(rescale()));
        connect(m_analyzer, SIGNAL(newMeasurement(QString)), m_markers, SLOT(on_newMeasurement(QString)));
        connect(m_analyzer, SIGNAL(measurementComplete()), m_markers, SLOT(on_measurementComplete()));
    }

    changeColorTheme(m_darkColorTheme);

    m_calibration = new Calibration();
    m_calibration->setAnalyzer(m_analyzer);
    m_calibration->start();
    connect(m_calibration,SIGNAL(setCalibrationMode(bool)),
            m_analyzer,SLOT(setCalibrationMode(bool)));
    connect(m_calibration,SIGNAL(setCalibrationMode(bool)),
            m_measurements,SLOT(setCalibrationMode(bool)));
    m_measurements->setCalibration(m_calibration);

    ui->checkBoxCalibration->blockSignals(true);
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
    m_lastSaveOpenPath = m_settings->value("lastSavePath", "").toString();
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
    if (g_developerMode) {
        m_userWidget->xAxis->setRange(range);
    }
    m_isRange = m_settings->value("isRange", false).toBool();
    emit isRangeChanged(m_isRange);

    if(!m_isRange)
    {
        ui->limitsBtn->setChecked(true);
        on_limitsBtn_clicked(true);
//        ui->rangeBtn->setChecked(false);
        m_lastEnteredFqFrom = range.lower;
        m_lastEnteredFqTo = range.upper;
        setFqFrom(range.lower);
        setFqTo(range.upper);
    }else
    {
        ui->rangeBtn->setChecked(true);
        on_rangeBtn_clicked(true);
//        ui->rangeBtn->setChecked(true);
//        ui->limitsBtn->setChecked(false);
        m_lastEnteredFqFrom = (range.upper + range.lower)/2;
        m_lastEnteredFqTo = (range.upper - range.lower)/2;
        setFqFrom((range.upper + range.lower)/2);
        setFqTo((range.upper - range.lower)/2);
    }

    ui->spinBoxPoints->setValue(m_dotsNumber);
    connect(ui->spinBoxPoints, SIGNAL(valueChanged(int)), this, SLOT(onSpinChanged(int)));
    connect(ui->fullBtn, &QPushButton::clicked, this, &MainWindow::onFullRange);

    m_autoFirmwareUpdateEnabled = m_settings->value("autoFirmwareUpdate", true).toBool();
    m_autoUpdateEnabled = m_settings->value("autoUpdate", true).toBool();

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
    QTimer::singleShot(100, this, [this](){
        updateGraph();
    });

    QTimer::singleShot(100, this, [this](){
        newSoftwareRequest();
    });

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
    }else
    {
        ui->groupBox_Presets->setTitle(tr("Presets (center, range), kHz"));
        ui->tableWidget_presets->horizontalHeaderItem(0)->setText(tr("Center"));
        ui->tableWidget_presets->horizontalHeaderItem(1)->setText(tr("Range(+/-)"));
    }

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

    connect(ui->labelMarquee, &MarqueeLabel::clicked, this, [=] (const QString& link) {
        QDesktopServices::openUrl(QUrl(link));
    });
    ui->labelMarquee->hide();

#ifdef _DEBUG
//{ test marquee
//    QTimer::singleShot(3000, this, [=](){
//        ui->labelMarquee->show();
//        QString fname = "c:\\Downloads\\AntScope_scroll.json";
//        ui->labelMarquee->load(fname);
//    });
//}
#endif

    m_settings->beginGroup("Connection");
    bool start = m_settings->value("same", false).toBool();
    ReDeviceInfo::InterfaceType _type = (ReDeviceInfo::InterfaceType)m_settings->value("type", ReDeviceInfo::HID).toInt();
    QString device_name = m_settings->value("name", "").toString();
    QString device_address = m_settings->value("id", "").toString();
    m_settings->endGroup();

    SelectionParameters::selected.type = (ReDeviceInfo::InterfaceType)_type;
    SelectionParameters::selected.name = device_name;
    SelectionParameters::selected.id = device_address;

    if (start && SelectionParameters::selected.valid() && !device_name.isEmpty() && !device_address.isEmpty()) {
//        SelectDeviceDialog dlg(true, this);
//        if (dlg.connectSilent(_type, device_name)) {
//            AnalyzerParameters* selected = AnalyzerParameters::current();
//            if (selected != nullptr) {
//                m_analyzer->on_connectDevice();
//            }
//        }
        QTimer::singleShot(500, this, [&](){
            on_refreshConnection();
        });
    } else {
        QTimer::singleShot(500, this, [&](){
            on_selectDeviceDialog();
        });
    }
}

MainWindow::~MainWindow()
{
    QList<QStringList*> values = m_BandsMap.values();
    while (!values.isEmpty()) {
        QStringList* lst = values.takeLast();
        delete lst;
    }
    m_BandsMap.clear();

    delete m_calibration;
    delete m_updater;
    delete m_updateDialog;

    m_settings->beginGroup("MainWindow");
    m_settings->setValue("geometry", this->geometry());
    m_settings->setValue("fullScreen", this->isMaximized());
    m_settings->setValue("dotsNumber", this->m_dotsNumber);

    m_settings->setValue("lastSavePath", m_lastSaveOpenPath);
    m_settings->setValue("measureSystemMetric", m_measureSystemMetric);
    m_settings->setValue("isRange", m_isRange);

    QString str;
//    str.append(QString::number(ui->tabWidget->indexOf(m_tab_swr)));
//    str.append(QString::number(ui->tabWidget->indexOf(m_tab_phase)));
//    str.append(QString::number(ui->tabWidget->indexOf(m_tab_rs)));
//    str.append(QString::number(ui->tabWidget->indexOf(m_tab_rp)));
//    str.append(QString::number(ui->tabWidget->indexOf(m_tab_rl)));
//    str.append(QString::number(ui->tabWidget->indexOf(m_tab_tdr)));
//    str.append(QString::number(ui->tabWidget->indexOf(m_tab_smith)));
//    str.append(QString::number(ui->tabWidget->indexOf(m_tab_user)));
//    str.append(QString::number(ui->tabWidget->indexOf(m_tab_multi)));
//    m_settings->setValue("tabSequence", str);

    for (int i=0; i<ui->tabWidget->count(); i++) {
        str += ui->tabWidget->widget(i)->objectName() + ",";
    }
    m_settings->setValue("tabOrder", str);

#ifndef NO_MULTITAB
    QString str_multi;
    foreach (auto tab, m_multiTabData.tabs) {
        str_multi += tab + ",";
    }
    m_settings->setValue("multiTab", str_multi);
#endif

    m_settings->setValue("currentTab",ui->tabWidget->currentIndex());
    m_settings->setValue("systemImpedance", m_Z0);
    m_settings->setValue("rangeLower", m_swrWidget->xAxis->range().lower);
    m_settings->setValue("rangeUpper", m_swrWidget->xAxis->range().upper);
    m_settings->setValue("autoFirmwareUpdate", m_autoFirmwareUpdateEnabled);
    m_settings->setValue("autoUpdate", m_autoUpdateEnabled);

    //m_settings->setValue("swrZoomState", m_swrZoomState);
    m_settings->setValue("phaseZoomState", m_phaseZoomState);
    m_settings->setValue("rsZoomState", m_rsZoomState);
    m_settings->setValue("rpZoomState", m_rpZoomState);
    m_settings->setValue("rlZoomState", m_rlZoomState);
    m_settings->setValue("tdrZoomState", m_tdrZoomState);
    m_settings->setValue("s21ZoomState", m_s21ZoomState);
    m_settings->setValue("smithZoomState", m_smithZoomState);
    m_settings->setValue("userZoomState", m_userZoomState);

    m_settings->setValue("languageNumber", m_languageNumber);

    m_settings->endGroup();

    m_settings->beginGroup("Settings");
    m_settings->setValue("restrictFq", m_fqRestrict);
    m_settings->setValue("maxMeasurements", g_maxMeasurements);
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
        default:break; // ignore all others
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
/*
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
*/
    QMainWindow::closeEvent(event);
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
    pen.setColor(QColor(255, 255, 255, 150));
    pen.setWidthF(INACTIVE_GRAPH_PEN_WIDTH);

    QFont fontTickLabel = m_swrWidget->xAxis->tickLabelFont();
    QFont fontLabel = fontTickLabel;
    fontTickLabel.setPointSize(11);
    fontLabel.setPointSize(12);

    bool bands_loaded = loadBands();
    QStringList* bands = nullptr;
    QString band;
    if (bands_loaded)
    {
        m_settings->beginGroup("Settings");
        band = m_settings->value("current_band", "ITU Region 1 - Europe, Africa").toString();
        m_settings->endGroup();
        if (m_BandsMap.contains(band))
        {
            bands = m_BandsMap[band];
        }
    }

    //-------SWR Widget---------------------------------------------
    qobject_cast<CustomPlot*>(m_swrWidget)->addGraph();//graph(0) - SWR
    setBands(m_swrWidget, bands, MIN_SWR, MAX_SWR);
    m_swrWidget->graph(0)->setPen(pen);
    m_swrWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_swrWidget->yAxis->setLabel(tr("SWR"));
    m_swrWidget->xAxis->setRange(0,1400000);
    m_swrWidget->yAxis->setRangeMin(MIN_SWR);
    m_swrWidget->yAxis->setRangeMax(MAX_SWR);
    m_swrWidget->yAxis->setRangeLower(MIN_SWR);
    //m_swrWidget->yAxis->setRangeUpper(m_swrZoomState+0.02);
    m_swrWidget->yAxis->setRangeUpper(10.02);
    m_swrWidget->yAxis->setNumberPrecision(2);
    m_swrWidget->yAxis->setAutoTicks(true);
    m_swrWidget->yAxis->setAutoSubTicks(true);
    m_swrWidget->yAxis->setTickStep(0.1);
    m_swrWidget->yAxis->setAutoTickCount(8);
    m_swrWidget->yAxis->setTickLength(8, 0);
    m_swrWidget->yAxis->setSubTickLength(4, 0);

     //| Qt::Vertical | Qt::Horizontal
    m_swrWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_swrWidget->axisRect()->setRangeZoom(Qt::Horizontal);// | Qt::Vertical);
    m_swrWidget->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_swrWidget->xAxis->setTickLabelFont(fontTickLabel);
    m_swrWidget->yAxis->setTickLabelFont(fontTickLabel);
    m_swrWidget->xAxis->setLabelFont(fontLabel);
    m_swrWidget->yAxis->setLabelFont(fontLabel);
    on_bandChanged(band);
    m_swrWidget->replot();

    //-------Phase Widget---------------------------------------------
    qobject_cast<CustomPlot*>(m_phaseWidget)->addGraph();//graph(0)
    setBands(m_phaseWidget, bands, -180, 180);
    m_phaseWidget->graph(0)->setPen(pen);
    m_phaseWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_phaseWidget->yAxis->setLabel(tr("Phase, Angle"));
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
    qobject_cast<CustomPlot*>(m_rsWidget)->addGraph();//graph(0)
    m_rsWidget->setAutoAddPlottableToLegend(false);
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
    qobject_cast<CustomPlot*>(m_rpWidget)->addGraph();//graph(0)
    m_rpWidget->setAutoAddPlottableToLegend(false);
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
    qobject_cast<CustomPlot*>(m_rlWidget)->addGraph();//graph(0)
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
    qobject_cast<CustomPlot*>(m_tdrWidget)->addGraph();//graph(0)
    //m_tdrWidget->setAutoAddPlottableToLegend(false);
    m_tdrWidget->graph(0)->setPen(pen);
    m_tdrWidget->xAxis->setLabel(tr("Length, m"));
    m_tdrWidget->xAxis->setRangeLower(0);
    m_tdrWidget->xAxis->setRangeUpper(1000);
    m_tdrWidget->yAxis->setRangeMin(-1);
    m_tdrWidget->yAxis->setRangeMax(1);
    m_tdrWidget->yAxis->setRange(-1,1);
    m_tdrWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_tdrWidget->axisRect()->setRangeZoom(Qt::Horizontal);
    m_tdrWidget->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_tdrWidget->xAxis->setTickLabelFont(fontTickLabel);
    m_tdrWidget->yAxis->setTickLabelFont(fontTickLabel);
    m_tdrWidget->xAxis->setLabelFont(fontLabel);
    m_tdrWidget->yAxis->setLabelFont(fontLabel);
    m_tdrWidget->yAxis->setLabel(tr("SR/IR"));
    m_tdrWidget->yAxis2->setVisible(true);
    m_tdrWidget->yAxis2->setTickLabelFont(fontTickLabel);
    m_tdrWidget->yAxis2->setLabelFont(fontLabel);
    m_tdrWidget->yAxis2->setRangeMin(0);
    m_tdrWidget->yAxis2->setRangeMax(5000);
    m_tdrWidget->yAxis2->setRange(0, 5000);
    m_tdrWidget->yAxis2->setLabel(tr("|Z|"));
    m_tdrWidget->replot();

    //-------S21 Widget------------------------------------------------
    qobject_cast<CustomPlot*>(m_s21Widget)->addGraph();//graph(0)
    m_s21Widget->graph(0)->setPen(pen);
    m_s21Widget->xAxis->setLabel(tr("Frequency, kHz"));
    m_s21Widget->xAxis->setRangeLower(0);
    m_s21Widget->xAxis->setRangeUpper(1000);
    m_s21Widget->yAxis->setRangeMin(-200);
    m_s21Widget->yAxis->setRangeMax(0);
    m_s21Widget->yAxis->setRange(-200,0);
    m_s21Widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_s21Widget->axisRect()->setRangeZoom(Qt::Horizontal);
    m_s21Widget->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    m_s21Widget->xAxis->setTickLabelFont(fontTickLabel);
    m_s21Widget->yAxis->setTickLabelFont(fontTickLabel);
    m_s21Widget->xAxis->setLabelFont(fontLabel);
    m_s21Widget->yAxis->setLabelFont(fontLabel);
    m_s21Widget->yAxis->setLabel(tr("S21, dB"));
    m_s21Widget->yAxis2->setTickLabelFont(fontTickLabel);
    m_s21Widget->yAxis2->setLabelFont(fontLabel);
    m_s21Widget->yAxis2->setRangeMin(0);
    m_s21Widget->yAxis2->setRangeMax(3);
    m_s21Widget->yAxis2->setRange(0, 3);
    m_s21Widget->yAxis2->setLabel(tr("Stage"));
    m_s21Widget->yAxis2->setVisible(g_developerMode);
    m_s21Widget->replot();

    //-------Smith Widget---------------------------------------------
    m_smithWidget->addGraph();//graph(0)
    m_smithWidget->xAxis->setRangeMin(-10);
    m_smithWidget->xAxis->setRangeMax(10);
    m_smithWidget->xAxis->setRange(-7,7);
    m_smithWidget->yAxis->setRangeMin(-10);
    m_smithWidget->yAxis->setRangeMax(10);
    m_smithWidget->yAxis->setRange(-7,7);
    m_smithWidget->replot();

    //---------User defined
    if (g_developerMode) {
        m_userWidget->setAutoAddPlottableToLegend(false);
        qobject_cast<CustomPlot*>(m_userWidget)->addGraph();//graph(0)
        setBands(m_userWidget, bands, MIN_USER_RANGE, MAX_USER_RANGE);
        m_userWidget->graph(0)->setPen(pen);
        m_userWidget->xAxis->setLabel(tr("Frequency, kHz"));
        m_userWidget->yAxis->setLabel(tr("User defined"));
        m_userWidget->xAxis->setRange(0,1400000);
        m_userWidget->yAxis->setRangeMin(MIN_USER_RANGE);
        m_userWidget->yAxis->setRangeMax(MAX_USER_RANGE);
        m_userWidget->yAxis->setRange(-m_userZoomState*80,m_userZoomState*80);
        m_userWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        m_userWidget->axisRect()->setRangeZoom(Qt::Horizontal);
        m_userWidget->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
        m_userWidget->xAxis->setTickLabelFont(fontTickLabel);
        m_userWidget->yAxis->setTickLabelFont(fontTickLabel);
        m_userWidget->xAxis->setLabelFont(fontLabel);
        m_userWidget->yAxis->setLabelFont(fontLabel);
        m_userWidget->replot();
    }
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
    m_settings->beginGroup("Settings");
    bool showName = m_settings->value("show-band-name", false).toBool();
    m_settings->endGroup();
    foreach (QString str, *bands)
    {
        QStringList list = str.split(',');
        if (list.size() == 2 || !showName)
        {
            addBand(widget, list[0].toDouble(), list[1].toDouble(), y1, y2);
        } else if (list.size() == 3) {
            addBand(widget, list[0].toDouble(), list[1].toDouble(), y1, y2, list[2]);
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

void MainWindow::addBand (QCustomPlot * widget, double x1, double x2, double y1, double y2, QString& name)
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

    if (!name.isEmpty()) {
        QRectF rr(QPointF(x1, y1), QPointF(x2, y2));
        QPointF pt = rr.center();
        QCPItemText* textItem = new QCPItemText( widget );
        textItem->setColor(QColor(50,50,150,150));
        textItem->setPen(Qt::NoPen);
        textItem->setText(name);
        textItem->position->setCoords(pt.x(), pt.y());
        textItem->setPositionAlignment(Qt::AlignHCenter);
        textItem->setRotation(270);

        m_itemRectList.append(textItem);
    }

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

    m_measurements->hideOneFqWidget();
    m_measurements->interrupt();
    emit stopMeasure();
}

void MainWindow::on_pressF9 ()
{
    if (!ui->singleStart->isEnabled())
        return;
    emit on_singleStart_clicked();
}

void MainWindow::on_pressF10 ()
{
    if (!ui->continuousStartBtn->isEnabled())
        return;
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
    if( str == "tab_swr")
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
        m_s21Widget->xAxis->setRange(m_swrWidget->xAxis->range());
        if (g_developerMode) {
            m_userWidget->xAxis->setRange(m_swrWidget->xAxis->range());
        }
        m_swrWidget->replot();
    }else if(str == "tab_phase")
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
        m_s21Widget->xAxis->setRange(m_phaseWidget->xAxis->range());
        if (g_developerMode) {
            m_userWidget->xAxis->setRange(m_phaseWidget->xAxis->range());
        }
        m_phaseWidget->replot();
    }else if(str == "tab_rs")
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
        m_s21Widget->xAxis->setRange(m_rsWidget->xAxis->range());
        if (g_developerMode) {
            m_userWidget->xAxis->setRange(m_rsWidget->xAxis->range());
        }
        m_rsWidget->replot();
    }else if(str == "tab_rp")
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
        m_s21Widget->xAxis->setRange(m_rpWidget->xAxis->range());
        if (g_developerMode) {
            m_userWidget->xAxis->setRange(m_rpWidget->xAxis->range());
        }
        m_rpWidget->replot();
    }else if(str == "tab_rl")
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
        m_rlWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_s21Widget->xAxis->setRange(m_userWidget->xAxis->range());
        if (g_developerMode) {
            m_userWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        }
        m_rlWidget->replot();
    }else if(str == "tab_tdr")
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
    }else if(str == "tab_s21")
    {
        double from = m_s21Widget->xAxis->getRangeLower();
        double to = m_s21Widget->xAxis->getRangeUpper();
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
        m_s21Widget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_s21Widget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_s21Widget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_s21Widget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_s21Widget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_s21Widget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_s21Widget->xAxis->range());
        if (g_developerMode) {
            m_userWidget->xAxis->setRange(m_s21Widget->xAxis->range());
        }
        m_s21Widget->replot();
    }else if(str == "tab_user")
    {
        double from = m_userWidget->xAxis->getRangeLower();
        double to = m_userWidget->xAxis->getRangeUpper();
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
        m_userWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_s21Widget->xAxis->setRange(m_userWidget->xAxis->range());
        m_userWidget->replot();
    }
}

void MainWindow::on_pressCtrlPlus ()
{    
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_swr")
    {
        QPointF pos = rect().center();
        QPoint delta = QPoint(0,1);
        Qt::ScrollPhase phase = Qt::NoScrollPhase;        QWheelEvent event(pos, pos, delta, delta, Qt::NoButton, Qt::ControlModifier, phase, false);
        on_mouseWheel_swr(&event);
//        int limit = g_developerMode ? 1 : SWR_ZOOM_LIMIT;
//        if(m_swrZoomState > limit)
//        {
//            m_swrZoomState = m_swrZoomState - m_swrZoomState/10;
//            m_swrWidget->yAxis->setRangeUpper(m_swrZoomState+0.02);
//            m_swrWidget->yAxis->setRangeLower(MIN_SWR);
//            m_swrWidget->replot();
//            if(m_markers)
//            {
//                QTimer::singleShot(5, m_markers, SLOT(redraw()));
//            }
//            if(m_measurements)
//            {
//                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
//            }
//        }

    }else if(str == "tab_phase")
    {
    }else if(str == "tab_rs")
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
    }else if(str == "tab_rp")
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
    }else if(str == "tab_rl")
    {
        int limit = g_developerMode ? 1 : SWR_ZOOM_LIMIT;
        if(m_rlZoomState > limit)
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
    }else if(str == "tab_tdr")
    {
        m_tdrWidget->replot();
    }else if(str == "tab_s21")
    {
        int limit = g_developerMode ? 1 : SWR_ZOOM_LIMIT;
        if(m_s21ZoomState > limit)
        {
            --m_s21ZoomState;
            m_s21Widget->yAxis->setRangeUpper(m_s21ZoomState*5);
            m_s21Widget->yAxis->setRangeLower(0);
            if(m_markers)
            {
                QTimer::singleShot(5, m_markers, SLOT(redraw()));
            }
            if(m_measurements)
            {
                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
            }
        }
    }else if(str == "tab_user")
    {
        if(m_userZoomState > 1)
        {
            --m_userZoomState;
            int val = m_userZoomState*80;
            m_userWidget->yAxis->setRangeLower(-val);
            m_userWidget->yAxis->setRangeUpper(val);
            m_userWidget->replot();
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

void MainWindow::on_pressMinus ()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_swr")
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
    }else if(str == "tab_phase")
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
    }else if(str == "tab_rs")
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
    }else if(str == "tab_rp")
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
    }else if(str == "tab_rl")
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
    }else if(str == "tab_tdr")
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
    }else if(str == "tab_s21")
    {
        double from = m_s21Widget->xAxis->getRangeLower();
        double to = m_s21Widget->xAxis->getRangeUpper();
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
        m_s21Widget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_s21Widget->xAxis->setRangeUpper(to);

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
        m_rlWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_s21Widget->replot();
    }else if(str == "tab_user")
    {
        double from = m_userWidget->xAxis->getRangeLower();
        double to = m_userWidget->xAxis->getRangeUpper();
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
        m_userWidget->xAxis->setRangeLower(from);

        if ((center + band) < ABSOLUTE_MAX_FQ)
        {
            to = center + band;
        }else
        {
            to = ABSOLUTE_MAX_FQ;
        }
        m_userWidget->xAxis->setRangeUpper(to);

        if(!m_isRange)
        {
            setFqFrom(from);
            setFqTo(to);
        }else
        {
            setFqFrom((to+from)/2);
            setFqTo((to-from)/2);
        }

        m_swrWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_userWidget->replot();

    }
}

void MainWindow::on_pressCtrlMinus ()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_swr")
    {
        QPointF pos = rect().center();
        QPoint delta = QPoint(0,-1);
        Qt::ScrollPhase phase = Qt::NoScrollPhase;
        QWheelEvent event(pos, pos, delta, delta, Qt::NoButton, Qt::ControlModifier, phase, false);

        on_mouseWheel_swr(&event);
        on_mouseWheel_swr(&event);
    }else if(str == "tab_phase")
    {
//        m_phaseWidget->replot();
    }else if(str == "tab_rs")
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
    }else if(str == "tab_rp")
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
    }else if(str == "tab_rl")
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
    }else if(str == "tab_tdr")
    {
        m_tdrWidget->replot();
    }else if(str == "tab_s21")
    {
        if(m_s21ZoomState <= 9)
        {
            ++m_s21ZoomState;
            m_s21Widget->yAxis->setRangeUpper(m_s21ZoomState*5);
            m_s21Widget->yAxis->setRangeLower(0);
            m_s21Widget->replot();
            if(m_markers)
            {
                QTimer::singleShot(5, m_markers, SLOT(redraw()));
            }
            if(m_measurements)
            {
                QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
            }
        }
    }else if(str == "tab_user")
    {
        if(m_userZoomState < 19)
        {
            ++m_userZoomState;
            int val = m_userZoomState*80;
            m_userWidget->yAxis->setRangeLower(-val);
            m_userWidget->yAxis->setRangeUpper(val);
            m_userWidget->replot();
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

void MainWindow::on_pressCtrlZero()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_swr")
    {
        if (g_developerMode) {

            //m_swrZoomState = 10;
            //m_swrWidget->yAxis->setRangeUpper(m_swrZoomState+0.02);
            m_swrWidget->yAxis->setRangeUpper(10.02);
            m_swrWidget->yAxis->setRangeLower(MIN_SWR);
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
    }else if(str == "tab_phase")
    {
//        m_phaseWidget->replot();
    }else if(str == "tab_rs")
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
    }else if(str == "tab_rp")
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
    }else if(str == "tab_rl")
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
    }else if(str == "tab_rl")
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
    }else if(str == "tab_s21")
    {
        m_s21Widget->yAxis->setRangeUpper(m_s21ZoomState*5);
        m_s21Widget->yAxis->setRangeLower(0);
        m_s21Widget->replot();
        if(m_markers)
        {
            QTimer::singleShot(5, m_markers, SLOT(redraw()));
        }
        if(m_measurements)
        {
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
    }else if(str == "tab_tdr")
    {
        m_tdrWidget->replot();
    }else if(str == "tab_user")
    {
        int val = m_userZoomState*80;
        m_userWidget->yAxis->setRangeLower(-val);
        m_userWidget->yAxis->setRangeUpper(val);
        m_userWidget->replot();
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

void MainWindow::on_pressLeft()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_swr")
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
        m_s21Widget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_swrWidget->replot();
    }else if(str == "tab_phase")
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
        m_s21Widget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_phaseWidget->replot();
    }else if(str == "tab_rs")
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
        m_s21Widget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rsWidget->replot();
    }else if(str == "tab_rp")
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
        m_s21Widget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rpWidget->replot();
    }else if(str == "tab_rl")
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
        m_s21Widget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rlWidget->replot();
    }else if(str == "tab_s21")
    {
        double from = m_s21Widget->xAxis->getRangeLower();
        double to = m_s21Widget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((from - diff) >= ABSOLUTE_MIN_FQ)
        {
            from -= diff;
            to -= diff;
            m_s21Widget->xAxis->setRangeLower(from);
            m_s21Widget->xAxis->setRangeUpper(to);
        }else
        {
            from = ABSOLUTE_MIN_FQ;
            m_s21Widget->xAxis->setRangeLower(from);
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
        m_s21Widget->replot();
    }else if(str == "tab_tdr")
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
    }else if(str == "tab_user")
    {
        double from = m_userWidget->xAxis->getRangeLower();
        double to = m_userWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((from - diff) >= ABSOLUTE_MIN_FQ)
        {
            from -= diff;
            to -= diff;
            m_userWidget->xAxis->setRangeLower(from);
            m_userWidget->xAxis->setRangeUpper(to);
        }else
        {
            from = ABSOLUTE_MIN_FQ;
            m_userWidget->xAxis->setRangeLower(from);
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

        m_swrWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_s21Widget->xAxis->setRange(m_userWidget->xAxis->range());
        m_userWidget->replot();
    }
}

void MainWindow::on_pressRight()
{
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_swr")
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
        m_s21Widget->xAxis->setRange(m_swrWidget->xAxis->range());
        m_swrWidget->replot();
    }else if(str == "tab_phase")
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
        m_s21Widget->xAxis->setRange(m_phaseWidget->xAxis->range());
        m_phaseWidget->replot();
    }else if(str == "tab_rs")
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
        m_s21Widget->xAxis->setRange(m_rsWidget->xAxis->range());
        m_rsWidget->replot();
    }else if(str == "tab_rp")
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
        m_s21Widget->xAxis->setRange(m_rpWidget->xAxis->range());
        m_rpWidget->replot();
    }else if(str == "tab_rl")
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
        m_s21Widget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rlWidget->replot();
    }else if(str == "tab_s21")
    {
        double from = m_s21Widget->xAxis->getRangeLower();
        double to = m_s21Widget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((to + diff) <= ABSOLUTE_MAX_FQ)
        {
            from += diff;
            to += diff;
            m_s21Widget->xAxis->setRangeLower(from);
            m_s21Widget->xAxis->setRangeUpper(to);
        }else
        {
            to = ABSOLUTE_MAX_FQ;
            m_s21Widget->xAxis->setRangeUpper(to);
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
        m_rpWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_rlWidget->xAxis->range());
        m_s21Widget->replot();
    }else if(str == "tab_tdr")
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
    }else if(str == "tab_user")
    {
        double from = m_userWidget->xAxis->getRangeLower();
        double to = m_userWidget->xAxis->getRangeUpper();
        double diff = (to - from)/10;

        if((to + diff) <= ABSOLUTE_MAX_FQ)
        {
            from += diff;
            to += diff;
            m_userWidget->xAxis->setRangeLower(from);
            m_userWidget->xAxis->setRangeUpper(to);
        }else
        {
            to = ABSOLUTE_MAX_FQ;
            m_userWidget->xAxis->setRangeUpper(to);
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

        m_swrWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_userWidget->replot();
    }
}

void MainWindow::on_pressCtrlC ()
{
    QCustomPlot* plot = nullptr;
    QString str = ui->tabWidget->currentWidget()->objectName();
    if( str == "tab_swr")
    {
        plot = m_swrWidget;
    }else if(str == "tab_phase")
    {
        plot = m_phaseWidget;
    }else if(str == "tab_rs")
    {
        plot = m_rsWidget;
    }else if(str == "tab_rp")
    {
        plot = m_rpWidget;
    }else if(str == "tab_rl")
    {
        plot = m_rlWidget;
    }else if(str == "tab_s21")
    {
        plot = m_s21Widget;
    }else if(str == "tab_tdr")
    {
        plot = m_tdrWidget;
    }else if(str == "tab_smith")
    {
        resizeWnd();
        plot = m_smithWidget;
    }else if(str == "tab_user")
    {
        plot = m_userWidget;
    }

    QPixmap pixmap = plot->grab();

    QPainter painter(&pixmap);
    QPixmap logo(":/new/prefix1/logo_watermark.png");
    painter.drawPixmap(10, 10, logo);

    QFont font = painter.font();
    font.setFamily("Courier New");
    painter.setFont(font);

    QString text = "RigExpert AntScope: antenna and cable analysis software";
    painter.setPen(qRgb(0x55, 0x7b, 0xce));
    QRect bound = painter.boundingRect(pixmap.rect(), Qt::AlignBottom|Qt::AlignRight, text);
    painter.drawText(bound.left()-2, bound.bottom()-1, text);

    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->setPixmap(pixmap);
}

void MainWindow::on_analyzerFound(int index)
{
    QString name = AnalyzerParameters::byIndex(index)->name();
    on_analyzerNameFound(name);
    m_analyzer->applyAnalyzer();
}

void MainWindow::on_analyzerNameFound(QString name)
{
    QString name1 = "AntScope2 v." + QString(ANTSCOPE2VER);
    setWindowTitle(name1 + " - " + name);

    bool zeroII = name.contains("Zero II");
    ui->singleStart->setEnabled(true);
    ui->continuousStartBtn->setEnabled(true);
    if (g_bAA55modeNewProtocol) {
        ui->analyzerDataBtn->setEnabled(true);
        ui->screenshotAA->setEnabled(false);
    } else if (!NanovnaAnalyzer::isConnected() && !zeroII) {
        ui->analyzerDataBtn->setEnabled(true);
        ui->screenshotAA->setEnabled(true);
    } else {
        ui->analyzerDataBtn->setEnabled(false);
        ui->screenshotAA->setEnabled(false);
    }

    if (!g_developerMode) {
        ui->labelMarquee->request();
        QTimer::singleShot(1000, this, [=]() {
            newSoftwareRequest();
        });
        QTimer::singleShot(1100, this, [=]() {
            m_analyzer->checkFirmwareUpdate();
        });
    } else {
        QString json_path = Settings::localDataFolder();
        QDir dir(json_path);
        QDir json_dir = dir.absoluteFilePath("RigExpert/AntScope2/");
        QStringList filters;
        filters << "*.json";
        QFileInfoList fil = json_dir.entryInfoList(filters, QDir::Files, QDir::Name);
        if (!fil.isEmpty()) {
            QString fname = fil[0].absoluteFilePath();
            ui->labelMarquee->load(fname);
            ui->labelMarquee->show();
        }
        QTimer::singleShot(1000, this, [=]() {
            newSoftwareRequest();
        });
        QTimer::singleShot(1100, this, [=]() {
            m_analyzer->checkFirmwareUpdate();
        });
    }

    if (name.contains("NanoVNA", Qt::CaseInsensitive)) {
        ui->spinBoxPoints->setEnabled(false);
        ui->spinBoxPoints->setValue(100);
    } else {
        ui->spinBoxPoints->setEnabled(true);
    }
}

void MainWindow::on_deviceDisconnected()
{
    QWidget::setCursor(Qt::ArrowCursor);
    QString name = "AntScope2 v." + QString(ANTSCOPE2VER);
    name += tr(" - Analyzer not connected");
    setWindowTitle(name);
    ui->singleStart->setEnabled(false);
    ui->continuousStartBtn->setEnabled(false);
    ui->analyzerDataBtn->setEnabled(false);
    ui->screenshotAA->setEnabled(false);
    ui->spinBoxPoints->setEnabled(true);

    PopUpIndicator::hideIndicator(this);
    m_analyzer->setIsMeasuring(false);
    ui->singleStart->setChecked(false);
    ui->continuousStartBtn->setChecked(false);
    m_measurements->setContinuous(false);
    m_bInterrupted = true;

    if (m_analyzer != nullptr)
        m_analyzer->searchAnalyzer();
}
/*
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

    int limit = g_developerMode ? 1 : SWR_ZOOM_LIMIT;
    if (e->modifiers() == Qt::ControlModifier)
    {
        if(m_measurements)
        {
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
        QCPRange range = m_swrWidget->yAxis->range();
        double lower = range.lower - 0.2;
        m_swrWidget->yAxis->setRangeLower((lower < MIN_SWR) ? MIN_SWR : lower);
        if(e->delta() < 0)
        {
            if(m_swrZoomState <= 9)
            {
                ++m_swrZoomState;                
                m_swrWidget->yAxis->setRangeUpper(m_swrZoomState+0.02);
            }
        }else
        {
            if(m_swrZoomState > limit)
            {
                m_swrZoomState = m_swrZoomState - m_swrZoomState/10;
                if (lower < MIN_SWR) {
                    m_swrWidget->yAxis->setRangeLower(MIN_SWR);
                    m_swrWidget->yAxis->setRangeUpper((MIN_SWR-lower)+m_swrZoomState+0.02);
                } else {
                    m_swrWidget->yAxis->setRangeUpper(m_swrZoomState+0.02);
                }
            }
        }
        QTimer::singleShot(5, m_markers, SLOT(redraw()));
    }

    emit rescale();
}
*/
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

    //int limit = g_developerMode ? 1 : SWR_ZOOM_LIMIT;
    if (e->modifiers() == Qt::ControlModifier)
    {
        if(m_measurements)
        {
            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
        }
        QCPRange range = m_swrWidget->yAxis->range();

        double length = range.size();
        if ((length >= 10 && e->angleDelta().y() > 0) || (length <= 0.1 && e->angleDelta().y() < 0))
            return;

        if (!g_developerMode && (length <= SWR_ZOOM_LIMIT) && (e->angleDelta().y() < 0))
            return;

        double lower = range.lower - 0.02;
        m_swrWidget->yAxis->setRangeLower(qMin(lower, MIN_SWR));
        double upper = range.upper;
        double delta = 0.1;
        if(length >= 5)
        {
            delta = 1;
        } else if (length >= 2.5) {
            delta = 0.5;
        } else if (length >= 1) {
            delta = 0.1;
        }

        if(e->angleDelta().y() < 0)
        {
            delta = -delta;
        }
        upper += delta + 0.02;
        if (upper > 10.02)
            upper = 10.02;
        m_swrWidget->yAxis->setRangeUpper(upper);
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
        if(e->angleDelta().y() < 0)
        {
            if(g_developerMode || state <= 19)
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
        if(e->angleDelta().y() < 0)
        {
            if(g_developerMode || state <= 19)
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
        if(e->angleDelta().y() < 0)
        {
            if(g_developerMode || m_rlZoomState <= 9)
            {
                ++m_rlZoomState;
                m_rlWidget->yAxis->setRangeUpper(m_rlZoomState*5);
                m_rlWidget->yAxis->setRangeLower(0);
                m_rlWidget->replot();
            }
        }else
        {
            int limit = g_developerMode ? 1 : SWR_ZOOM_LIMIT;
            if(m_rlZoomState > limit)
            {
                --m_rlZoomState;
                m_rlWidget->yAxis->setRangeUpper(m_rlZoomState*5);
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
        double lo = m_rlWidget->yAxis->range().lower;
        double up = m_rlWidget->yAxis->range().upper;
        if(y >= lo && y <= up)
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
    static int state = 0;
    //qDebug() << "MainWindow::on_mouseWheel_tdr: state" << state << ", lo " << m_tdrWidget->yAxis->getRangeLower() << ", up " << m_tdrWidget->yAxis->getRangeUpper();
    if (e->modifiers() == Qt::ControlModifier)
    {
//        if(m_measurements)
//        {
//            QTimer::singleShot(1, m_measurements, SLOT(on_redrawGraphs()));
//        }
//        double up = m_tdrWidget->yAxis->getRangeUpper();
//        double lo = m_tdrWidget->yAxis->getRangeLower();

        if(e->angleDelta().y() < 0)
        {
            if(state <= 30)
            {
                ++state;
//                m_tdrWidget->yAxis->setRangeUpper(up+0.1);//(up + up*0.1 );
//                m_tdrWidget->yAxis->setRangeLower(lo-0.1);//(lo - lo*0.1);
                m_tdrWidget->yAxis->scaleRange(1.1, 0);
                m_tdrWidget->replot();
            }
        } else {
            if(state > -30)
            {
                --state;
//                m_tdrWidget->yAxis->setRangeUpper(up-0.1);//(up - up*0.1 );
//                m_tdrWidget->yAxis->setRangeLower(lo+0.1);//(lo + lo*0.1);
                m_tdrWidget->yAxis->scaleRange(0.9, 0);
                m_tdrWidget->replot();
            }
        }
        QTimer::singleShot(5, m_markers, SLOT(redraw()));
        //emit rescale();
    }
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

void MainWindow::on_mouseMove_s21(QMouseEvent * e)
{
    QPointF pos = e->pos();
    double x = m_s21Widget->xAxis->pixelToCoord(e->pos().x());
    if( (x >= m_s21Widget->xAxis->range().lower) && (x <= m_s21Widget->xAxis->range().upper))
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

void MainWindow::on_mouseWheel_user(QWheelEvent * e)
{
    static int state = 1;
    double from  = m_userWidget->xAxis->getRangeLower();
    double to = m_userWidget->xAxis->getRangeUpper();
    if(!m_isRange)
    {
        setFqFrom(from);
        setFqTo(to);
    }else
    {
        setFqFrom((from+to)/2);
        setFqTo((to-from)/2);
    }

    m_swrWidget->xAxis->setRange(m_userWidget->xAxis->range());
    m_phaseWidget->xAxis->setRange(m_userWidget->xAxis->range());
    m_rpWidget->xAxis->setRange(m_userWidget->xAxis->range());
    m_rsWidget->xAxis->setRange(m_userWidget->xAxis->range());
    m_rlWidget->xAxis->setRange(m_userWidget->xAxis->range());

    if (e->modifiers() == Qt::ControlModifier)
    {
        int val;
        if(e->angleDelta().y() < 0)
        {
            if(g_developerMode || state <= 19)
            {
                ++state;
                val = state*80;
                m_userWidget->yAxis->setRangeLower(-val);
                m_userWidget->yAxis->setRangeUpper(val);
                m_userWidget->replot();
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
                m_userWidget->yAxis->setRangeLower(-val);
                m_userWidget->yAxis->setRangeUpper(val);
                m_userWidget->replot();
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

void MainWindow::on_mouseMove_user(QMouseEvent *e)
{
    m_isMouseClick = false;
    double x = m_userWidget->xAxis->pixelToCoord(e->pos().x());
    double y = m_userWidget->yAxis->pixelToCoord(e->pos().y());
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
        if(y >= m_userWidget->yAxis->range().lower && y <= m_userWidget->yAxis->range().upper)
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
        m_swrWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_phaseWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rpWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rsWidget->xAxis->setRange(m_userWidget->xAxis->range());
        m_rlWidget->xAxis->setRange(m_userWidget->xAxis->range());
    }
}

void MainWindow::createTabs (QString sequence)
{
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QStringList tabs = sequence.split(',', Qt::SkipEmptyParts);
    foreach (const QString tab, tabs)
    {
        if (tab == "tab_swr") {
            m_tab_swr = new GLWidget();
            m_tab_swr->setObjectName(QStringLiteral("tab_swr"));
            QHBoxLayout* layout = new QHBoxLayout(m_tab_swr);
            layout->setSpacing(6);
            layout->setContentsMargins(11, 11, 11, 11);
            layout->setObjectName(QStringLiteral("horizontalLayout_1"));
            m_swrWidget = new CustomPlot(1, m_tab_swr);
            qobject_cast<GLWidget*>(m_tab_swr)->setPlotter(m_swrWidget);
            m_swrWidget->setObjectName(QStringLiteral("swr_widget"));
            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_swrWidget->sizePolicy().hasHeightForWidth());
            m_swrWidget->setSizePolicy(sizePolicy);
            layout->addWidget(m_swrWidget);
            ui->tabWidget->addTab(m_tab_swr, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_swr), QApplication::translate("MainWindow", "SWR", 0));
            m_mapWidgets.insert(QStringLiteral("swr_widget"), m_swrWidget);
        }
        if (tab == "tab_phase") {
            m_tab_phase = new GLWidget();
            m_tab_phase->setObjectName(QStringLiteral("tab_phase"));

            QHBoxLayout* layout = new QHBoxLayout(m_tab_phase);
            layout->setSpacing(6);
            layout->setContentsMargins(11, 11, 11, 11);
            layout->setObjectName(QStringLiteral("horizontalLayout_2"));
            m_phaseWidget = new CustomPlot(1, m_tab_phase);
            qobject_cast<GLWidget*>(m_tab_phase)->setPlotter(m_phaseWidget);
            m_phaseWidget->setObjectName(QStringLiteral("phase_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_phaseWidget->sizePolicy().hasHeightForWidth());
            m_phaseWidget->setSizePolicy(sizePolicy);
            layout->addWidget(m_phaseWidget);

            ui->tabWidget->addTab(m_tab_phase, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_phase), QApplication::translate("MainWindow", "Phase", 0));
            m_mapWidgets.insert(QStringLiteral("phase_widget"), m_phaseWidget);
        }
        if (tab == "tab_rs") {
            m_tab_rs = new GLWidget();
            m_tab_rs->setObjectName(QStringLiteral("tab_rs"));

            QHBoxLayout* layout = new QHBoxLayout(m_tab_rs);
            layout->setSpacing(6);
            layout->setContentsMargins(11, 11, 11, 11);
            layout->setObjectName(QStringLiteral("horizontalLayout_3"));
            m_rsWidget = new CustomPlot(3, m_tab_rs);

            qobject_cast<GLWidget*>(m_tab_rs)->setPlotter(m_rsWidget);
            m_rsWidget->setObjectName(QStringLiteral("rs_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_rsWidget->sizePolicy().hasHeightForWidth());
            m_rsWidget->setSizePolicy(sizePolicy);
            layout->addWidget(m_rsWidget);

            ui->tabWidget->addTab(m_tab_rs, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_rs), QApplication::translate("MainWindow", "Z=R+jX", 0));
            m_mapWidgets.insert(QStringLiteral("rs_widget"), m_rsWidget);
        }
        if (tab == "tab_rp") {
            m_tab_rp = new GLWidget();
            m_tab_rp->setObjectName(QStringLiteral("tab_rp"));

            QHBoxLayout* layout = new QHBoxLayout(m_tab_rp);
            layout->setSpacing(6);
            layout->setContentsMargins(11, 11, 11, 11);
            layout->setObjectName(QStringLiteral("horizontalLayout_4"));
            m_rpWidget = new CustomPlot(3, m_tab_rp);
            qobject_cast<GLWidget*>(m_tab_rp)->setPlotter(m_rpWidget);
            m_rpWidget->setObjectName(QStringLiteral("rp_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_rpWidget->sizePolicy().hasHeightForWidth());
            m_rpWidget->setSizePolicy(sizePolicy);
            layout->addWidget(m_rpWidget);

            ui->tabWidget->addTab(m_tab_rp, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_rp), QApplication::translate("MainWindow", "Z=R||+jX", 0));
            m_mapWidgets.insert(QStringLiteral("rp_widget"), m_rpWidget);
        }
        if (tab == "tab_rl") {
            m_tab_rl = new GLWidget();
            m_tab_rl->setObjectName(QStringLiteral("tab_rl"));
            QHBoxLayout* layout = new QHBoxLayout(m_tab_rl);
            layout->setSpacing(6);
            layout->setContentsMargins(11, 11, 11, 11);
            layout->setObjectName(QStringLiteral("horizontalLayout_2"));
            m_rlWidget = new CustomPlot(1, m_tab_rl);
            qobject_cast<GLWidget*>(m_tab_rl)->setPlotter(m_rlWidget);
            m_rlWidget->setObjectName(QStringLiteral("rl_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_rlWidget->sizePolicy().hasHeightForWidth());
            m_rlWidget->setSizePolicy(sizePolicy);
            layout->addWidget(m_rlWidget);
            ui->tabWidget->addTab(m_tab_rl, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_rl), QApplication::translate("MainWindow", "RL", 0));
            m_mapWidgets.insert(QStringLiteral("rl_widget"), m_rlWidget);
        }
        if (tab == "tab_s21") {
            m_tab_s21 = new GLWidget();
            m_tab_s21->setObjectName(QStringLiteral("tab_s21"));

            QHBoxLayout* layout = new QHBoxLayout(m_tab_s21);
            layout->setSpacing(6);
            layout->setContentsMargins(11, 11, 11, 11);
            layout->setObjectName(QStringLiteral("horizontalLayout_s21"));
            m_s21Widget = new CustomPlot(2, m_tab_s21);
            qobject_cast<GLWidget*>(m_tab_s21)->setPlotter(m_s21Widget);
            m_s21Widget->setObjectName(QStringLiteral("s21_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_s21Widget->sizePolicy().hasHeightForWidth());
            m_s21Widget->setSizePolicy(sizePolicy);
            layout->addWidget(m_s21Widget);

            ui->tabWidget->addTab(m_tab_s21, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_s21), QApplication::translate("MainWindow", "S21", 0));
            m_mapWidgets.insert(QStringLiteral("s21_widget"), m_s21Widget);
        }
        if (tab == "tab_tdr") {
            m_tab_tdr = new GLWidget();
            m_tab_tdr->setObjectName(QStringLiteral("tab_tdr"));

            QHBoxLayout* layout = new QHBoxLayout(m_tab_tdr);
            layout->setSpacing(6);
            layout->setContentsMargins(11, 11, 11, 11);
            layout->setObjectName(QStringLiteral("horizontalLayout_6"));
            m_tdrWidget = new CustomPlot(2, m_tab_tdr);
            qobject_cast<GLWidget*>(m_tab_tdr)->setPlotter(m_tdrWidget);
            m_tdrWidget->setObjectName(QStringLiteral("tdr_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_tdrWidget->sizePolicy().hasHeightForWidth());
            m_tdrWidget->setSizePolicy(sizePolicy);
            layout->addWidget(m_tdrWidget);

            ui->tabWidget->addTab(m_tab_tdr, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_tdr), QApplication::translate("MainWindow", "TDR", 0));
            m_mapWidgets.insert(QStringLiteral("tdr_widget"), m_tdrWidget);
        }
        if (tab == "tab_smith") {
            m_tab_smith = new GLWidget();
            m_tab_smith->setObjectName(QStringLiteral("tab_smith"));

            QHBoxLayout* layout = new QHBoxLayout(m_tab_smith);
            layout->setSpacing(6);
            layout->setContentsMargins(11, 11, 11, 11);
            layout->setObjectName(QStringLiteral("horizontalLayout_7"));
            m_smithWidget = new CustomPlot(1, m_tab_smith);
            qobject_cast<GLWidget*>(m_tab_smith)->setPlotter(m_smithWidget);
            m_smithWidget->setObjectName(QStringLiteral("smith_widget"));
            m_smithWidget->mShowHint = false;

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_smithWidget->sizePolicy().hasHeightForWidth());
            m_smithWidget->setSizePolicy(sizePolicy);
            layout->addWidget(m_smithWidget);

            ui->tabWidget->addTab(m_tab_smith, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_smith), QApplication::translate("MainWindow", "Smith", 0));

            m_smithWidget->xAxis->setTicks(false);
            m_smithWidget->yAxis->setTicks(false);
            m_smithWidget->xAxis->setVisible(false);
            m_smithWidget->yAxis->setVisible(false);
            m_mapWidgets.insert(QStringLiteral("smith_widget"), m_smithWidget);
        }
        if (tab == "tab_user") {
            //createUserTab();
            m_tab_user = new GLWidget();
            m_tab_user->setObjectName(QStringLiteral("tab_user"));

            QHBoxLayout* layout = new QHBoxLayout(m_tab_user);
            layout->setSpacing(6);
            layout->setContentsMargins(11, 11, 11, 11);
            layout->setObjectName(QStringLiteral("horizontalLayout_8"));
            m_userWidget = new CustomPlot(1, m_tab_user);
            qobject_cast<GLWidget*>(m_tab_user)->setPlotter(m_userWidget);
            m_userWidget->setObjectName(QStringLiteral("user_widget"));

            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(2);
            sizePolicy.setHeightForWidth(m_userWidget->sizePolicy().hasHeightForWidth());
            m_userWidget->setSizePolicy(sizePolicy);
            layout->addWidget(m_userWidget);

            ui->tabWidget->addTab(m_tab_user, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_user), QApplication::translate("MainWindow", "User defined", 0));
            m_mapWidgets.insert(QStringLiteral("user_widget"), m_userWidget);
        }
#ifndef NO_MULTITAB
        if (tab == "tab_multi") {
            m_tab_multi = new GLWidget();
            m_tab_multi->setObjectName(QStringLiteral("tab_multi"));
            QVBoxLayout* layout = new QVBoxLayout(m_tab_multi);
            layout->setSpacing(6);
            layout->setContentsMargins(11, 11, 11, 11);
            layout->setObjectName(QStringLiteral("layout_multi"));

            ui->tabWidget->addTab(m_tab_multi, QString());
            ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_multi), QApplication::translate("MainWindow", "Multi", 0));
            ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(m_tab_multi), false);
            ui->printBtn->setEnabled(true);
            //ui->tabWidget->widget(ui->tabWidget->indexOf(m_tab_multi))->setVisible(false);
        }
#endif
    }

#ifndef NO_MULTITAB
    if (!g_developerMode) {
        ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(m_tab_user), false);
//        ui->tabWidget->widget(ui->tabWidget->indexOf(m_tab_user))->setVisible(false);
    }
#endif

    m_settings->beginGroup("Settings");
    QString strColor = m_settings->value("chart-background", "#ffffff").toString();
    QColor color;
    color.setNamedColor(strColor);
    setChartBackground(color);
    m_settings->endGroup();

    ui->tabWidget->setCurrentIndex(0);

#ifndef NO_MULTITAB
    ui->tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tabWidget->tabBar(), &QTabBar::customContextMenuRequested, [=](const QPoint& point) {
        if (point.isNull())
            return;
        QMenu menu(this);

        QTabBar* tabBar = ui->tabWidget->tabBar()         ;
        int tabIndex = tabBar->tabAt(point);
        QWidget* tab = ui->tabWidget->widget(tabBar->tabAt(point));
        QString tabName = tab->objectName();
        if (tabName != "tab_multi") {
            if (m_multiTabData.isFull())
                return;
            menu.addAction(tr("Move chart to the tab Multi"), this, [=]() {
                toMultiTab(tabIndex);
            });
        } else {
            menuMultiTab(menu);
        }
        menu.exec(tabBar->mapToGlobal(point));
    });

    QToolButton *btn = new QToolButton();
    btn->setText("+");
    btn->setToolTip("Add multi-charts");
    connect(btn, &QAbstractButton::clicked, this, [=]() {
        showMultiTab();
    });
    btn->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(btn, &QToolButton::customContextMenuRequested, [=](const QPoint& point) {
        if (point.isNull())
            return;
        QMenu menu(this);
        menuMultiTab(menu);
        if (menu.exec(btn->mapToGlobal(point)) != nullptr) {
            if (!m_multiTabData.tabs.isEmpty()) {
                ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(m_tab_multi), true);
                ui->printBtn->setEnabled(false);
                //ui->tabWidget->widget(ui->tabWidget->indexOf(m_tab_multi))->setVisible(true);
                ui->tabWidget->setCurrentWidget(m_tab_multi);
            } else {
                ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(m_tab_multi), false);
                ui->printBtn->setEnabled(true);
                //ui->tabWidget->widget(ui->tabWidget->indexOf(m_tab_multi))->setVisible(false);
                ui->tabWidget->setCurrentWidget(m_tab_swr);
            }
        }
    });
    ui->tabWidget->setCornerWidget(btn, Qt::TopRightCorner);
#endif

//    qDebug() << "createTabs:";
//    for (int i=0; i<ui->tabWidget->count(); i++) {
//        QWidget* plot = ui->tabWidget->widget(i);
//        QString name = plot->objectName();
//        qDebug() << name;
//    }
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
    if (g_developerMode)
        m_userWidget->xAxis->setRange(range);
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
    connect(m_analyzerData,&AnalyzerData::signalSaveFile,this,&MainWindow::on_SaveFile);
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
#ifdef NEW_ANALYZER
    AnalyzerParameters* param = AnalyzerParameters::current();
    if (param == nullptr)
        return;
    int wd = param->getWidth();
    int ht = param->getHeight();
#else
    int wd = lcdWidth[m_analyzer->getModel()];
    int ht = lcdHeight[m_analyzer->getModel()];
#endif
    if (CustomAnalyzer::customized()) {
        CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
        if (ca != nullptr) {
            wd = ca->width();
            ht = ca->height();
        }
    }

    if (analyzer()->connectionType() == ReDeviceInfo::Serial && param->name() != "AA-230 ZOOM") {
        QMessageBox::warning(nullptr, tr("Screen shot"), tr("To get screenshots on this analyzer, you need to use the LCD2Clip utility from the https://rigexpert.com"));
        return;
    }
    m_screenshot = new Screenshot(this, m_analyzer->getModel(), ht, wd);
    m_screenshot->setAttribute(Qt::WA_DeleteOnClose);
    m_screenshot->setWindowTitle("Screenshot");

    connect(m_analyzer,SIGNAL(analyzerScreenshotDataArrived(QByteArray)),m_screenshot,SLOT(on_newData(QByteArray)));
    connect(m_screenshot,SIGNAL(screenshotComplete()),m_analyzer,SLOT(on_screenshotComplete()));
    connect(m_screenshot,SIGNAL(newScreenshot()),m_analyzer,SLOT(makeScreenshot()));

    m_analyzer->makeScreenshot();

    m_screenshot->exec();
    m_screenshot = nullptr;
}

void MainWindow::on_singleStart_clicked()
{
    m_measurements->setContinuous(false);

    if (isMeasuring())
    {
        m_bInterrupted = true;
        emit stopMeasure();
        ui->singleStart->setChecked(true);
        ui->continuousStartBtn->setChecked(false);
        if (g_developerMode) {
            m_measurements->hideOneFqWidget();
        }
        return;
    }

    ui->singleStart->setChecked(true);
    ui->continuousStartBtn->setChecked(false);

    //if (g_developerMode)
    {
        ui->singleStart->setChecked(true);
        quint64 fqFrom = ui->lineEdit_fqFrom->text().remove(' ').toLongLong();
        quint64 fqTo = ui->lineEdit_fqTo->text().remove(' ').toLongLong();
        bool oneFq = m_isRange ? (fqTo==0) : (fqTo==fqFrom);
        if (oneFq) {
            on_startOneFq(fqFrom, m_dotsNumber);
            return;
        }
    }


    double start;
    double stop;

#ifdef NEW_ANALYZER
    AnalyzerParameters* param = AnalyzerParameters::current();
    qint64 minFreq = param == nullptr ? 100 : param->minFq().toULongLong();
    qint64 maxFreq = param == nullptr ? ABSOLUTE_MAX_FQ : param->maxFq().toULongLong();
#else
    qint64 minFreq = minFq[m_analyzer->getAnalyzerModel()].toULongLong();
    qint64 maxFreq = maxFq[m_analyzer->getAnalyzerModel()].toULongLong();
#endif

    if (CustomAnalyzer::customized()) {
        CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
        if (ca != nullptr) {
            QString strMin = ca->minFq();
            minFreq = strMin.toULongLong();
            QString strMax = ca->maxFq();
            maxFreq = strMax.toULongLong();
        }
        getEnteredFq(start, stop);
    } else {
        getEnteredFq(start, stop);
        if (m_fqRestrict)
        {
            AnalyzerParameters::normalizeFq(start, stop);
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
    }

    if(m_fqRestrict && (stop > static_cast<double>(maxFreq)))
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
    if (m_fqRestrict) {
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
    }
    QCPRange range(start, stop);
    m_swrWidget->xAxis->setRange(range);
    m_phaseWidget->xAxis->setRange(range);
    m_rsWidget->xAxis->setRange(range);
    m_rpWidget->xAxis->setRange(range);
    m_rlWidget->xAxis->setRange(range);
    m_s21Widget->xAxis->setRange(range);
    if (g_developerMode) {
        m_userWidget->xAxis->setRange(range);
    }
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

    if(ui->tabWidget->currentWidget()->objectName() == "tab_tdr")
    {
#ifdef OLD_TDR

#ifdef NEW_ANALYZER
    AnalyzerParameters* param = AnalyzerParameters::current();
    qint64 minFq_ = param == nullptr ? 100 : param->minFq().toULongLong()*1000;
    qint64 maxFq_ = param == nullptr ? ABSOLUTE_MAX_FQ : param->maxFq().toULongLong()*1000;
#else
    qint64 minFq_ = minFq[m_analyzer->getAnalyzerModel()].toULongLong()*1000;
    qint64 maxFq_ = maxFq[m_analyzer->getAnalyzerModel()].toULongLong()*1000;
#endif
        if (CustomAnalyzer::customized()) {
            CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
            if (ca != nullptr) {
                minFq_ = ca->minFq().toULongLong();
                maxFq_ = ca->maxFq().toULongLong();
            }
        }
        int dots = m_dotsNumber;
        if(dots < TDR_MINPOINTS)
            dots = TDR_MINPOINTS;
        if(dots > TDR_MAXPOINTS)
            dots = TDR_MAXPOINTS;

        emit measure(minFq_, maxFq_, dots);
        m_measurements->startTDRProgress(this, dots);
#else
        m_measurements->startTDRProgress(analyzer(), this);
#endif
    }
    else if(ui->tabWidget->currentWidget()->objectName() == "tab_user")
    {
        emit measureUser(start*1000, stop*1000, m_dotsNumber);
    }
    else if(ui->tabWidget->currentWidget()->objectName() == "tab_s21")
    {
        emit measureS21(start*1000, stop*1000, m_dotsNumber);
    }
    else
    {
        emit measure(start*1000, stop*1000, m_dotsNumber);
    }
    ui->measurmentsSaveBtn->setEnabled(true);
    ui->exportBtn->setEnabled(true);
    ui->measurmentsDeleteBtn->setEnabled(!m_analyzer->isMeasuring());
    ui->measurmentsClearBtn->setEnabled(!m_analyzer->isMeasuring());

    dtStartMeasurement = QDateTime::currentDateTime();
}

void MainWindow::on_continuousStartBtn_clicked(bool checked)
{
    if (isMeasuring())
    {
        m_bInterrupted = true;
        emit stopMeasure();
        ui->singleStart->setChecked(false);
        ui->continuousStartBtn->setChecked(false);
        m_isContinuos = false;
        m_measurements->setContinuous(false);

        if (g_developerMode) {
            m_measurements->hideOneFqWidget();
        }
        return;
    }
    if(ui->tabWidget->currentWidget()->objectName() == "tab_tdr") {
        ui->continuousStartBtn->setChecked(false);
        return;
    }

    if (g_developerMode) {
        quint64 fqFrom = ui->lineEdit_fqFrom->text().remove(' ').toLongLong();
        quint64 fqTo = ui->lineEdit_fqTo->text().remove(' ').toLongLong();
        bool oneFq = m_isRange ? (fqTo==0) : (fqTo==fqFrom);
        if (oneFq) {
            ui->continuousStartBtn->setChecked(true);
            on_startOneFq(fqFrom, m_dotsNumber);
            return;
        }
    }

    ui->singleStart->setChecked(false);
    m_isContinuos = checked;
    m_analyzer->setContinuos(m_isContinuos);
    if(m_isContinuos)
    {
        m_bInterrupted = false;
        double start;
        double stop;

        // 20210423
        //start = getFqFrom();
        //stop = getFqTo();
        getEnteredFq(start, stop);

#ifdef NEW_ANALYZER
    AnalyzerParameters* param = AnalyzerParameters::current();
    qint64 minFreq = param == nullptr ? 100 : param->minFq().toULongLong();
    qint64 maxFreq = param == nullptr ? ABSOLUTE_MAX_FQ : param->maxFq().toULongLong();
#else
    qint64 minFreq = minFq[m_analyzer->getAnalyzerModel()].toULongLong();
    qint64 maxFreq = maxFq[m_analyzer->getAnalyzerModel()].toULongLong();
#endif
        if (CustomAnalyzer::customized()) {
            CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
            if (ca != nullptr) {
                minFreq = ca->minFq().toULongLong();
                maxFreq = ca->maxFq().toULongLong();
            }
        } else {
            AnalyzerParameters::normalizeFq(start, stop);
        }

        if(m_fqRestrict && (stop > static_cast<double>(maxFreq)))
        {
            stop = maxFreq;
            if(!m_isRange)
            {
                setFqTo(stop);
            }else
            {
                setFqTo((stop-start)/2);
            }
        }
        if (m_fqRestrict) {
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
        }
        QCPRange range(start, stop);
        m_swrWidget->xAxis->setRange(range);
        m_phaseWidget->xAxis->setRange(range);
        m_rsWidget->xAxis->setRange(range);
        m_rpWidget->xAxis->setRange(range);
        m_rlWidget->xAxis->setRange(range);
        m_s21Widget->xAxis->setRange(range);
        if (g_developerMode)
            m_userWidget->xAxis->setRange(range);

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

        if(ui->tabWidget->currentWidget()->objectName() == "tab_user")
        {
            emit measureUser(start*1000, stop*1000, m_dotsNumber);
        } else {
            emit measure(start*1000, stop*1000, m_dotsNumber);
        }
        ui->measurmentsSaveBtn->setEnabled(true);
        ui->exportBtn->setEnabled(true);
        ui->measurmentsDeleteBtn->setEnabled(false);
        ui->measurmentsClearBtn->setEnabled(false);
    }else
    {
        m_bInterrupted = true;
        m_analyzer->setContinuos(false);
    }
    m_measurements->setContinuous(m_isContinuos);
}

void MainWindow::on_startOneFq(quint64 _fq, int _dots)
{
    m_isContinuos = true;
    m_analyzer->setContinuos(m_isContinuos);
    m_measurements->setContinuous(m_isContinuos);

    emit measureOneFq(this, _fq*1000, _dots);

    ui->measurmentsSaveBtn->setEnabled(false);
    ui->exportBtn->setEnabled(false);
    ui->measurmentsDeleteBtn->setEnabled(false);
    ui->measurmentsClearBtn->setEnabled(false);
}

void MainWindow::on_measurementComplete()
{
    if (m_analyzer->connectionType() == ReDeviceInfo::NANO)
        return;
    if (g_developerMode) {
        if (m_measurements->isOneFqMode()) {
            on_continuousStartBtn_clicked(false);
            return;
        }
    }

//{ TODO should be checked for autoclibration
#if 0
    int autoCalibration = m_measurements->getAutoCalibration();
    if (autoCalibration != 0) {
        m_measurements->stopTDRProgress();
        autoCalibrate();
    } else {
        m_measurements->stopTDRProgress();
    }
#endif
    int autoCalibration = m_measurements->getAutoCalibration();
    if (autoCalibration != 0) {
        m_measurements->stopAutocalibrateProgress();
        autoCalibrate();
    } else {
        m_measurements->stopTDRProgress();
    }
//}

    m_tdrWidget->xAxis->setRangeLower(0);

    QTimer::singleShot(5, m_markers, SLOT(redraw()));
    if(m_isContinuos)
    {
        ui->singleStart->setChecked(false);

        double start;
        double stop;

        // 20210423
        //start = getFqFrom();
        //stop = getFqTo();
        getEnteredFq(start, stop);

#ifdef NEW_ANALYZER
    AnalyzerParameters* param = AnalyzerParameters::current();
    qint64 minFreq = param == nullptr ? 100 : param->minFq().toULongLong();
    qint64 maxFreq = param == nullptr ? ABSOLUTE_MAX_FQ : param->maxFq().toULongLong();
#else
    qint64 minFreq = minFq[m_analyzer->getAnalyzerModel()].toULongLong();
    qint64 maxFreq = maxFq[m_analyzer->getAnalyzerModel()].toULongLong();
#endif
        if (CustomAnalyzer::customized()) {
            CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
            if (ca != nullptr) {
                minFreq = ca->minFq().toULongLong();
                maxFreq = ca->maxFq().toULongLong();
            }
        } else {
            AnalyzerParameters::normalizeFq(start, stop);
        }
        if(m_fqRestrict && (stop > static_cast<double>(maxFreq)))
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
        if(m_fqRestrict) {
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
        }
        QCPRange range(start, stop);
        m_swrWidget->xAxis->setRange(range);
        m_phaseWidget->xAxis->setRange(range);
        m_rsWidget->xAxis->setRange(range);
        m_rpWidget->xAxis->setRange(range);
        m_rlWidget->xAxis->setRange(range);
        m_s21Widget->xAxis->setRange(range);
        if (g_developerMode)
            m_userWidget->xAxis->setRange(range);
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
            ui->continuousStartBtn->setChecked(false);
        }
        m_measurements->setContinuous(m_isContinuos);
    } else {
        ui->singleStart->setChecked(false);
        ui->continuousStartBtn->setChecked(false);
        m_measurements->on_measurementComplete();
        m_bInterrupted = true;
        ui->measurmentsDeleteBtn->setEnabled(true);
        ui->measurmentsClearBtn->setEnabled(true);
        ui->exportBtn->setEnabled(true);
        ui->measurmentsSaveBtn->setEnabled(true);
        m_analyzer->setContinuos(false);
        m_analyzer->setIsMeasuring(false);
        PopUpIndicator::setIndicatorVisible(false);
    }
}

void MainWindow::on_measurementCompleteNano()
{
//{ TODO should be checked for autoclibration
#if 0
    int autoCalibration = m_measurements->getAutoCalibration();
    if (autoCalibration != 0) {
        m_measurements->stopTDRProgress();
        autoCalibrate();
    } else {
        m_measurements->stopTDRProgress();
    }
#endif
    int autoCalibration = m_measurements->getAutoCalibration();
    if (autoCalibration != 0) {
        m_measurements->stopAutocalibrateProgress();
        autoCalibrate();
    } else {
        m_measurements->stopTDRProgress();
    }
//}
    m_tdrWidget->xAxis->setRangeLower(0);
    QTimer::singleShot(5, m_markers, SLOT(redraw()));
    if(m_isContinuos)
    {
        ui->singleStart->setChecked(false);
        if (!m_bInterrupted)
        {
            // frequencies were saved by nanjkAnalyzer
            emit measureContinuous(0, 0, m_dotsNumber);
        } else {
            m_bInterrupted = true;
            ui->measurmentsDeleteBtn->setEnabled(true);
            ui->measurmentsClearBtn->setEnabled(true);
            m_analyzer->setContinuos(false);
            m_analyzer->setIsMeasuring(false);
            PopUpIndicator::setIndicatorVisible(false);
            ui->continuousStartBtn->setChecked(false);
        }
        m_measurements->setContinuous(m_isContinuos);
    } else { // single mode
        ui->singleStart->setChecked(false);
        ui->continuousStartBtn->setChecked(false);
        m_measurements->on_measurementComplete();
        m_bInterrupted = true;
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
        //m_smithWidget->yAxis->setRange(m_smithWidget->xAxis->range());
    }else
    {
        double alfa = (double)height/width;
        double range = 14 * alfa;
        m_smithWidget->yAxis->setRangeLower((-1)*range/2);
        m_smithWidget->yAxis->setRangeUpper(range/2);
        //m_smithWidget->xAxis->setRange(m_smithWidget->yAxis->range());
    }
}

void MainWindow::on_settingsBtn_clicked()
{
    emit stopMeasure();
    m_analyzer->setIsMeasuring(false);
    ui->singleStart->setChecked(false);
    ui->continuousStartBtn->setChecked(false);
    ui->singleStart->setEnabled(false);
    ui->continuousStartBtn->setEnabled(false);
    ui->settingsBtn->setEnabled(false);
    m_measurements->setContinuous(false);
    m_bInterrupted = true;
    if (m_settingsDialog == nullptr) {
        m_settingsDialog = new Settings(this);
        connect(&m_settingsDialog->licenseAgent(), &LicenseAgent::registered, this, [=](){
            ui->singleStart->setEnabled(true);
            ui->singleStart->setChecked(true);
            ui->continuousStartBtn->setEnabled(true);
            ui->settingsBtn->setEnabled(true);
        });
    }
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
    m_settingsDialog->setRestrictFq(m_fqRestrict);

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
        connect(m_settingsDialog, &Settings::exportCableSettings,
                m_measurements, &Measurements::on_exportCableSettings);
    }

    if(m_markers)
    {
        //m_settingsDialog->setMarkersHintChecked(m_markers->getMarkersHintEnabled());
        connect(m_settingsDialog, SIGNAL(markersHintChecked(bool)),
                m_markers,SLOT(setMarkersHintEnabled(bool)));
    }

    // TODO
    // ui->pushButtonConnect, use SelectionParameters::selected
    //connect(m_settingsDialog, &Settings::connectDevice, m_analyzer, &AnalyzerPro::on_connectDevice);
/*
    connect(m_settingsDialog, &Settings::connectNanoVNA,
            m_analyzer, &AnalyzerPro::on_connectNanoNVA);
    connect(m_settingsDialog, &Settings::disconnectNanoVNA,
            m_analyzer, &AnalyzerPro::on_disconnectNanoNVA);
    connect(m_settingsDialog, &Settings::connectSerial,
            m_analyzer, &AnalyzerPro::on_connectSerial);
    connect(m_settingsDialog, &Settings::disconnectSerial,
            m_analyzer, &AnalyzerPro::on_disconnectSerial);
    connect(m_settingsDialog, &Settings::connectBluetooth,
            m_analyzer, &AnalyzerPro::on_connectBluetooth);
    connect(m_settingsDialog, &Settings::disconnectBluetooth,
            m_analyzer, &AnalyzerPro::on_disconnectBluetooth);
*/
    connect(m_settingsDialog, &Settings::disconnectDevice,
            m_analyzer, &AnalyzerPro::on_disconnectDevice);

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

    connect(m_settingsDialog, SIGNAL(languageChanged(int)), this, SLOT(on_translate(int)));

    connect(m_settingsDialog, SIGNAL(bandChanged(QString)), this, SLOT(on_bandChanged(QString)));

    connect(m_settingsDialog, &Settings::fqRestrictChecked, this, [this](bool checked) {
        this->m_fqRestrict=checked;
    });
    connect(m_settingsDialog, &Settings::reloadBands, [=](QString band) {
        loadBands();
        on_bandChanged(band);
    });
    connect(m_settingsDialog, &Settings::chartBackgroundChanged, [=](QColor color) {
        setChartBackground(color);
        if (m_measurements != nullptr)
            m_measurements->setBriefHintColor();
    });

    bool was_customized = CustomAnalyzer::customized();

    //m_settingsDialog->exec();
    if(!m_settingsDialog->isVisible())
        m_settingsDialog->show();

    m_settings->beginGroup("Settings");
    bool dark = m_settings->value("darkColorTheme", m_darkColorTheme).toBool();
    m_settings->endGroup();

    if (m_darkColorTheme != dark)
        changeColorTheme(dark);

    if (CustomAnalyzer::customized()) {
        CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
        if (ca != nullptr) {
            on_analyzerNameFound(ca->alias());
        }
    } else if (was_customized) {
        m_analyzer->searchAnalyzer();
    }

    bool force = true;
    m_calibration->start(force);
    ui->checkBoxCalibration->setEnabled(m_calibration->isCalibrationPerformed());
    ui->checkBoxCalibration->setChecked(m_calibration->getCalibrationEnabled());

    ui->measurmentsDeleteBtn->setEnabled(!m_analyzer->isMeasuring());
    ui->measurmentsClearBtn->setEnabled(!m_analyzer->isMeasuring());
    m_measurements->on_redrawGraphs(false);
    updateGraph();
    if (m_markers->markersHintEnabled()) {
        m_markers->repaint();
    }
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
    int columns = ui->tableWidget_measurments->columnCount();
    QList <QTableWidgetItem *> list = ui->tableWidget_measurments->selectedItems();
    for(int i = 0; i < list.length(); i+=columns)
    {
        QTableWidgetItem * item = list.at(i);
        int rowNumber = item->row();
        m_measurements->deleteRow(rowNumber);
    }

    if(ui->tableWidget_measurments->rowCount() == 0)
    {
        //onFullRange(true);
        //{ Fedoseev's request 2022-11-11
        //qint64 from = m_lastEnteredFqFrom;
        //qint64 to =  m_lastEnteredFqTo;
        //qint64 range = (to - from);
        //on_dataChanged(from + range/2, range, m_dotsNumber);
        //}
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
        m_markers->changeMarkersHint();
        m_markers->redraw();
    }
    m_measurements->replot();
}


void MainWindow::on_measurementsClearBtn_clicked(bool)
{
    if(m_analyzer->isMeasuring())
    {
        return;
    }

    m_measurements->on_measurementComplete();
    while(ui->tableWidget_measurments->rowCount() != 0)
    {
        QTableWidgetItem * item = ui->tableWidget_measurments->item(0, 0);
        int rowNumber = item->row();
        m_measurements->deleteRow(rowNumber);
    }

    //{ Antonov's request: keep user's values
    //onFullRange(true);
    double from = m_lastEnteredFqFrom;
    double to =  m_lastEnteredFqTo;
    AnalyzerParameters::normalizeFq(from, to);
    //{ Fedoseev's request 2022-11-11
    //qint64 range = (to - from);
    //on_dataChanged(from + range/2, range, m_dotsNumber);
    //}
    //}

    if(ui->tableWidget_measurments->rowCount() == 0)
    {
        ui->measurmentsSaveBtn->setEnabled(false);
        ui->measurmentsDeleteBtn->setEnabled(false);
        ui->measurmentsClearBtn->setEnabled(false);
        ui->exportBtn->setEnabled(false);
    }
    if(m_markers)
    {
        m_markers->changeMarkersHint();
        m_markers->redraw();
    }
    m_measurements->replot();
}

void MainWindow::on_tableWidget_measurments_cellClicked(int row, int column)
{
    Q_UNUSED(column)
    int count = m_swrWidget->graphCount();

    if(count > 0)
    {
        for(int i = 1; i < count; ++i)
        {
            int pen_width = ((i-1) == row) ? ACTIVE_GRAPH_PEN_WIDTH : INACTIVE_GRAPH_PEN_WIDTH;
            int j = (i-1)*3 + 1;
            {
                QPen pen = m_swrWidget->graph(i)->pen();
                pen.setWidth(pen_width);
                m_swrWidget->graph(i)->setPen(pen);
                m_phaseWidget->graph(i)->setPen(pen);
                m_rlWidget->graph(i)->setPen(pen);
                m_s21Widget->graph(i)->setPen(pen);
                m_measurements->getMeasurement(count - 2 - (i-1))->smithCurve->setPen(pen);

                for (int ii=0; ii<3; ii++) {
                    pen = m_rpWidget->graph(j+ii)->pen();
                    pen.setWidth(pen_width);
                    m_rpWidget->graph(j+ii)->setPen(pen);

                    pen = m_rsWidget->graph(j+ii)->pen();
                    pen.setWidth(pen_width);
                    m_rsWidget->graph(j+ii)->setPen(pen);

                    pen = m_tdrWidget->graph(j+ii)->pen();
                    pen.setWidth(pen_width);
                    m_tdrWidget->graph(j+ii)->setPen(pen);
                }

                if (g_developerMode) {
                    measurement* mm = m_measurements->getMeasurement(count - i-1);
                    int index = m_measurements->getBaseUserGraphIndex(i-1);
                    int cnt = mm->userGraphs.size();
                    for (int ii=0; ii<cnt; ii++) {
                        pen = m_userWidget->graph(index + ii)->pen();
                        pen.setWidth(pen_width);
                        m_userWidget->graph(index + ii)->setPen(pen);
                    }
                }
            }
        }
        updateGraph();
    }
}

void MainWindow::on_tableWidget_measurments_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);

    if (m_measurements->isEmpty())
        return;
    qint32 count = m_measurements->getMeasurementLength();
    measurement* mm = m_measurements->getMeasurement(count - row - 1);
    if (!mm->visible)
        return;
    qint64 from = mm->qint64From/1000;
    qint64 to = mm->qint64To/1000;
    double sw = (to-from)/2.0;
    double center = from + sw;
    if(!m_isRange)
    {
        setFqFrom(from);
        setFqTo(to);
    }else
    {
        setFqFrom(center);
        setFqTo(sw);
    }
    QCPRange range;
    range.lower = from;
    range.upper = to;

    m_swrWidget->xAxis->setRange(range);
    m_phaseWidget->xAxis->setRange(range);
    m_rsWidget->xAxis->setRange(range);
    m_rpWidget->xAxis->setRange(range);
    m_rlWidget->xAxis->setRange(range);
    m_s21Widget->xAxis->setRange(range);
    if (g_developerMode)
        m_userWidget->xAxis->setRange(range);

    QString str = ui->tabWidget->currentWidget()->objectName();
    if (str == "tab_tdr") {
        int dist = m_measurements->calcTdrDist(&mm->dataRX);
        if (dist != 0) {
            range.lower = 0;
            range.upper = dist;
            QWidget::setCursor(Qt::WaitCursor);
            m_measurements->redrawTDR(row);
            m_tdrZRange = m_measurements[row].tdrZRange();
            QWidget::setCursor(Qt::ArrowCursor);
        }
    }

    updateGraph();
}

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
    QString name = ui->tabWidget->currentWidget()->objectName();
    if (name == "tab_multi") {
        return;
        //m_print = new Printmulti(m_multiTabData.tabs);
    } else {
        m_print = new Print();
    }
    m_print->setAttribute(Qt::WA_DeleteOnClose);
    m_print->updateMarkers(m_markers->getMarkersCount(), m_measurements->getMeasurementLength(),
                           m_markers->updateInfo(m_markers->markersHint()->getColumns()));

    m_settings->beginGroup("Settings");
    QString band = m_settings->value("current_band", "ITU Region 1 - Europe, Africa").toString();
    m_settings->endGroup();
    QStringList* bands = nullptr;
    if (m_BandsMap.contains(band))
    {
        bands = m_BandsMap[band];
    }

#ifdef NEW_ANALYZER
    AnalyzerParameters* param = AnalyzerParameters::current();
    QString model = CustomAnalyzer::customized() ?
                CustomAnalyzer::currentPrototype() : (param == nullptr ? "" : param->name());
#else
    QString model = CustomAnalyzer::customized() ?
                CustomAnalyzer::currentPrototype() : names[m_analyzer->getModel()];
#endif
    QString string;
    if (!model.isEmpty())
        string += model + ", ";
    QDateTime datetime = QDateTime::currentDateTime();
    string += datetime.toString("dd.MM.yyyy-hh:mm, ");

    if(name == "tab_swr")
    {
        m_print->setName("SWR");
        string += "SWR graph";
        m_print->drawBands(bands, MIN_SWR, MAX_SWR);
        //m_print->setRange(m_swrWidget->xAxis->range(),m_swrWidget->yAxis->range());
        m_print->setRange(m_swrWidget);
        m_print->setLabel(m_swrWidget->xAxis->label(), m_swrWidget->yAxis->label());
        int cnt = m_swrWidget->graphCount();
        for(int i = 1; i < cnt; ++i)
        {
            QModelIndex myIndex = ui->tableWidget_measurments->model()->
                    index( i-1, COL_NAME, QModelIndex());
            QPen pen = m_swrWidget->graph(i)->pen();
            pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
            m_print->setData(m_swrWidget->graph(i)->data(), pen, myIndex.data().toString());
        }
    }else if(name == "tab_phase")
    {
        string += "Phase graph";
        m_print->drawBands(bands, m_phaseWidget->yAxis->range().lower, m_phaseWidget->yAxis->range().upper);
        //m_print->setRange(m_phaseWidget->xAxis->range(),m_phaseWidget->yAxis->range());
        m_print->setRange(m_phaseWidget);
        m_print->setLabel(m_phaseWidget->xAxis->label(), m_phaseWidget->yAxis->label());
        for(int i = 1; i < m_phaseWidget->graphCount(); ++i)
        {
            QModelIndex myIndex = ui->tableWidget_measurments->model()->
                    index( i-1, COL_NAME, QModelIndex());
            QPen pen = m_phaseWidget->graph(i)->pen();
            pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
            m_print->setData(m_phaseWidget->graph(i)->data(), pen, myIndex.data().toString());
        }
    }else if(name == "tab_rs")
    {
        string += "RXZ graph";
        m_print->drawBands(bands, m_rsWidget->yAxis->range().lower, m_rsWidget->yAxis->range().upper);
        //m_print->setRange(m_rsWidget->xAxis->range(),m_rsWidget->yAxis->range());
        m_print->setRange(m_rsWidget);
        m_print->setLabel(m_rsWidget->xAxis->label(), m_rsWidget->yAxis->label());
        for(int i = 1; i < m_rsWidget->graphCount(); ++i)
        {
            QPen pen = m_rsWidget->graph(i)->pen();
            pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
            m_print->setData(m_rsWidget->graph(i)->data(), pen, m_rsWidget->graph(i)->name());
        }
    }else if(name == "tab_rp")
    {
        string += "RXZ parallel graph";
        m_print->drawBands(bands, m_rpWidget->yAxis->range().lower, m_rpWidget->yAxis->range().upper);
        //m_print->setRange(m_rpWidget->xAxis->range(),m_rpWidget->yAxis->range());
        m_print->setRange(m_rpWidget);
        m_print->setLabel(m_rpWidget->xAxis->label(), m_rpWidget->yAxis->label());
        for(int i = 1; i < m_rpWidget->graphCount(); ++i)
        {
            QPen pen = m_rpWidget->graph(i)->pen();
            pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
            m_print->setData(m_rpWidget->graph(i)->data(), pen, m_rpWidget->graph(i)->name());
        }
    }else if(name == "tab_rl")
    {
        string += "RL graph";
        m_print->drawBands(bands, m_rlWidget->yAxis->range().lower, m_rlWidget->yAxis->range().upper);
        //m_print->setRange(m_rlWidget->xAxis->range(),m_rlWidget->yAxis->range());
        m_print->setRange(m_rlWidget);
        m_print->setLabel(m_rlWidget->xAxis->label(), m_rlWidget->yAxis->label());
        for(int i = 1; i < m_rlWidget->graphCount(); ++i)
        {
            QModelIndex myIndex = ui->tableWidget_measurments->model()->
                                  index( i-1, COL_NAME, QModelIndex());
            QPen pen = m_rlWidget->graph(i)->pen();
            pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
            m_print->setData(m_rlWidget->graph(i)->data(), pen, myIndex.data().toString());
        }
    }else if(name == "tab_s21")
    {
        string += "S21 graph";
        m_print->drawBands(bands, m_s21Widget->yAxis->range().lower, m_s21Widget->yAxis->range().upper);
        m_print->setRange(m_s21Widget);
        m_print->setLabel(m_s21Widget->xAxis->label(), m_s21Widget->yAxis->label());
        for(int i = 1; i < m_s21Widget->graphCount(); ++i)
        {
            QModelIndex myIndex = ui->tableWidget_measurments->model()->
                                  index( i-1, COL_NAME, QModelIndex());
            QPen pen = m_s21Widget->graph(i)->pen();
            pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
            m_print->setData(m_s21Widget->graph(i)->data(), pen, myIndex.data().toString());
        }
    }else if(name == "tab_tdr")
    {
        m_print->setName("TDR");
        string += "TDR graph";
        m_print->drawBands(bands, m_tdrWidget->yAxis->range().lower, m_tdrWidget->yAxis->range().upper);
        //m_print->setRange(m_tdrWidget->xAxis->range(),m_tdrWidget->yAxis->range());
        m_print->setRange(m_tdrWidget);
        QCPRange rr(0, m_tdrZRange);
        m_print->setRange_yAxis2(rr);
        m_print->setLabel(m_tdrWidget->xAxis->label(), m_tdrWidget->yAxis->label());
        for(int i = 1; i < m_tdrWidget->graphCount(); ++i)
        {
            //int i = 3;
            QPen pen = m_tdrWidget->graph(i)->pen();
            pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
            m_print->setData(m_tdrWidget->graph(i)->data(), pen, m_tdrWidget->graph(i)->name());
        }
    }else if(name == "tab_smith")
    {
        string += "Smith graph";
        m_print->drawSmithImage();
        //m_print->setRange(m_smithWidget->xAxis->range(),m_smithWidget->yAxis->range());
        m_print->setRange(m_smithWidget);
        m_print->setLabel(m_smithWidget->xAxis->label(), m_smithWidget->yAxis->label());

        for(int i = 0; i < m_measurements->getMeasurementLength(); ++i)
        {
            QModelIndex myIndex = ui->tableWidget_measurments->model()->
                                index( m_smithWidget->graphCount()-i-1, COL_NAME, QModelIndex());
            QPen pen = m_measurements->getMeasurement(i)->smithCurve->pen();
            pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
            m_print->setSmithData(&m_measurements->getMeasurement(i)->smithGraph, pen, myIndex.data().toString());
        }
    }else if(name == "tab_user")
    {
        string += "User defined";
        m_print->drawBands(bands, m_userWidget->yAxis->range().lower, m_userWidget->yAxis->range().upper);
        //m_print->setRange(m_userWidget->xAxis->range(),m_userWidget->yAxis->range());
        m_print->setRange(m_userWidget);
        m_print->setLabel(m_userWidget->xAxis->label(), m_userWidget->yAxis->label());
        for(int i = 1; i < m_userWidget->graphCount(); ++i)
        {
            m_print->setData(m_userWidget->graph(i)->data(), m_userWidget->graph(i)->pen(), m_userWidget->graph(i)->name());
        }
    }
#ifndef NO_MULTITAB
    else if(name == "tab_multi") {
        string += "Multi";
        foreach (auto tab, m_multiTabData.tabs) {
            QCustomPlot* plot = plotForTab(tab);
            if (plot != nullptr) {
                m_print->drawBands(bands, plot->yAxis->range().lower, plot->yAxis->range().upper);
                ((Printmulti*)m_print)->setRange(tab, plot);
                m_print->setLabel(plot->xAxis->label(), plot->yAxis->label());
                for(int i = 1; i < plot->graphCount(); ++i)
                {
                    QPen pen = plot->graph(i)->pen();
                    pen.setWidth(INACTIVE_GRAPH_PEN_WIDTH);
                    ((Printmulti*)m_print)->setData(tab, plot->graph(i)->data(), pen, plot->graph(i)->name());
                }
            }
        }
    }
#endif
    if(name != "tab_smith")
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
        if(m_lastSaveOpenPath.indexOf('.') >= 0)
        {
            m_lastSaveOpenPath.remove(m_lastSaveOpenPath.indexOf('.'),4);
            m_lastSaveOpenPath.append(".asd");
        }
        QString path = QFileDialog::getSaveFileName(this, "Save file", m_lastSaveOpenPath, "AntScope2 (*.asd )");
        if(!path.isEmpty())
        {
            m_lastSaveOpenPath = path;
            QTableWidgetItem * item = list.at(0);
            int row = item->row();
            m_measurements->saveData(row, path);
            QFileInfo fi(path);
            QString fname = fi.baseName();
            //item->setText(fname);

            measurement* mm = m_measurements->getMeasurement(m_measurements->getMeasurementLength()-row-1);
            QString mmName = mm->name;
            int pos = mmName.indexOf("> ");
            if (pos != -1)
                mmName = mmName.left(pos+2);
            mm->name = mmName + fname;
            ui->tableWidget_measurments->item(row, COL_NAME)->setText(mm->name);
        }
    }
}

void MainWindow::on_measurementsOpenBtn_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Open file", m_lastSaveOpenPath, "AntScope2 (*.asd )");
    if(!path.isEmpty())
    {
        m_lastSaveOpenPath = path;

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
    if (m_lastExportImportPath.isEmpty()) {
        m_settings->beginGroup("Export");
        m_lastExportImportPath = m_settings->value("lastExportPath", "").toString();
        m_settings->endGroup();
    }
    if (m_lastExportImportPath.isEmpty()) {
        m_lastExportImportPath = m_lastSaveOpenPath;
    }

    QString path = QFileDialog::getOpenFileName(this, "Open file", m_lastExportImportPath,  "S1p (*.s1p);;"
                                                                                    "Csv (*.csv);;"
                                                                                    "Nwl (*.nwl);;"
                                                                                    "AntScope2 (*.asd );;"

                                                                                    "All files (*.*)");
    if (path.isEmpty())
        return;

    m_measurements->loadData(path);
    ui->measurmentsSaveBtn->setEnabled(true);
    ui->exportBtn->setEnabled(true);
    ui->measurmentsDeleteBtn->setEnabled(true);
    ui->measurmentsClearBtn->setEnabled(true);
    m_lastExportImportPath = path;
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

    m_measurements->on_impedanceChanged(m_Z0);
}

void MainWindow::updateGraph ()
{
    QCustomPlot* plot = nullptr;
    try {
        QString str = ui->tabWidget->currentWidget()->objectName();
        if( str == "tab_swr")
        {
            plot = m_swrWidget;
        }else if(str == "tab_phase")
        {
            plot = m_phaseWidget;
        }else if(str == "tab_rs")
        {
            plot = m_rsWidget;
        }else if(str == "tab_rp")
        {
            plot = m_rpWidget;
        }else if(str == "tab_rl")
        {
            plot = m_rlWidget;
        }else if(str == "tab_s21")
        {
            plot = m_s21Widget;
        }else if(str == "tab_tdr")
        {
            plot = m_tdrWidget;
        }else if(str == "tab_smith")
        {
            resizeWnd();
            plot = m_smithWidget;
        }else if(str == "tab_user")
        {
            plot = m_userWidget;
        }
#ifndef NO_MULTITAB
        else if(str == "tab_multi") {
            for (int idx=0; idx<m_multiTabData.tabs.size(); idx++) {
                QString tab_name = m_multiTabData.tabs[idx];
                QString plot_name = g_mapTabPlotNames[tab_name];
                m_mapWidgets[plot_name]->replot();
            }
            return;
        }
#endif
    } catch(...) {
        return;
    }
    if (plot != nullptr)
        plot->replot();
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
        double from;
        double to;
        getEnteredFq(from, to);
        AnalyzerParameters::normalizeFq(from, to);

        m_isRange = false;
        ui->rangeBtn->setChecked(false);
        ui->startLabel->setText(tr("Start"));
        ui->stopLabel->setText(tr("Stop"));
        ui->groupBox_Presets->setTitle(tr("Presets (limits), kHz"));
        ui->tableWidget_presets->horizontalHeaderItem(0)->setText(tr("Start"));
        ui->tableWidget_presets->horizontalHeaderItem(1)->setText(tr("Stop"));

        setFqFrom(from);
        setFqTo(to);
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
        double start;
        double stop;
        getEnteredFq(start, stop);
        double range = (stop - start)/2;
        double center = start + range;
        AnalyzerParameters::normalizeFqRange(center, range);

        m_isRange = true;
        ui->limitsBtn->setChecked(false);
        ui->startLabel->setText(tr("Center"));
        ui->stopLabel->setText(tr("Range (+/-)"));
        ui->groupBox_Presets->setTitle(tr("Presets (center, range), kHz"));
        ui->tableWidget_presets->horizontalHeaderItem(0)->setText(tr("Center"));
        ui->tableWidget_presets->horizontalHeaderItem(1)->setText(tr("Range(+/-)"));
        // 20210423
        //double from = getFqFrom();
        //double to = getFqTo();
        setFqFrom(center);
        setFqTo(range);
        emit isRangeChanged(m_isRange);
    }
}


void MainWindow::setFqFrom(QString from)
{
    from.remove(' ');
    from = appendSpaces(from);
    ui->lineEdit_fqFrom->setText(from);
}

void MainWindow::setFqFrom(double from)
{
    QString sFrom = QString::number(from,'f', 0);
    sFrom = appendSpaces(sFrom);
    ui->lineEdit_fqFrom->setText(sFrom);
}

void MainWindow::setFqTo(QString to)
{
    to.remove(' ');
    to = appendSpaces(to);
    ui->lineEdit_fqTo->setText(to);
}

void MainWindow::setFqTo(double to)
{
    QString sTo = QString::number(to,'f', 0);
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


void MainWindow::changeFqFrom(bool _backupValue)
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
            if (g_developerMode)
                m_userWidget->xAxis->setRangeUpper(upper);
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
        if (g_developerMode)
            m_userWidget->xAxis->setRangeLower(lower);
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
            if (g_developerMode)
                m_userWidget->xAxis->setRangeUpper(upper);
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
        if (g_developerMode) {
            m_userWidget->xAxis->setRangeUpper(lower+ui->lineEdit_fqTo->text().remove(' ').toDouble()*2);
            m_userWidget->xAxis->setRangeLower(lower);
        }
    }
    if (_backupValue) {
        m_lastEnteredFqFrom = lower;
        m_lastEnteredFqTo = upper;
    }
    updateGraph();
}

void MainWindow::changeFqTo(bool _backupValue)
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
            if (g_developerMode)
                m_userWidget->xAxis->setRangeLower(lower);
        }
        m_swrWidget->xAxis->setRangeUpper(upper);
        m_phaseWidget->xAxis->setRangeUpper(upper);
        m_rsWidget->xAxis->setRangeUpper(upper);
        m_rpWidget->xAxis->setRangeUpper(upper);
        m_rlWidget->xAxis->setRangeUpper(upper);
        if (g_developerMode)
            m_userWidget->xAxis->setRangeUpper(upper);
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
            if (g_developerMode)
                m_userWidget->xAxis->setRangeLower(lower);
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
        if (g_developerMode) {
            m_userWidget->xAxis->setRangeUpper(upper);
            m_userWidget->xAxis->setRangeLower(lower);
        }
    }
    if (_backupValue) {
        m_lastEnteredFqFrom = lower;
        m_lastEnteredFqTo = upper;
    }
    updateGraph();
    ui->singleStart->setFocus();
}

void MainWindow::on_lineEdit_fqFrom_editingFinished()
{
    changeFqFrom(true);
}

void MainWindow::on_lineEdit_fqTo_editingFinished()
{
    changeFqTo(true);
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
    if(str == "tab_tdr" || str == "tab_smith")
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

bool MainWindow::loadLanguage(QString locale)
{ //locale: en, ukr, ru, ja, etc.
    QString title = windowTitle();
    bool res = m_qtLanguageTranslator->load("QtLanguage_" + locale, Settings::languageDataFolder());
    qApp->installTranslator(m_qtLanguageTranslator);
    ui->retranslateUi(this);

    m_swrWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_swrWidget->yAxis->setLabel(tr("SWR"));
    m_phaseWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_phaseWidget->yAxis->setLabel(tr("Phase, Angle"));
    m_rsWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_rsWidget->yAxis->setLabel(tr("Rs, Ohm"));
    m_rpWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_rpWidget->yAxis->setLabel(tr("Rp, Ohm"));
    m_rlWidget->xAxis->setLabel(tr("Frequency, kHz"));
    m_rlWidget->yAxis->setLabel(tr("RL, dB"));
    m_tdrWidget->xAxis->setLabel(tr("Length, m"));
    m_s21Widget->xAxis->setLabel(tr("Frequency, kHz"));

    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_swr), QApplication::translate("MainWindow", "SWR", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_phase), QApplication::translate("MainWindow", "Phase", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_rs), QApplication::translate("MainWindow", "Z=R+jX", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_rp), QApplication::translate("MainWindow", "Z=R||+jX", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_rl), QApplication::translate("MainWindow", "RL", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_tdr), QApplication::translate("MainWindow", "TDR", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_s21), QApplication::translate("MainWindow", "S21", 0));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_smith), QApplication::translate("MainWindow", "Smith", 0));
    if (g_developerMode)
        ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_user), QApplication::translate("MainWindow", "User defined", 0));

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

void MainWindow::on_SaveFile(int row, QString path)
{
    //int row = ui->tableWidget_measurments->rowCount() - 1;
    saveFile(row, path);
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
    if (!file.exists()) {
        file.setFileName(Settings::programDataPath("itu-regions-defaults.txt"));
    }

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
        setBands(m_swrWidget, bands, MIN_SWR, MAX_SWR);
        setBands(m_phaseWidget, bands, -180, 180);
        setBands(m_rsWidget, bands, -2000, 2000);
        setBands(m_rpWidget, bands, -2000, 2000);
        setBands(m_rlWidget, bands, 0, 50);
        if (g_developerMode)
            setBands(m_userWidget, bands, MIN_USER_RANGE, MAX_USER_RANGE);
    }
}


void MainWindow::onSpinChanged(int value)
{
    if (!g_developerMode) {
        if (value > MAX_DOTS) {
            value = MAX_DOTS;
            ui->spinBoxPoints->setValue(value);
        }
    }
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
    double center_khz = (double)_center_khz;
    double range_khz = (double)_range_khz;
    AnalyzerParameters::normalizeFqRange(center_khz, range_khz);
    _center_khz = (qint64)center_khz;
    _range_khz = (qint64)range_khz;

    ui->spinBoxPoints->setValue(_dots);
    if (m_isRange) {
        ui->lineEdit_fqFrom->setText(QString::number(_center_khz));
        ui->lineEdit_fqTo->setText(QString::number(_range_khz));
    } else {
        ui->lineEdit_fqFrom->setText(QString::number(_center_khz - _range_khz));
        ui->lineEdit_fqTo->setText(QString::number(_center_khz + _range_khz));
    }
    changeFqTo();
    changeFqFrom();
    //on_lineEdit_fqTo_editingFinished();
    //on_lineEdit_fqFrom_editingFinished();
}


void MainWindow::on_importFinished(double _fqMin_khz, double _fqMax_khz)
{
    double _range = (_fqMax_khz - _fqMin_khz);
    double _center = (_fqMin_khz + _range / 2);

    measurement* mm = m_measurements->getMeasurement(m_measurements->getMeasurementLength() - 1);
    if (mm != nullptr)
        mm->set(_center*1000, _range*1000, mm->dataRX.size()-1);

    on_dataChanged((qint64)_center, (qint64)_range, ui->spinBoxPoints->value());

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
    //int model = m_analyzer->getModel();

#ifdef NEW_ANALYZER
    AnalyzerParameters* param = AnalyzerParameters::current();
    qint64 from = param == nullptr ? 100 : param->minFq().toULongLong();
    qint64 to = param == nullptr ? ABSOLUTE_MAX_FQ : param->maxFq().toULongLong();
#else
    qint64 from = minFq[model].toULongLong();
    qint64 to = maxFq[model].toULongLong();
#endif

    qint64 range = to - from;

    m_lastEnteredFqFrom = from;
    m_lastEnteredFqTo = to;

    if (CustomAnalyzer::customized()) {
        CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
        if (ca != nullptr) {
            from = ca->minFq().replace(" ", "").toULongLong();
            to = ca->maxFq().replace(" ", "").toULongLong();
            range = to - from;
        }
    }
    on_dataChanged(from + range/2, range, m_dotsNumber);
}

void MainWindow::on_presssCtrlAltShiftM()
{
    if (!g_developerMode)
        return;

    if (!ui->singleStart->isEnabled())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    on_measurementsClearBtn_clicked(true);

    m_measurements->setAutoCalibration(1);

    QString cmd = "cals\r";
    if (!m_analyzer->sendCommand(cmd)) {
        return;
    }
    QCoreApplication::processEvents();
    QThread::sleep(2);

    cmd = "calt\r";
    if (!m_analyzer->sendCommand(cmd)) {
        return;
    }
    QCoreApplication::processEvents();
    QThread::sleep(2);

    m_measurements->setFarEndMeasurement(0);
    onFullRange(true);
    m_dotsNumber = 200;
    QString style = "QPushButton:checked{"
            "background-color: rgb(255, 1, 52);}";
    ui->singleStart->setStyleSheet(style);

    on_singleStart_clicked();
    QApplication::processEvents();
}

void MainWindow::autoCalibrate()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QPair<double, double> calibr = m_measurements->autoCalibrate(); // <CableResistance, CableLength>
    QString cmd = QString("calrl%1,%2\r")
            .arg((double)calibr.first, 0, 'f', 8, QLatin1Char(' '))
            .arg((double)calibr.second, 0, 'f', 8, QLatin1Char(' '));
    m_analyzer->sendCommand(cmd);

    QString style = "QPushButton:checked{"
            "background-color: rgb(1, 178, 255);}";
    ui->singleStart->setStyleSheet(style);
    QApplication::restoreOverrideCursor();

    QString notify = QString("Autocalibration: CableResistance=%1, CableLength=%2")
            .arg((double)calibr.first, 0, 'f', 8, QLatin1Char(' '))
            .arg((double)calibr.second, 0, 'f', 8, QLatin1Char(' '));
    QRect rn(0, 0, rect().width(), 40);
    Notification::showMessage(notify, QColor(Qt::white), rn, 5000, ui->tabWidget->currentWidget());
    return;

}

void MainWindow::onMeasurementError()
{
    QApplication::beep();
    //showErrorPopup(tr("Measurement ERROR!"), 2000);
    on_pressEsc();
}

void MainWindow::showErrorPopup(QString text, int msDuration)
{
    //qDebug() << "MainWindow::showErrorPopup";

    QLabel* label = new QLabel();
    label->setText(text);
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(label);

    QFrame* frame = new QFrame();
    frame->setLayout(layout);

    QVBoxLayout* layout1 = new QVBoxLayout();
    layout1->addWidget(frame);

    QWidget* widget = new QWidget(this);
    widget->setWindowFlags(Qt::FramelessWindowHint |
                   Qt::Tool | Qt::WindowStaysOnTopHint);
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->setAttribute(Qt::WA_ShowWithoutActivating);

    widget->setLayout(layout1);
    widget->adjustSize();

    int wd = widget->width();
    int ht = widget->height();
    QRect r = geometry();
    int x = r.center().x()-wd/2;
    int y = r.center().y()-ht/2;
    widget->setGeometry(x, y, wd, ht);

    label->setStyleSheet("QLabel { "
                         "margin-top: 6px;"
                         "margin-bottom: 6px;"
                         "margin-left: 10px;"
                         "margin-right: 10px; "
                         "color: white; "
                         "font-weight: bold; "
                         "font-size: 16px; "
                         "}");
    frame->setStyleSheet("border: 1px; border-radius: 8px;");
    widget->setStyleSheet("background-color:red;");

    widget->show();

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [timer, widget]() {
        widget->hide();
        widget->deleteLater();
        timer->deleteLater();
    });

    timer->start(msDuration);
}

void MainWindow::on_tableWidgetMeasurmentsContextMenu(const QPoint& pos)
{
    QTableWidgetItem *item = ui->tableWidget_measurments->itemAt(pos);
    if (item != nullptr)
    {
        int row = item->row();
        QPen pen = m_swrWidget->graph(row+1)->pen();
        QColor color = QColorDialog::getColor(pen.color(), this );
        if( color.isValid() )
        {
            changeMeasurmentsColor(row, color);
        }
    }
}

void MainWindow::changeMeasurmentsColor(int _row, QColor& _color)
{
    int count = m_swrWidget->graphCount();
    if(count > 0)
    {
        int i=_row+1;
        QPen pen = m_swrWidget->graph(i)->pen();
        pen.setColor(_color);
        m_swrWidget->graph(i)->setPen(pen);
        m_phaseWidget->graph(i)->setPen(pen);
        m_rlWidget->graph(i)->setPen(pen);
        m_measurements->getMeasurement(count - 2 - (i-1))->smithCurve->setPen(pen);
        updateGraph();
    }
}

void MainWindow::on_showNotification(QString msg, QString url)
{
//    QRect rn(0, 0, rect().width(), 40);
//    Notification::showMessage(msg, url, rn, 5000, ui->tabWidget->currentWidget());

    QJsonObject json;
    json["message"] = msg;
    json["linkto"] = url;
    json["textcolor"] = QString("#ff8888");
    json["waittime"] = 5;
    MarqueeString str(json);
    QList<MarqueeString> list;
    list << str;
    ui->labelMarquee->addStrings(list);
    ui->labelMarquee->show();
}

QTabWidget* MainWindow::tabWidget()
{
    return ui->tabWidget;
}

void MainWindow::on_presssCtrlAltShiftN()
{
    if (!g_developerMode)
        return;

    if (!ui->singleStart->isEnabled())
        return;

    connect(m_analyzer, &AnalyzerPro::updateAutocalibrate5, this, [this](int _dots, QString _msg){
        if (_msg.contains("START")) {
            m_measurements->startAutocalibrateProgress(this, _dots);
            m_measurements->progressDlg()->updateActionInfo("Adjustment of signal scaling factor");
            m_measurements->progressDlg()->setCancelable(false);
            m_measurements->progressDlg()->setValue(0);
        } else {
            int _max = m_measurements->progressDlg()->maxValue();
            m_measurements->progressDlg()->setValue(_max - _dots);
            m_measurements->progressDlg()->updateStatusInfo(QString(tr("Remains %1").arg(_dots)));
        }
    });
    QObject::connect(m_analyzer, &AnalyzerPro::stopAutocalibrate5, this, [this]() {
        QObject::disconnect(m_analyzer, &AnalyzerPro::stopAutocalibrate5, this, nullptr);
        QObject::disconnect(m_analyzer, &AnalyzerPro::updateAutocalibrate5, this, nullptr);
        m_measurements->stopAutocalibrateProgress();
    });

    m_analyzer->setParseState(WAIT_CALFIVEKOHM_START);
    m_analyzer->sendCommand("CALFIVEKOHM\r");
}

void MainWindow::changeColorTheme(bool _dark)
{
    m_darkColorTheme = _dark;

    QString style;
    if (m_darkColorTheme) {
        qApp->setStyle(QStyleFactory::create("fusion"));
        QPalette asPalette;
        asPalette.setColor(QPalette::WindowText, QColor(1,178,255));
        asPalette.setColor(QPalette::Button, QColor(89, 89, 89));
        asPalette.setColor(QPalette::ButtonText, QColor(255,255,255));
        asPalette.setColor(QPalette::Highlight, QColor(1,178,255));
        asPalette.setColor(QPalette::Window, QColor(79, 79, 79));
        asPalette.setColor(QPalette::Dark, QColor(1,178,255));
        asPalette.setColor(QPalette::Light, QColor(1,178,255));

        qApp->setPalette(asPalette);

        style = "QPushButton:checked{"
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

        style = "QGroupBox {border: 2px solid rgb(1, 178, 255); margin-top: 1ex;}"
                "QGroupBox::title {"
                "subcontrol-origin: margin;"
                "subcontrol-position: top center;" /* position at the top center */
                "padding: 0 3px;}"
                "QGroupBox::title {color: white;}";

        ui->groupBox_Run->setStyleSheet(style);

        style = "QCheckBox:disabled{"
                "color: rgb(119, 119, 119);}";
        ui->checkBoxCalibration->setStyleSheet(style);
    } else {
        qApp->setPalette(m_lightPalette);

        style = "";
        ui->limitsBtn->setStyleSheet(style);
        ui->rangeBtn->setStyleSheet(style);
        ui->exportBtn->setStyleSheet(style);
        ui->printBtn->setStyleSheet(style);
        ui->screenshotAA->setStyleSheet(style);
        ui->analyzerDataBtn->setStyleSheet(style);
        ui->measurmentsSaveBtn->setStyleSheet(style);
        ui->measurmentsDeleteBtn->setStyleSheet(style);
        ui->presetsDeleteBtn->setStyleSheet(style);
        ui->measurmentsClearBtn->setStyleSheet(style);
        ui->singleStart->setStyleSheet(style);
        ui->continuousStartBtn->setStyleSheet(style);
        ui->groupBox_Run->setStyleSheet(style);
        ui->checkBoxCalibration->setStyleSheet(style);
    }
    if (m_markers != NULL)
        m_markers->changeColorTheme(m_darkColorTheme);

    m_measurements->changeColorTheme(m_darkColorTheme);
    m_measurements->on_redrawGraphs();
}

void MainWindow::getEnteredFq(double& start, double& stop)
{
    if (m_isRange) {
        double center = ui->lineEdit_fqFrom->text().remove(' ').toDouble();
        double range = ui->lineEdit_fqTo->text().remove(' ').toDouble();
        start = center - range;
        stop = center + range;
    } else {
        start = ui->lineEdit_fqFrom->text().remove(' ').toDouble();
        stop = ui->lineEdit_fqTo->text().remove(' ').toDouble();
    }
}

void MainWindow::setChartBackground(QColor _color)
{
    if (!_color.isValid())
        return;
    QBrush brush(_color);
    QColor inverse(255-_color.red(), 255-_color.green(), 255-_color.blue());
    QPen pen(inverse);
    pen.setWidth(1);

    foreach (QCustomPlot *_plot, m_mapWidgets) {
        if (_plot == m_smithWidget) {
            continue;
        }
        _plot->setBackground(brush);
        _plot->xAxis->setTickLabelColor(inverse);
        _plot->xAxis->setLabelColor(inverse);
        _plot->xAxis->setSubTickPen(pen);
        _plot->xAxis->setBasePen(pen);
        _plot->xAxis->setTickPen(pen);

        _plot->yAxis->setTickLabelColor(inverse);
        _plot->yAxis->setLabelColor(inverse);
        _plot->yAxis->setSubTickPen(pen);
        _plot->yAxis->setBasePen(pen);
        _plot->yAxis->setTickPen(pen);
    }
}

void MainWindow::newSoftwareRequest()
{
    if (m_updater == nullptr) {
        m_updater = new Updater();
        connect(m_updater, &Updater::newVersionAvailable, this, &MainWindow::on_showNotification);
        connect(m_updater,SIGNAL(progress(int)), this, SIGNAL(updateProgress(int)));
    }
    m_updater->on_checkUpdates();
}


void MainWindow::createUserTab()
{
    m_tab_user = new GLWidget();
    m_tab_user->setObjectName(QStringLiteral("tab_user"));

    QHBoxLayout* layout = new QHBoxLayout(m_tab_user);
    layout->setSpacing(6);
    layout->setContentsMargins(11, 11, 11, 11);
    layout->setObjectName(QStringLiteral("horizontalLayout_user"));
    m_userWidget = new CustomPlot(1, m_tab_user);
    qobject_cast<GLWidget*>(m_tab_user)->setPlotter(m_userWidget);
    m_userWidget->setObjectName(QStringLiteral("user_widget"));

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(2);
    sizePolicy.setHeightForWidth(m_userWidget->sizePolicy().hasHeightForWidth());
    m_userWidget->setSizePolicy(sizePolicy);
    layout->addWidget(m_userWidget);

    ui->tabWidget->addTab(m_tab_user, QString());
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(m_tab_user), QApplication::translate("MainWindow", "User defined", 0));
    m_mapWidgets.insert(QStringLiteral("user_widget"), m_userWidget);
}


#ifndef NO_MULTITAB
void MainWindow::toMultiTab(int tab_index)
{
    QWidget* tab = ui->tabWidget->widget(tab_index);
    m_multiTabData.tabs << tab->objectName();
    buildMultiTabLayout();
    ui->tabWidget->setTabVisible(tab_index, false);
    ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(m_tab_multi), true);
    ui->printBtn->setEnabled(false);
//    ui->tabWidget->widget(tab_index)->setVisible(false);
//    ui->tabWidget->widget(ui->tabWidget->indexOf(m_tab_multi))->setVisible(true);
}

void MainWindow::fromMultiTab(int tab_index)  // ???? tab_index ????
{
    QString tab_name = ui->tabWidget->widget(tab_index)->objectName();
    m_multiTabData.tabs.removeOne(tab_name);
    buildMultiTabLayout();

    QWidget* tab = ui->tabWidget->widget(tab_index);
    QString plot_name = g_mapTabPlotNames[tab_name];
    tab->layout()->addWidget(m_mapWidgets[plot_name]);
    ui->tabWidget->setTabVisible(tab_index, true);
    ui->printBtn->setEnabled(false);
    //ui->tabWidget->widget(tab_index)->setVisible(true);

    if (m_multiTabData.tabs.isEmpty())
        ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(m_tab_multi), false);
        ui->printBtn->setEnabled(true);
        //ui->tabWidget->widget(ui->tabWidget->indexOf(m_tab_multi))->setVisible(false);
}

QMenu& MainWindow::menuMultiTab(QMenu &menu)
{
    QMap<QString, QPair<int, QString>> tab_title; // <tab_name, <index, title> >
    for (int i=0; i<ui->tabWidget->count(); i++) {
        QString tab_name = ui->tabWidget->widget(i)->objectName();
        if (tab_name != "tab_multi") {
            if (!g_developerMode && tab_name == "tab_user")
                continue;
            tab_title[tab_name] = QPair<int, QString>(i, ui->tabWidget->tabText(i));
        }
    }
    foreach (const QString& tab_name, tab_title.keys()) {
        if (tab_name.isEmpty())
            continue;
        if (!m_multiTabData.tabs.contains(tab_name)) {
            QPair<int, QString> pair = tab_title[tab_name];
            QString plot_name = pair.second;
            menu.addAction(tr("Join ") + plot_name, this, [this, pair]() {
                toMultiTab(pair.first);
            });
        }
    }
    menu.addSeparator();
    foreach (const QString& tab_name, m_multiTabData.tabs) {
        if (tab_name.isEmpty())
            continue;
        QPair<int, QString> pair = tab_title[tab_name];
        QString plot_name = pair.second;
        menu.addAction(tr("Close ") + plot_name, this, [this, pair]() {
            fromMultiTab(pair.first);
        });
    }
    if (!m_multiTabData.tabs.isEmpty()) {
        menu.addSeparator();
        menu.addAction(tr("Close all "), this, [this, tab_title]() {
            foreach(const QString& tab_name, m_multiTabData.tabs) {
                QPair<int, QString> pair = tab_title[tab_name];
                fromMultiTab(pair.first);
            }
            ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(m_tab_multi), false);
            ui->printBtn->setEnabled(true);
            //ui->tabWidget->widget(ui->tabWidget->indexOf(m_tab_multi))->setVisible(false);
        });
    }
    return menu;
}

void MainWindow::buildMultiTabLayout()
{
    int tabs = m_multiTabData.tabs.size();
    if (tabs == 0) {
        return;
    }
//    if (tabs == 4) {
//        return;
//    }

    if (m_tab_multi == nullptr)
        return;

    QLayout* layout = m_tab_multi->layout();
    if (layout != nullptr) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != 0)
            layout->removeItem (item);
    }

    foreach(const QString& tab, m_multiTabData.tabs) {
        QString plot = g_mapTabPlotNames[tab];
        QWidget* widget = m_mapWidgets[plot];
        layout->addWidget(widget);
    }
    if (m_multiTabData.tabs.contains("tab_smith")) {
        QSize sz = size();
        sz += QSize(-2, -2);
        resize(sz);
        resizeWnd();
        m_smithWidget->replot();
        sz += QSize(2, 2);
        QTimer::singleShot(1, this, [this, sz]() {
            resize(sz);
            resizeWnd();
            m_smithWidget->replot();
        });
    }
    m_tab_multi->show();
}

void MainWindow::showMultiTab()
{
    QMenu menu;
    menuMultiTab(menu);
    if (menu.exec(QCursor::pos()) != nullptr) {
        if (!m_multiTabData.tabs.isEmpty()) {
            ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(m_tab_multi), true);
            ui->printBtn->setEnabled(false);
            //ui->tabWidget->widget(ui->tabWidget->indexOf(m_tab_multi))->setVisible(true);
            ui->tabWidget->setCurrentWidget(m_tab_multi);
        } else {
            ui->tabWidget->setTabVisible(ui->tabWidget->indexOf(m_tab_multi), false);
            ui->printBtn->setEnabled(true);
            //ui->tabWidget->widget(ui->tabWidget->indexOf(m_tab_multi))->setVisible(false);
            ui->tabWidget->setCurrentWidget(m_tab_swr);
        }
    }
}

void MainWindow::replot_multiTab()
{
    for (int idx=0; idx<MainWindow::m_mainWindow->m_multiTabData.tabs.size(); idx++) {
        QString tab_name = MainWindow::m_mainWindow->m_multiTabData.tabs[idx];
        QString plot_name = g_mapTabPlotNames[tab_name];
        CustomPlot* plot = qobject_cast<CustomPlot*>(m_mapWidgets[plot_name]);
        plot->drawIncrementally();
    }
}

QCustomPlot* MainWindow::plotForTab(const QString& tab)
{
    QString plot_name = g_mapTabPlotNames[tab];
    return m_mapWidgets[plot_name];
}

void MainWindow::restoreMultitab(const QString& tabs)
{
    if (!tabs.isEmpty()) {
        QStringList list = tabs.split(',', Qt::SkipEmptyParts);
        foreach (auto tab_name, list) {
            if (tab_name == "tab_user" && !g_developerMode)
                continue;
            for (int idx = 0; idx<ui->tabWidget->count(); idx++) {
                if (ui->tabWidget->widget(idx)->objectName() == tab_name) {
                    toMultiTab(idx);
                    break;
                }
            }
        }
    }
}
#endif

void MainWindow::on_selectDeviceDialog()
{
    SelectDeviceDialog dlg(false, this);
    if (dlg.exec() == QDialog::Accepted) {
        AnalyzerParameters* selected = AnalyzerParameters::current();
        if (selected != nullptr) {
            m_analyzer->on_connectDevice(dlg.analyzer());
            emit m_analyzer->analyzerFound(selected->index());
        }
    }
    closeSettingsDialog();
    ui->settingsBtn->setEnabled(true);
}

void MainWindow::on_refreshConnection()
{
    AnalyzerParameters* ap = AnalyzerParameters::byName(SelectionParameters::selected.name);
    if (ap != nullptr) {
        SelectionParameters::selected.modelIndex = ap->index();
         AnalyzerParameters::setCurrent(ap);
         if (m_analyzer->refreshConnection()) {
            return;
         }
    }
    QTimer::singleShot(100, this, [=](){
       on_selectDeviceDialog();
    });

}

void MainWindow::closeSettingsDialog()
{
    if (m_settingsDialog == nullptr)
         return;
    m_settingsDialog->close();
    m_settingsDialog->deleteLater();
    m_settingsDialog=nullptr;
    m_measurements->on_currentTab(m_measurements->currentTab());
    ui->singleStart->setEnabled(true);
    ui->continuousStartBtn->setEnabled(true);
    ui->settingsBtn->setEnabled(true);
}

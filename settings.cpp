#include "settings.h"
#include "ui_settings.h"
#include "popupindicator.h"
#include "analyzer/customanalyzer.h"
#include "fqinputvalidator.h"
#include "licensesdialog.h"
#include "analyzer/nanovna_analyzer.h"

extern bool g_developerMode;
extern int g_maxMeasurements; // see measurements.cpp
extern QString appendSpaces(const QString& number);

QString Settings::iniFilePath;
Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings),
    m_analyzer(NULL),
    m_calibration(NULL),
    m_isComplete(false),
    m_generalTimer(NULL),
    m_markersHintEnabled(true),
    m_graphHintEnabled(true),
    m_graphBriefHintEnabled(true),
    m_onlyOneCalib(false),
    m_metricChecked(false),
    m_farEndMeasurement(0)
{
    ui->setupUi(this);
    PopUpIndicator::setIndicatorVisible(false);

    QString style = "QPushButton:disabled{"
                        "background-color: rgb(59, 59, 59);"
                        "color: rgb(119, 119, 119);}";

    ui->openOpenFileBtn->setStyleSheet(style);
    ui->openCalibBtn->setStyleSheet(style);
    ui->shortOpenFileBtn->setStyleSheet(style);
    ui->shortCalibBtn->setStyleSheet(style);
    ui->loadOpenFileBtn->setStyleSheet(style);
    ui->loadCalibBtn->setStyleSheet(style);
    ui->calibWizard->setStyleSheet(style);
    ui->licencesBtn->setStyleSheet(style);

    // NanoVNA support
    NanovnaAnalyzer::detectPorts();
    ui->connectNanovnaBtn->setStyleSheet(style);
    ui->connectNanovnaBtn->setVisible(NanovnaAnalyzer::portsCount() != 0);
    if (NanovnaAnalyzer::isConnected()) {
        ui->connectNanovnaBtn->setText(tr("Disconnect NanoVNA"));
    } else {
        ui->connectNanovnaBtn->setText(tr("Connect NanoVNA"));
    }
    connect(ui->connectNanovnaBtn, &QPushButton::clicked, this, &Settings::on_connectNanovna);

    style = "QGroupBox {border: 2px solid rgb(100,100,100); margin-top: 1ex;}";
    style += "QGroupBox::title {color: rgb(1, 178, 255);}";
    ui->groupBox_10->setStyleSheet(style);
    ui->groupBox_11->setStyleSheet(style);

    ui->browseLine->setText(tr("Choose file"));
    ui->updateProgressBar->hide();
    ui->checkUpdatesBtn->setEnabled(false);

    ui->openProgressBar->hide();
    ui->shortProgressBar->hide();
    ui->loadProgressBar->hide();

    QString path = Settings::setIniFile();
    m_settings = new QSettings(path, QSettings::IniFormat);
    m_settings->beginGroup("Settings");

    m_markersHintEnabled = m_settings->value("markersHintEnabled", true).toBool();
    m_graphHintEnabled = m_settings->value("graphHintEnabled", true).toBool();
    m_graphBriefHintEnabled = m_settings->value("graphBriefHintEnabled", true).toBool();
    m_restrictFq = m_settings->value("restrictFq", true).toBool();

    bool dark = m_settings->value("darkColorTheme", true).toBool();
    if (dark)
        ui->radioButtonDark->setChecked(true);
    else
        ui->radioButtonLight->setChecked(true);
    connect(ui->radioButtonDark, &QRadioButton::toggled, this, [this](bool checked) {
        m_settings->beginGroup("Settings");
        m_settings->setValue("darkColorTheme", checked);
        m_settings->endGroup();
        //QMessageBox::warning(this, tr("Color Theme"), tr("You must reload the program to change the color theme."), QMessageBox::Ok);
    });

    ui->tabWidget->setCurrentIndex(m_settings->value("currentIndex",0).toInt());
    ui->markersHintCheckBox->setChecked(m_markersHintEnabled);
    ui->graphHintCheckBox->setChecked(m_graphHintEnabled);
    ui->graphBriefHintCheckBox->setChecked(m_graphBriefHintEnabled);

    ui->spinBoxMeasurements->setValue(g_maxMeasurements);
    // TODO developer(?)
    ui->fqRestrictCheckBox->setChecked(g_developerMode ? m_restrictFq : true);
    //if (!g_developerMode) {
        ui->fqRestrictCheckBox->setVisible(false);
    //}
    // ///
    m_settings->endGroup();

    connect(ui->lineEdit_systemImpedance, &QLineEdit::editingFinished, this, &Settings::on_systemImpedance);

    ui->cableComboBox->addItem(tr("Change parameters or choose from list..."));
    ui->cableComboBox->setStyleSheet("QComboBox { combobox-popup: 0; }");
    ui->cableComboBox->setMaxVisibleItems(20);

    connect(ui->lineEditMin, &QLineEdit::editingFinished, this, &Settings::on_fqMinFinished);
    connect(ui->lineEditMax, &QLineEdit::editingFinished, this, &Settings::on_fqMaxFinished);

    ui->lineEditPoints->setText("500");
    connect(ui->lineEditPoints, &QLineEdit::editingFinished, this, &Settings::on_PointsFinished);
    connect(ui->exportBtn, &QPushButton::clicked, this, &Settings::on_exportCableSettings);

    //{
    // TODO Bug #2247: update doesn't work from Antscope2
    ui->tabWidget->removeTab(4);
    //}

    if (!g_developerMode) {
        ui->tabWidget->removeTab(3);
    } else {
        initCustomizeTab();
    }

    QString cablesPath = Settings::programDataPath("cables.txt");
    openCablesFile(cablesPath);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->serialPortComboBox->addItem(info.portName());
    }
    connect(ui->closeBtn, SIGNAL(pressed()), this, SLOT(close()));
    ui->closeBtn->setFocus();

    // TODO not implemented yet
    ui->licencesBtn->setVisible(false);
}

Settings::~Settings()
{
    double Z0 = ui->lineEdit_systemImpedance->text().toDouble();
    if((Z0 > 0) && (Z0 <= 1000))
    {
        emit Z0Changed(Z0);
    }

    CustomAnalyzer::save();

    g_maxMeasurements = ui->spinBoxMeasurements->value();

    m_settings->beginGroup("Settings");
    m_settings->setValue("markersHintEnabled", m_markersHintEnabled);
    m_settings->setValue("graphHintEnabled", m_graphHintEnabled);
    m_settings->setValue("graphBriefHintEnabled", m_graphBriefHintEnabled);
    m_settings->setValue("restrictFq", m_restrictFq);
    m_settings->setValue("maxMeasurements", g_maxMeasurements);
    m_settings->setValue("darkColorTheme", ui->radioButtonDark->isChecked());

    m_settings->setValue("currentIndex",ui->tabWidget->currentIndex());
    m_settings->endGroup();

    // auto calibration
    m_settings->beginGroup("Auto-calibration");
    m_settings->setValue("cable_length_min", ui->lineEditMinLength->text().toDouble());
    m_settings->setValue("cable_length_max", ui->lineEditMaxLength->text().toDouble());
    m_settings->setValue("cable_length_steps", ui->lineEditStepLength->text().toDouble());
    m_settings->setValue("cable_res_min", ui->lineEditMinR->text().toDouble());
    m_settings->setValue("cable_res_max", ui->lineEditMaxR->text().toDouble());
    m_settings->setValue("cable_res_steps", ui->lineEditStepR->text().toDouble());
    m_settings->endGroup();

    m_settings->beginGroup("MainWindow");
    m_settings->setValue("measureSystemMetric", m_metricChecked);
    m_settings->endGroup();

    if(m_analyzer != NULL)
    {
        m_analyzer->setIsMeasuring(false);
    }

    if(m_generalTimer)
    {
        m_generalTimer->stop();
        delete m_generalTimer;
        m_generalTimer = NULL;
    }
    emit paramsChanged();

    delete ui;
}

void Settings::setZ0(double _Z0)
{
    ui->lineEdit_systemImpedance->setText(QString::number(_Z0));
}


void Settings::on_browseBtn_clicked()
{
    QFileInfo info;
    QString path;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open firmware file"),
                                                 "","*.bin");
    m_path = fileName;
    ui->browseLine->setText(fileName);

    if (!fileName.isEmpty()) {

        info.setFile(fileName);
        path = info.absolutePath();
        m_isComplete = m_analyzer->checkFile(m_path);
        if (m_isComplete) {
            m_pathToFw = fileName;
            ui->updateBtn->setEnabled(true);
        }
    } else {
        ui->browseLine->setText(tr("Choose file"));
        m_isComplete = false;
    }
}

void Settings::on_checkUpdatesBtn_clicked()
{
    ui->checkUpdatesBtn->setText(tr("Checking"));
    if(m_generalTimer)
    {
        m_generalTimer->stop();
        delete m_generalTimer;
    }
    m_generalTimer = new QTimer(this);
    connect(m_generalTimer, SIGNAL(timeout()), this, SLOT(on_generalTimerTick()));
    m_generalTimer->start(200);
    emit checkUpdatesBtn();
}

void Settings::on_generalTimerTick()
{
    static qint32 state = 0;
    static qint32 ticks = 0;
    ticks++;
    if(ticks >= 25)
    {
        ui->checkUpdatesBtn->setText(tr("Check Updates"));
        ticks = 0;
        m_generalTimer->stop();
        return;
    }
    QString strChecking = tr("Checking");
    switch(state)
    {
    case 0 :
        state++;
        ui->checkUpdatesBtn->setText(strChecking);
        break;
    case 1 :
        state++;
        ui->checkUpdatesBtn->setText(strChecking + ".");
        break;
    case 2 :
        state++;
        ui->checkUpdatesBtn->setText(strChecking + "..");
        break;
    case 3 :
        state = 0;
        ui->checkUpdatesBtn->setText(strChecking + "...");
        break;
    default:
        state = 0;
        break;
    }
}

void Settings::setAnalyzer(Analyzer * analyzer)
{
    if(analyzer)
    {
        m_analyzer = analyzer;

        connect(ui->licencesBtn, &QPushButton::pressed, this, &Settings::on_licensesBtnPressed);

        qint32 num =  m_analyzer->getModel();
        if(num != 0)
        {
            ui->checkUpdatesBtn->setEnabled(true);
            ui->analyzerModelLabel->setText(m_analyzer->getModelString());
            ui->serialLabel->setText(m_analyzer->getSerialNumber());
            QString version = QString::number(m_analyzer->getVersion());
            if(version.length() == 3)
            {
                version.insert(1,".");
            }
            ui->versionLabel->setText(version);
        }else
        {
            ui->checkUpdatesBtn->setEnabled(false);
            m_analyzer->closeComPort();
            findBootloader();
        }
    }
}

void Settings::setCalibration(Calibration * calibration)
{
    if(calibration)
    {
        m_calibration = calibration;
        ui->labelOpenState->setText(m_calibration->getOpenFileName());
        ui->labelShortState->setText(m_calibration->getShortFileName());
        ui->labelLoadState->setText(m_calibration->getLoadFileName());
        ui->lineEditPoints->setText(QString::number(m_calibration->dotsNumber()));

        if(m_calibration->getCalibrationPerformed())
        {
            if(m_calibration->getCalibrationEnabled())
            {
                emit calibrationEnabled(true);
            }
            else
            {
                emit calibrationEnabled(false);
            }
        }
    }
}

void Settings::setGraphHintChecked(bool checked)
{
    ui->graphHintCheckBox->setChecked(checked);
}

void Settings::setGraphBriefHintChecked(bool checked)
{
    ui->graphBriefHintCheckBox->setChecked(checked);
}

void Settings::setMarkersHintChecked(bool checked)
{
    ui->markersHintCheckBox->setChecked(checked);
}

void Settings::findBootloader (void)
{
    static qint32 counter = 0;
    QList<ReDeviceInfo> list;
    list = ReDeviceInfo::availableDevices(ReDeviceInfo::Serial);
    for(int n = 0; n < list.length(); ++n)
    {
        QString name = ReDeviceInfo::deviceName(list.at(n));
        for(int i = QUANTITY-1; i > 0; i--)
        {
            if(names[i].indexOf(name) >= 0 )
            {
                m_analyzer->setComAnalyzerFound(true);
                m_analyzer->setAnalyzerModel(i);
                m_analyzer->openComPort(list.at(n).portName(),115200);
                ui->analyzerModelLabel->setText(name);
                ui->checkUpdatesBtn->setEnabled(true);
                break;
            }
        }
    }
    if (list.length() == 0)
    {
        int model = m_analyzer->getHidModel();
        if( model != 0)
        {
            m_analyzer->setHidAnalyzerFound(true);
            m_analyzer->setAnalyzerModel(model);
            ui->analyzerModelLabel->setText(names[model]);
            ui->checkUpdatesBtn->setEnabled(true);
        }else
        {
            counter++;
            if(counter < 5)
            {
                QTimer::singleShot(300,this, SLOT(findBootloader()));
            }
            ui->checkUpdatesBtn->setEnabled(false);
        }
    }
    if((m_analyzer->getAnalyzerModel() == 0) && (counter == 5))
    {
        counter = 0;
        QTimer::singleShot( 1, m_analyzer, SLOT(on_findAA30ZERO()));
    }
}

void Settings::on_autoUpdatesCheckBox_clicked(bool checked)
{
    emit autoUpdatesCheckBox(checked);
}

void Settings::on_updateBtn_clicked()
{
    ui->updateBtn->setEnabled(false);
    ui->updateBtn->setText("Updating...");
    ui->updateProgressBar->show();
    emit updateBtn(m_pathToFw);
}

void Settings::on_percentChanged(qint32 percent)
{
    if(percent == 100)
    {
        ui->updateBtn->setText("Update");
        ui->updateBtn->setEnabled(true);
        ui->updateProgressBar->hide();
        ui->updateProgressBar->setValue(0);
    }
    ui->updateProgressBar->setValue(percent);
}

void Settings::on_graphHintCheckBox_clicked(bool checked)
{
    emit graphHintChecked(checked);
    m_graphHintEnabled = checked;
}

void Settings::on_graphBriefHintCheckBox_clicked(bool checked)
{
    emit graphBriefHintChecked(checked);
    m_graphBriefHintEnabled = checked;
}

void Settings::on_markersHintCheckBox_clicked(bool checked)
{
    emit markersHintChecked(checked);
    m_markersHintEnabled = checked;
}

void Settings::on_fqRestrictCheckBox_clicked(bool checked)
{
    emit fqRestrictChecked(!checked);
    m_restrictFq = !checked;
}

void Settings::on_calibWizard_clicked()
{
    enableButtons(false);
    QMessageBox::information(NULL, tr("Open"),
                         tr("Please connect OPEN standard and press OK."));
    emit startCalibration();
}

void Settings::on_percentCalibrationChanged(qint32 state, qint32 percent)
{
    switch (state) {
    case 1:
        ui->openProgressBar->setValue(percent);
        if(percent == 100)
        {
            if(m_onlyOneCalib)
            {
                ui->openProgressBar->hide();
                ui->labelOpenState->setText("cal_open.s1p");
                m_onlyOneCalib = false;
                enableButtons(true);
            }
        }else
        {
            ui->openProgressBar->show();
        }
        break;
    case 2:
        ui->shortProgressBar->show();
        ui->shortProgressBar->setValue(percent);
        if(percent == 100)
        {
            if(m_onlyOneCalib)
            {
                ui->shortProgressBar->hide();
                ui->labelShortState->setText("cal_short.s1p");
                m_onlyOneCalib = false;
                enableButtons(true);
            }
        }
        break;
    case 3:
        ui->loadProgressBar->show();
        ui->loadProgressBar->setValue(percent);
        if(percent == 100)
        {
            if(m_onlyOneCalib)
            {
                ui->loadProgressBar->hide();
                ui->labelLoadState->setText("cal_load.s1p");
                m_onlyOneCalib = false;
            }else
            {
                ui->openProgressBar->hide();
                ui->shortProgressBar->hide();
                ui->loadProgressBar->hide();
                ui->labelOpenState->setText("cal_open.s1p");
                ui->labelShortState->setText("cal_short.s1p");
                ui->labelLoadState->setText("cal_load.s1p");
            }
            enableButtons(true);
        }
        break;
    default:
        break;
    }
}

void Settings::on_openCalibBtn_clicked()
{
    enableButtons(false);
    m_onlyOneCalib = true;
    if (QMessageBox::information(NULL, tr("Open"),
                         tr("Please connect OPEN standard and press OK.")) == QMessageBox::Ok)
        emit startCalibrationOpen();
}

void Settings::on_shortCalibBtn_clicked()
{
    enableButtons(false);
    m_onlyOneCalib = true;
    QMessageBox::information(NULL, tr("Short"),
                         tr("Please connect SHORT standard and press OK."));
    emit startCalibrationShort();
}

void Settings::on_loadCalibBtn_clicked()
{
    enableButtons(false);
    m_onlyOneCalib = true;
    QMessageBox::information(NULL, tr("Load"),
                         tr("Please connect LOAD standard and press OK."));
    emit startCalibrationLoad();
}


void Settings::enableButtons(bool enabled)
{
    ui->openOpenFileBtn->setEnabled(enabled);
    ui->openCalibBtn->setEnabled(enabled);
    ui->shortOpenFileBtn->setEnabled(enabled);
    ui->shortCalibBtn->setEnabled(enabled);
    ui->loadOpenFileBtn->setEnabled(enabled);
    ui->loadCalibBtn->setEnabled(enabled);

    ui->calibWizard->setEnabled(enabled);
}

void Settings::on_openOpenFileBtn_clicked()
{
    QString dir = localDataPath("Calibration");
    QString path = QFileDialog::getOpenFileName(this, tr("Open 'open calibration' file"),
                                                dir,"*.s1p");
    QStringList list;
    list = path.split("/");
    if(list.length() == 1)
    {
        list.clear();
        list = path.split("\\");
    }
    ui->labelOpenState->setText(list.last());
    emit openOpenFile(path);
}

void Settings::on_shortOpenFileBtn_clicked()
{
    QString dir = localDataPath("Calibration");
    QString path = QFileDialog::getOpenFileName(this, tr("Open 'short calibration' file"),
                                             dir,"*.s1p");
    QStringList list;
    list = path.split("/");
    if(list.length() == 1)
    {
        list.clear();
        list = path.split("\\");
    }
    ui->labelShortState->setText(list.last());

    emit shortOpenFile(path);
}

void Settings::on_loadOpenFileBtn_clicked()
{
    QString dir = localDataPath("Calibration");
    QString path = QFileDialog::getOpenFileName(this, tr("Open 'load calibration' file"),
                                                dir,"*.s1p");
    QStringList list;
    list = path.split("/");
    if(list.length() == 1)
    {
        list.clear();
        list = path.split("\\");
    }
    ui->labelLoadState->setText(list.last());
    emit loadOpenFile(path);
}

void Settings::setMeasureSystemMetric(bool state)
{
    ui->measureSystemMetric->setChecked(state);
    m_metricChecked = state;

    ui->measureSystemAmerican->setChecked(!state);
    m_americanChecked = !state;
}

void Settings::on_measureSystemMetric_clicked(bool checked)
{
    if(!checked)
    {
        if(m_metricChecked)
        {
            ui->measureSystemMetric->setChecked(true);
        }else
        {
            m_metricChecked = checked;
        }
    }else
    {
        emit changeMeasureSystemMetric(checked);
        m_metricChecked = checked;
        m_americanChecked = !checked;
        ui->measureSystemAmerican->setChecked(!checked);
    }
}

void Settings::on_measureSystemAmerican_clicked(bool checked)
{
    if(!checked)
    {
        if(m_americanChecked)
        {
            ui->measureSystemAmerican->setChecked(true);
        }else
        {
            m_americanChecked = checked;
        }
    }else
    {
        emit changeMeasureSystemMetric(!checked);
        m_americanChecked = checked;
        m_metricChecked = !checked;
        ui->measureSystemMetric->setChecked(!checked);
    }
}

void Settings::on_doNothingBtn_clicked(bool checked)
{
    if( !checked )
    {
        if(m_farEndMeasurement == 0)
        {
            ui->doNothingBtn->setChecked(true);
        }
    }else
    {
        m_farEndMeasurement = 0;
        ui->addCableBtn->setChecked(false);
        ui->subtractCableBtn->setChecked(false);
        emit paramsChanged();
        cableActionEnableButtons(false);
    }
}

void Settings::on_subtractCableBtn_clicked(bool checked)
{
    if( !checked )
    {
        if(m_farEndMeasurement == 1)
        {
            ui->subtractCableBtn->setChecked(true);
        }
    }else
    {
        m_farEndMeasurement = 1;
        ui->addCableBtn->setChecked(false);
        ui->doNothingBtn->setChecked(false);
        emit paramsChanged();
        cableActionEnableButtons(true);
    }
}

void Settings::on_addCableBtn_clicked(bool checked)
{
    if( !checked )
    {
        if(m_farEndMeasurement == 2)
        {
            ui->addCableBtn->setChecked(true);
        }
    }else
    {
        m_farEndMeasurement = 2;
        ui->subtractCableBtn->setChecked(false);
        ui->doNothingBtn->setChecked(false);
        emit paramsChanged();
        cableActionEnableButtons(true);
    }
}

void Settings::cableActionEnableButtons(bool enabled)
{
    ui->cableR0->setEnabled(enabled);
    ui->cableLossComboBox->setEnabled(enabled);
    ui->cableLen->setEnabled(enabled);
    ui->conductiveLoss->setEnabled(enabled);
    ui->dielectricLoss->setEnabled(enabled);
    ui->atFq->setEnabled(enabled);
    ui->anyFq->setEnabled(enabled);
}


//Cable-------------------------------------------------------------------------
void Settings::setCableVelFactor(double value)
{
    ui->velocityFactor->setText(QString::number(value,'f',2));
}
double Settings::getCableVelFactor(void)const
{
    return ui->velocityFactor->text().toDouble();
}
//------------------------------------------------------------------------------
void Settings::setCableResistance(double value)
{
    ui->cableR0->setText(QString::number(value));
}
double Settings::getCableResistance(void)const
{
    return ui->cableR0->text().toDouble();
}
//------------------------------------------------------------------------------
void Settings::setCableLossConductive(double value)
{
    ui->conductiveLoss->setText(QString::number(value));
}
double Settings::getCableLossConductive(void)const
{
    return ui->conductiveLoss->text().toDouble();
}
//------------------------------------------------------------------------------
void Settings::setCableLossDielectric(double value)
{
    ui->dielectricLoss->setText(QString::number(value));
}
double Settings::getCableLossDielectric(void)const
{
    return ui->dielectricLoss->text().toDouble();
}
//------------------------------------------------------------------------------
void Settings::setCableLossFqMHz(double value)
{
    ui->atMHz->setText(QString::number(value));
}
double Settings::getCableLossFqMHz(void)const
{
    return ui->atMHz->text().toDouble();
}
//------------------------------------------------------------------------------
void Settings::setCableLossUnits(int value)
{
    ui->cableLossComboBox->setCurrentIndex(value);
}
int Settings::getCableLossUnits(void)const
{
    return ui->cableLossComboBox->currentIndex();
}
//------------------------------------------------------------------------------
void Settings::setCableLossAtAnyFq(bool value)
{
    if(value)
    {
        ui->anyFq->setChecked(value);
    }else
    {
        ui->atFq->setChecked(!value);
    }
}
bool Settings::getCableLossAtAnyFq(void)const
{
    return ui->anyFq->isChecked();
}
//------------------------------------------------------------------------------
void Settings::setCableLength(double value)
{
    ui->cableLen->setText(QString::number(value));
}
double Settings::getCableLength(void)const
{
    return ui->cableLen->text().toDouble();
}
//------------------------------------------------------------------------------
void Settings::setCableFarEndMeasurement(int value)
{
    m_farEndMeasurement = value;
    if(m_farEndMeasurement == 0)
    {
        ui->doNothingBtn->setChecked(true);
        cableActionEnableButtons(false);
    }else if(m_farEndMeasurement == 1)
    {
        ui->subtractCableBtn->setChecked(true);
    }else if(m_farEndMeasurement == 2)
    {
        ui->addCableBtn->setChecked(true);
    }
}
int Settings::getCableFarEndMeasurement(void)const
{
    return m_farEndMeasurement;
}
//------------------------------------------------------------------------------
void Settings::setCableIndex(int value)
{
    if(value >= 0)
        ui->cableComboBox->setCurrentIndex(value);
}
int Settings::getCableIndex(void)const
{
    return ui->cableComboBox->currentIndex();
}
//------------------------------------------------------------------------------


void Settings::openCablesFile(QString path)
{
    m_cablesList.clear();

    ui->cableComboBox->addItem(tr("Ideal 50-Ohm cable"));
    m_cablesList.append(tr("Ideal 50-Ohm cable, 50, 0.66, 0.0, 0.0, 0, 0"));
    ui->cableComboBox->addItem(tr("Ideal 75-Ohm cable"));
    m_cablesList.append(tr("Ideal 75-Ohm cable, 75, 0.66, 0.0, 0.0, 0, 0"));
    ui->cableComboBox->addItem(tr("Ideal 25-Ohm cable"));
    m_cablesList.append(tr("Ideal 25-Ohm cable, 25, 0.66, 0.0, 0.0, 0, 0"));
    ui->cableComboBox->addItem(tr("Ideal 37.5-Ohm cable"));
    m_cablesList.append(tr("Ideal 37.5-Ohm cable, 37.5, 0.66, 0.0, 0.0, 0, 0"));

    if (path.isEmpty())
        return;

    QFile file(path);
    bool res = file.open(QFile::ReadOnly);
    if(!res)
    {
        QMessageBox::information(this, tr("Can't open file"), path, QMessageBox::Close);
        return;
    }

    QTextStream in(&file);
    QString line;

    do
    {
        line = in.readLine();

        if( (line == "") || (line.at(0) == ';'))
        {
            continue;
        }else
        {
            QList <QString> list;
            list = line.split(',');
            if(list.length() == 7)
            {
                ui->cableComboBox->addItem(list.at(0));
                m_cablesList.append(line);
            }else
            {
                qDebug() << "Settings::openCablesFile: Error: Len != 7";
            }
        }
    } while (!line.isNull());
}


void Settings::on_cableComboBox_currentIndexChanged(int index)
{
    if(index > 0)
    {
        QString str = m_cablesList.at(index-1);
        QList <QString> paramsList = str.split(',');
        //1. Cable name
        ui->cableR0->setText( paramsList.at(1));//2. R0 in Ohm
        ui->velocityFactor->setText(paramsList.at(2));//3. Velocity factor
        ui->conductiveLoss->setText(paramsList.at(3));//4. Conductive loss
        ui->dielectricLoss->setText(paramsList.at(4));//5. Dielectric loss
        ui->cableLossComboBox->setCurrentIndex(paramsList.at(5).toInt());//6. Loss units (0=dB/100ft, 1=dB/ft, 2=dB/100m, 3=dB/m)
        bool anyFq = (bool)paramsList.at(6).toInt();//7. Frequency in MHz at which loss is specified (or 0 for any frequency)
        if(!anyFq)
        {
            ui->anyFq->setChecked(true);
        }else
        {
            ui->atFq->setChecked(true);
        }
    }
}

void Settings::on_updateGraphsBtn_clicked()
{
    emit paramsChanged();
}

QString Settings::setIniFile()
{
    //iniFilePath = Settings::localDataPath() + "AntScope2.ini";
    iniFilePath = Settings::localDataPath("AntScope2.ini");
    return iniFilePath;
}

QString Settings::localDataPath(QString _fileName)
{
// Mac OS X and iOS
#ifdef Q_OS_DARWIN
    QDir dir_ini3 = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return dir_ini3.absoluteFilePath("RigExpert/AntScope2/" + _fileName);
#endif
// Linux
#ifdef Q_OS_LINUX
    extern bool g_raspbian;
    if (g_raspbian)
    {
        return "/usr/share/RigExpert/AntScope2/" + _fileName;
    }
    QDir dir_ini2 = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return dir_ini2.absoluteFilePath("RigExpert/AntScope2/" + _fileName);
#endif
// Windows
#ifdef Q_OS_WIN
    QDir dir_ini1 = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    return dir_ini1.absoluteFilePath("RigExpert/AntScope2/" + _fileName);
#endif
  qDebug("TODO Settings::localDataPath");
  return QString();
}


QString Settings::languageDataFolder()
{
    extern bool g_raspbian;
    if (g_raspbian)
    {
        return "/usr/share/RigExpert/AntScope2";
    }
    return QCoreApplication::applicationDirPath();
}

QString Settings::programDataPath(QString _fileName)
{
// Mac OS X and iOS
#ifdef Q_OS_DARWIN
    QDir dir = QCoreApplication::applicationDirPath();
    return dir.absoluteFilePath("Resources/" + _fileName);
#endif

// win32 and win64
#ifdef Q_OS_WIN
    QStringList list = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    qDebug() << list;

    for (int idx=0; idx<list.size(); idx++)
    {
        QDir dir = list[idx];
        QString path = dir.absoluteFilePath("RigExpert/AntScope2/" + _fileName);
        if (QFile::exists(path))
            return path;
    }
    return QString();
#endif
  //qDebug("TODO Settings::programDataPath");
// Linux
#ifdef Q_OS_LINUX
    extern bool g_raspbian;
    if (g_raspbian)
    {
        return "/usr/share/RigExpert/AntScope2";
    }

    QDir dir = QCoreApplication::applicationDirPath();
    QString name = dir.absoluteFilePath("Resources/" + _fileName);
    return name;
#endif

  return QString();
}

void Settings::on_aa30bootFound()
{
    ui->serialLabel->setText(m_analyzer->getSerialNumber());
    ui->analyzerModelLabel->setText(m_analyzer->getModelString());
    QString version = QString::number(m_analyzer->getVersion());
    ui->versionLabel->setText(version);
    ui->checkUpdatesBtn->setEnabled(true);
}

void Settings::on_aa30updateComplete()
{
    this->close();
}

void Settings::on_autoUpdatesCheckBox(bool checked)
{
    emit firmwareAutoUpdateStateChanged(checked);
}

void Settings::on_checkBox_AntScopeAutoUpdate_clicked(bool checked)
{
    emit antScopeAutoUpdateStateChanged(checked);
}

void Settings::setFirmwareAutoUpdate(bool checked)
{
    ui->autoUpdatesCheckBox->setChecked(checked);
}

void Settings::setAntScopeAutoUpdate(bool checked)
{
    ui->checkBox_AntScopeAutoUpdate->setChecked(checked);
}

void Settings::setAntScopeVersion(QString version)
{
    ui->antScopeVersion->setText(version);
}

void Settings::setAutoDetectMode(bool state, QString portName)
{
    ui->autoDetect->setChecked(state);
    m_autoDetectChecked = state;
    ui->manualDetect->setChecked(!state);
    m_manualDetectChecked = !state;

    ui->serialPortComboBox->setEnabled(!state);

    int found = -1;
    for(int i = 0; i < ui->serialPortComboBox->count(); ++i)
    {
        if( portName == ui->serialPortComboBox->itemText(i))
        {
            found = i;
            break;
        }
    }
    if(found >= 0)
    {
        ui->serialPortComboBox->setCurrentIndex(found);
    }else
    {
        ui->serialPortComboBox->addItem(portName);
        ui->serialPortComboBox->setCurrentText(portName);
    }
}

void Settings::on_autoDetect_clicked(bool checked)
{
    if(!checked)
    {
        if(m_autoDetectChecked)
        {
            ui->autoDetect->setChecked(true);
        }else
        {
            m_manualDetectChecked = checked;
        }
    }else
    {
        emit changedAutoDetectMode(true);
        m_autoDetectChecked = checked;
        m_manualDetectChecked = !checked;
        ui->manualDetect->setChecked(!checked);
    }
    ui->serialPortComboBox->setEnabled(false);
}

void Settings::on_manualDetect_clicked(bool checked)
{
    if(!checked)
    {
        if(m_manualDetectChecked)
        {
            ui->manualDetect->setChecked(true);
        }else
        {
            m_autoDetectChecked = checked;
        }
    }else
    {
        emit changedAutoDetectMode(false);
        m_manualDetectChecked = checked;
        m_autoDetectChecked = !checked;
        ui->autoDetect->setChecked(!checked);
    }
    ui->serialPortComboBox->setEnabled(true);
}

void Settings::on_serialPortComboBox_activated(const QString &arg1)
{
    emit changedSerialPort(arg1);
}

void Settings::setLanguages(QStringList list, int number)
{
    ui->languageComboBox->addItems(list);
    ui->languageComboBox->setCurrentIndex(number);
}

void Settings::on_translate()
{
    ui->retranslateUi(this);
    ui->cableComboBox->setItemText(0, tr("Change parameters or choose from list..."));
}

void Settings::on_languageComboBox_currentIndexChanged(int index)
{
    emit languageChanged(index);
}

void Settings::on_closeButton_clicked()
{
    accept();
}

void Settings::onBandsComboBox_currentIndexChanged(int index)
{
    QString band = ui->bandsCombobox->itemText(index);

    m_settings->beginGroup("Settings");
    m_settings->setValue("current_band", band);
    m_settings->endGroup();

    emit bandChanged(band);
}

void Settings::setBands(QList<QString> list)
{
    foreach (QString band, list) {
        ui->bandsCombobox->addItem(band);
    }

    m_settings->beginGroup("Settings");
    QString current_band = m_settings->value("current_band", "").toString();
    m_settings->endGroup();
    connect(ui->bandsCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(onBandsComboBox_currentIndexChanged(int)));
    ui->bandsCombobox->setCurrentText(current_band);
}

void Settings::initCustomizeTab()
{
    ui->comboBoxName->blockSignals(true);
    ui->comboBoxPrototype->blockSignals(true);

    ui->comboBoxPrototype->hide();
    ui->label_19->hide();

    ui->comboBoxPrototype->clear();
    ui->comboBoxName->clear();


    //CustomAnalyzer::load(m_settings);
    QString curAlias = CustomAnalyzer::currentAlias();
    for (int idx=0; idx<QUANTITY; idx++) {
        ui->comboBoxPrototype->addItem(names[idx]);
    }
    const QMap<QString, CustomAnalyzer>& map = CustomAnalyzer::getMap();
    QStringList keys = map.keys();
    for (int idx=0; idx<keys.size(); idx++) {
        ui->comboBoxName->addItem(map[keys[idx]].alias());
    }

    CustomAnalyzer::setCurrent(curAlias);
    CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
    if (ca != nullptr) {
        ui->comboBoxName->setCurrentText(ca->alias());
        ui->lineEditMin->setText(ca->minFq());
        ui->lineEditMax->setText(ca->maxFq());
        ui->spinBoxWidth->setValue(ca->width());
        ui->spinBoxHeight->setValue(ca->height());
        ui->comboBoxPrototype->setCurrentText(ca->prototype());
    } else {
        on_comboBoxName_currentIndexChanged(ui->comboBoxName->currentIndex());
    }

    connect(ui->customizeCheckBox, &QCheckBox::toggled, this, &Settings::on_enableCustomizeControls);
    connect(ui->btnAdd, &QPushButton::clicked, this, &Settings::on_addButton);
    connect(ui->btnRemove, &QPushButton::clicked, this, &Settings::on_removeButton);
    connect(ui->btnAply, &QPushButton::clicked, this, &Settings::onApplyButton);
    connect(ui->comboBoxPrototype, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxPrototype_currentIndexChanged(int)));
    connect(ui->comboBoxName, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBoxName_currentIndexChanged(int)));
    ui->customizeCheckBox->setChecked(CustomAnalyzer::customized());

    ui->comboBoxName->blockSignals(false);
    ui->comboBoxPrototype->blockSignals(false);
    on_enableCustomizeControls(CustomAnalyzer::customized());

    // auto calibration
    m_settings->beginGroup("Auto-calibration");
    ui->lineEditMinLength->setText(QString::number(m_settings->value("cable_length_min", 0).toDouble()));
    ui->lineEditMaxLength->setText(QString::number(m_settings->value("cable_length_max", 0.02).toDouble()));
    ui->lineEditStepLength->setText(QString::number(m_settings->value("cable_length_steps", 100).toDouble()));
    ui->lineEditMinR->setText(QString::number(m_settings->value("cable_res_min", 20).toDouble()));
    ui->lineEditMaxR->setText(QString::number(m_settings->value("cable_res_max", 40).toDouble()));
    ui->lineEditStepR->setText(QString::number(m_settings->value("cable_res_steps", 100).toDouble()));
    m_settings->endGroup();
}

void Settings::on_enableCustomizeControls(bool enable)
{
    ui->comboBoxName->setEnabled(enable);
    ui->comboBoxPrototype->setEnabled(enable);
    ui->lineEditMin->setEnabled(enable);
    ui->lineEditMax->setEnabled(enable);
    ui->spinBoxWidth->setEnabled(enable);
    ui->spinBoxHeight->setEnabled(enable);
    ui->btnAdd->setEnabled(enable);
    ui->btnRemove->setEnabled(enable);
    CustomAnalyzer::customize(enable);
}

void Settings::on_comboBoxPrototype_currentIndexChanged(int index)
{
    if (index < 0)
        return;
    ui->lineEditMin->setText(minFq[index]);
    ui->lineEditMax->setText(maxFq[index]);
    ui->spinBoxWidth->setValue(lcdWidth[index]);
    ui->spinBoxHeight->setValue(lcdHeight[index]);
}

void Settings::on_comboBoxName_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    QString key = ui->comboBoxName->currentText();
    if (!key.isEmpty()) {
        CustomAnalyzer::setCurrent(key);
        CustomAnalyzer* ca = CustomAnalyzer::get(key);
        if (ca != nullptr) {
            ui->comboBoxName->setCurrentText(ca->alias());
            ui->lineEditMin->setText(ca->minFq());
            ui->lineEditMax->setText(ca->maxFq());
            ui->spinBoxWidth->setValue(ca->width());
            ui->spinBoxHeight->setValue(ca->height());
            ui->comboBoxPrototype->setCurrentText(ca->prototype());
        }
    }
}

void Settings::onApplyButton()
{
    if (ui->comboBoxName->currentText().isEmpty())
        return;
    CustomAnalyzer ca;
    ca.setAlias(ui->comboBoxName->currentText());
    ca.setPrototype(ui->comboBoxPrototype->currentText());
    ca.setMinFq(ui->lineEditMin->text());
    ca.setMaxFq(ui->lineEditMax->text());
    ca.setWidth(ui->spinBoxWidth->value());
    ca.setHeight(ui->spinBoxHeight->value());
    CustomAnalyzer::add(ca);
    CustomAnalyzer::setCurrent(ca.alias());
    CustomAnalyzer::save();

    // auto calibration
    m_settings->beginGroup("Auto-calibration");
    m_settings->setValue("cable_length_min", ui->lineEditMinLength->text().toDouble());
    m_settings->setValue("cable_length_max", ui->lineEditMaxLength->text().toDouble());
    m_settings->setValue("cable_length_steps", ui->lineEditStepLength->text().toDouble());
    m_settings->setValue("cable_res_min", ui->lineEditMinR->text().toDouble());
    m_settings->setValue("cable_res_max", ui->lineEditMaxR->text().toDouble());
    m_settings->setValue("cable_res_steps", ui->lineEditStepR->text().toDouble());
    m_settings->endGroup();

    initCustomizeTab();
}

void Settings::on_removeButton()
{
    if (ui->comboBoxName->currentText().isEmpty())
        return;
    CustomAnalyzer::remove(ui->comboBoxName->currentText());
    CustomAnalyzer::save();
    initCustomizeTab();
}

void Settings::on_addButton()
{
    ui->comboBoxName->setCurrentText("");
    ui->comboBoxPrototype->setCurrentText("names[0]");
    ui->lineEditMin->setText("0");
    ui->lineEditMax->setText("0");
    ui->spinBoxWidth->setValue(0);
    ui->spinBoxHeight->setValue(0);
}

void Settings::on_fqMinFinished()
{
    QString str = ui->lineEditMin->text();
    str.remove(' ');
    ui->lineEditMin->setText(appendSpaces(str));
}

void Settings::on_fqMaxFinished()
{
    QString str = ui->lineEditMax->text();
    str.remove(' ');
    ui->lineEditMax->setText(appendSpaces(str));
}

void Settings::on_PointsFinished()
{
    QString str = ui->lineEditPoints->text();
    m_calibration->setDotsNumber(str.toInt());
}

void Settings::on_systemImpedance()
{
    qDebug() << "Settings::on_systemImpedance";
    double Z0 = ui->lineEdit_systemImpedance->text().toDouble();
    if((Z0 > 0) && (Z0 <= 1000))
    {
        emit Z0Changed(Z0);
    }
}

void Settings::on_exportCableSettings()
{
    QString desc;
    if (m_farEndMeasurement != 0) {
        QString units="dB/100ft";
        int index = ui->cableLossComboBox->currentIndex();//(paramsList.at(5).toInt());//6. Loss units (0=dB/100ft, 1=dB/ft, 2=dB/100m, 3=dB/m)
        switch(index) {
            case 1: units="dB/ft"; break;
            case 2: units="dB/100m"; break;
            case 3: units="dB/m"; break;
        }
        QString fq = ui->anyFq->isChecked() ? "any frequency" : (ui->atMHz->text() + " MHz");

        desc += QString("! %1 cable:\n")
                .arg(m_farEndMeasurement==1?"Subtract":"Add");
        desc += QString("! Velocity factor %1\n")
                .arg(ui->velocityFactor->text().toDouble(), 0, 'f', 6, QChar(' '));
        desc += QString("! Length %1, R0 %2\n")
                .arg(ui->cableLen->text().toDouble(), 0, 'f', 6, QChar(' '))
                .arg(ui->cableR0->text().toDouble(), 0, 'f', 2, QChar(' '));
        desc += QString("! Conductive loss %1 %2 at %3\n")
                .arg(ui->conductiveLoss->text().toDouble(), 0, 'f', 6, QChar(' '))
                .arg(units)
                .arg(fq);
        desc += QString("! Dielectric loss %1 %2 at %3")
                .arg(ui->dielectricLoss->text().toDouble(), 0, 'f', 6, QChar(' '))
                .arg(units)
                .arg(fq);
    } else {
        desc = "! Ignore cable";
    }
    emit exportCableSettings(desc);
}


void Settings::on_licensesBtnPressed()
{
    LicensesDialog* dlg = new LicensesDialog(m_analyzer, nullptr);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("Licenses"));
    dlg->exec();
}

void Settings::on_connectNanovna()
{
    if (NanovnaAnalyzer::isConnected())
        emit disconnectNanoVNA();
    else
        emit connectNanoVNA();

    close();
    //QMessageBox::warning(this, "Connect NanoVNA", "Not implemented yet!", QMessageBox::Ok);
}



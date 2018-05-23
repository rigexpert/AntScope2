#include "settings.h"
#include "ui_settings.h"

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

    ui->tabWidget->setCurrentIndex(m_settings->value("currentIndex",0).toInt());
    ui->markersHintCheckBox->setChecked(m_markersHintEnabled);
    ui->graphHintCheckBox->setChecked(m_graphHintEnabled);
    ui->graphBriefHintCheckBox->setChecked(m_graphBriefHintEnabled);

    m_settings->endGroup();

    ui->cableComboBox->addItem(tr("Change parameters or choose from list..."));
    QString cablesPath = Settings::appPath();
    cablesPath += "cables.txt";
    openFile(cablesPath);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->serialPortComboBox->addItem(info.portName());
    }
}

Settings::~Settings()
{
    emit paramsChanged();
    double Z0 = ui->lineEdit_systemImpedance->text().toDouble();
    if((Z0 > 0) && (Z0 <= 100))
    {
        emit Z0Changed(Z0);
    }

    m_settings->beginGroup("Settings");
    m_settings->setValue("markersHintEnabled", m_markersHintEnabled);
    m_settings->setValue("graphHintEnabled", m_graphHintEnabled);
    m_settings->setValue("graphBriefHintEnabled", m_graphBriefHintEnabled);
    m_settings->setValue("currentIndex",ui->tabWidget->currentIndex());
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
    switch(state)
    {
    case 0 :
        state++;
        ui->checkUpdatesBtn->setText(tr("Checking"));
        break;
    case 1 :
        state++;
        ui->checkUpdatesBtn->setText(tr("Checking."));
        break;
    case 2 :
        state++;
        ui->checkUpdatesBtn->setText(tr("Checking.."));
        break;
    case 3 :
        state = 0;
        ui->checkUpdatesBtn->setText(tr("Checking..."));
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

        if(m_calibration->getCalibrationPerformed())
        {
            if(m_calibration->getCalibrationEnabled())
            {
                ui->turnOnOffBtn->setText("Turn Off");
                ui->labelWizardStatus->setText("Calibration is On");
                emit calibrationEnabled(true);
            }
            else
            {
                ui->turnOnOffBtn->setText("Turn On");
                ui->labelWizardStatus->setText("Calibration is Off");
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
        for(quint32 i = QUANTITY-1; i > 0; i--)
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
    QMessageBox::information(NULL, tr("Open"),
                         tr("Please connect OPEN standard and press OK."));
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

void Settings::on_turnOnOffBtn_clicked()
{
    if(m_calibration->getCalibrationPerformed())
    {
        if(ui->turnOnOffBtn->text() == "Turn On")
        {
            ui->turnOnOffBtn->setText("Turn Off");
            ui->labelWizardStatus->setText("Calibration is On");
            emit calibrationEnabled(true);
        }
        else if (ui->turnOnOffBtn->text() == "Turn Off")
        {
            ui->turnOnOffBtn->setText("Turn On");
            ui->labelWizardStatus->setText("Calibration is Off");
            emit calibrationEnabled(false);
        }
    }else
    {
        QMessageBox::information(NULL, tr("Calibration not performed"),
                              tr("Calibration not performed."));
    }
}

void Settings::enableButtons(bool enabled)
{
    ui->openOpenFileBtn->setEnabled(enabled);
    ui->openCalibBtn->setEnabled(enabled);
    ui->shortOpenFileBtn->setEnabled(enabled);
    ui->shortCalibBtn->setEnabled(enabled);
    ui->loadOpenFileBtn->setEnabled(enabled);
    ui->loadCalibBtn->setEnabled(enabled);

    ui->turnOnOffBtn->setEnabled(enabled);
    ui->calibWizard->setEnabled(enabled);
}

void Settings::on_openOpenFileBtn_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open 'open calibration' file"),
                                                "Calibration/","*.s1p");
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
    QString path = QFileDialog::getOpenFileName(this, tr("Open 'short calibration' file"),
                                             "Calibration/","*.s1p");
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
    QString path = QFileDialog::getOpenFileName(this, tr("Open 'load calibration' file"),
                                                "Calibration/","*.s1p");
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


void Settings::openFile(QString path)
{
    QFile file(path);
    bool res = file.open(QFile::ReadOnly);
    if(res)
    {
        m_cablesList.clear();
        QTextStream in(&file);
        QString line;

        ui->cableComboBox->addItem(tr("Ideal 50-Ohm cable"));
        m_cablesList.append(tr("Ideal 50-Ohm cable, 50, 0.66, 0.0, 0.0, 0, 0"));
        ui->cableComboBox->addItem(tr("Ideal 75-Ohm cable"));
        m_cablesList.append(tr("Ideal 75-Ohm cable, 75, 0.66, 0.0, 0.0, 0, 0"));
        ui->cableComboBox->addItem(tr("Ideal 25-Ohm cable"));
        m_cablesList.append(tr("Ideal 25-Ohm cable, 25, 0.66, 0.0, 0.0, 0, 0"));
        ui->cableComboBox->addItem(tr("Ideal 37.5-Ohm cable"));
        m_cablesList.append(tr("Ideal 37.5-Ohm cable, 37.5, 0.66, 0.0, 0.0, 0, 0"));

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
                    qDebug() << "Error: Len != 7";
                }
            }
        }while (!line.isNull());
    }
}


void Settings::on_cableComboBox_currentIndexChanged(int index)
{
    if(index != 0)
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
    iniFilePath = Settings::appPath() + "AntScope2.ini";
    return iniFilePath;
}

QString Settings::appPath()
{
    WCHAR path[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);
    QDir dir(QString::fromWCharArray(path));
    return dir.absoluteFilePath("RigExpert/AntScope2/");
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

void Settings::on_checkBox_AntScopeAutoUpdate_clicked(bool checked)
{
    emit antScopeAutoUpdateStateChanged(checked);
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
}

void Settings::on_languageComboBox_currentIndexChanged(int index)
{
    emit languageChanged(index);
}

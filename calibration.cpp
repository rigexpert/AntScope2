#include "calibration.h"
#include "settings.h"
#include "popupindicator.h"
#include "mainwindow.h"

Calibration::Calibration(QObject *parent) : QObject(parent),
    m_state(CALIB_NONE),
    m_dotsCount(0),
    m_dotsNumber(500),
    m_onlyOneCalib(false),
    m_Z0(50),
    m_OSLCalibrationEnabled(false),
    m_OSLCalibrationPerformed(false),
    m_analyzer(NULL),
    m_settings(NULL)
{
    init();
}

void Calibration::init(const QString& _serial)
{
    QString serial = _serial;
    if (serial.isEmpty()) {
        serial = SelectionParameters::selected.serial;
    }
    setSerial(serial);
    if (serial.isEmpty()) {
        m_OSLCalibrationPerformed = false;
        return;
    }

    QString iniFilePath = Settings::localDataPath("AntScope2.ini");

    m_calibrationPath = Settings::localDataPath("Calibration");
    if (!serial.isEmpty()) {
        m_calibrationPath += "/" + serial;
    }

    QDir().mkdir(m_calibrationPath);
    m_settings = new QSettings(iniFilePath, QSettings::IniFormat);
    m_settings->beginGroup("Calibration");
    m_Z0 = m_settings->value("Z0", m_Z0).toDouble();
    setDotsNumber(m_settings->value("DotsNumber", 500).toInt());
    m_settings->endGroup();

    QDir dir = m_calibrationPath;
    m_openCalibFilePath = dir.absoluteFilePath("cal_open.s1p");
    m_shortCalibFilePath = dir.absoluteFilePath("cal_short.s1p");
    m_loadCalibFilePath = dir.absoluteFilePath("cal_load.s1p");
//    bool performed = QFile::exists(m_openCalibFilePath)
//        && QFile::exists(m_shortCalibFilePath)
//        && QFile::exists(m_loadCalibFilePath);

//    m_OSLCalibrationPerformed = performed;
    start(true);
    m_OSLCalibrationEnabled = false;
}

Calibration::~Calibration()
{
    m_settings->beginGroup("Calibration");

    m_settings->setValue("Z0", m_Z0);
    m_settings->setValue("Performed", m_OSLCalibrationPerformed);
    m_settings->setValue("Enabled", m_OSLCalibrationEnabled);
    m_settings->setValue("OpenPath", m_openCalibFilePath);
    m_settings->setValue("ShortPath", m_shortCalibFilePath);
    m_settings->setValue("LoadPath", m_loadCalibFilePath);
    m_settings->setValue("DotsNumber", dotsNumber());

    m_settings->endGroup();
    m_settings->sync();
    QString file = m_settings->fileName();
}

void Calibration::start(bool force)
{
    if (!isAnalyzerConnected()) // m_OSLCalibrationPerformed ?
        return;
    //QString notChoosed = tr("Not chosen");
    m_OSLCalibrationPerformed = false;
    if(force) // || m_OSLCalibrationPerformed)
    {
        if(!QFile::exists(m_openCalibFilePath) || !m_openData.loadData(m_openCalibFilePath,&m_Z0))
        {
            return;
        }
        if(!QFile::exists(m_shortCalibFilePath) || !m_shortData.loadData(m_shortCalibFilePath,&m_Z0))
        {
            return;
        }
        if(!QFile::exists(m_loadCalibFilePath) || !m_loadData.loadData(m_loadCalibFilePath,&m_Z0))
        {
            return;
        }
        m_OSLCalibrationPerformed = true;
    }
}

bool Calibration::getCalibrationPerformed(void)
{
    return m_OSLCalibrationPerformed;
}

bool Calibration::getCalibrationEnabled(void)
{
    return m_OSLCalibrationEnabled;
}

void Calibration::setAnalyzer(AnalyzerPro *analyzer)
{
    m_analyzer = analyzer;
}

QString Calibration::getCalibrationPath()
{
    return m_calibrationPath;
}

QString Calibration::getOpenFileName()
{
    QStringList list;
    list = m_openCalibFilePath.split("/");
    if(list.length() == 1)
    {
        list.clear();
        list = m_openCalibFilePath.split("\\");
    }
    return list.last();
}

QString Calibration::getShortFileName()
{
    QStringList list;
    list = m_shortCalibFilePath.split("/");
    if(list.length() == 1)
    {
        list.clear();
        list = m_shortCalibFilePath.split("\\");
    }
    return list.last();
}
QString Calibration::getLoadFileName()
{
    QStringList list;
    list = m_loadCalibFilePath.split("/");
    if(list.length() == 1)
    {
        list.clear();
        list = m_loadCalibFilePath.split("\\");
    }
    return list.last();
}

void Calibration::on_newData(RawData _rawData)
{
    double R = _rawData.r;
    double X = _rawData.x;

    double Gre = (R*R-m_Z0*m_Z0+X*X)/((R+m_Z0)*(R+m_Z0)+X*X);
    double Gim = (2*m_Z0*X)/((R+m_Z0)*(R+m_Z0)+X*X);

    switch (m_state)
    {
    case CALIB_OPEN:
        m_openData.setData(_rawData.fq, Gre, Gim, R, X);
        break;
    case CALIB_SHORT:
        m_shortData.setData(_rawData.fq, Gre, Gim, R, X);
        break;
    case CALIB_LOAD:
        m_loadData.setData(_rawData.fq, Gre, Gim, R, X);
        break;
    default:
        break;
    }

    ++m_dotsCount;
    int percent = 100*m_dotsCount/dotsNumber();
    if(percent > 100)
    {
        percent = 100;
    }
    emit progress(m_state, percent);

    if(m_dotsCount > m_dotsNumber)
    {
        m_dotsCount = 0;
        PopUpIndicator::hideIndicator();

        emit setCalibrationMode(false);
        QDir dir = m_calibrationPath;
        switch (m_state)
        {
        case CALIB_OPEN:
            m_openData.saveData(dir.absoluteFilePath("cal_open.s1p"),m_Z0);
            m_openCalibFilePath = dir.absoluteFilePath("cal_open.s1p");
            if(!m_onlyOneCalib)
            {
                PopUpIndicator::hideIndicator();
                if (QMessageBox::information(NULL, tr("Short"),
                                     tr("Please connect SHORT standard and press OK.")) == QMessageBox::Ok) {
                    PopUpIndicator::showIndicator();
                    m_state = CALIB_SHORT;
                    on_startCalibration();
                }
            }else
            {
                cancel();            }
            break;
        case CALIB_SHORT:
            m_shortData.saveData(dir.absoluteFilePath("cal_short.s1p"),m_Z0);
            m_shortCalibFilePath = dir.absoluteFilePath("cal_short.s1p");
            if(!m_onlyOneCalib)
            {
                PopUpIndicator::hideIndicator();
                if (QMessageBox::information(NULL, tr("Load"),
                                     tr("Please connect LOAD standard and press OK.")) == QMessageBox::Ok) {
                    PopUpIndicator::showIndicator();
                    m_state = CALIB_LOAD;
                    on_startCalibration();
                }
            }else
            {
                cancel();
            }
            break;
        case CALIB_LOAD:
            m_loadData.saveData(dir.absoluteFilePath("cal_load.s1p"),m_Z0);
            m_loadCalibFilePath = dir.absoluteFilePath("cal_load.s1p");
            if(!m_onlyOneCalib)
            {
                m_OSLCalibrationPerformed = true;
                QMessageBox::information(NULL, tr("Finish"),
                             tr("Calibration finished!"));
            }
            cancel();
            break;
        default:
            break;
        }

    }
}

void Calibration::cancel()
{
    PopUpIndicator::hideIndicator();
    m_state = CALIB_NONE;
    m_onlyOneCalib = false;
    disconnect(m_analyzer,SIGNAL(newData(rawData)),
               this, SLOT(on_newData(rawData)));
}

void Calibration::clearCalibration(void)
{
    m_openData.clear();
    m_shortData.clear();
    m_loadData.clear();
}
/*
void Calibration::on_startCalibration()
{
    m_dotsCount = 0;
    if(m_state == CALIB_NONE)
    {
        clearCalibration();
        connect(m_analyzer,SIGNAL(newData(rawData)),
                this, SLOT(on_newData(rawData)));
    }
    m_state++;

    if(m_analyzer != NULL)
    {
        emit setCalibrationMode(true);
        m_analyzer->on_measureCalib(dotsNumber());
    }
}*/
void Calibration::on_startCalibration()
{
    m_dotsCount = 0;
    if((m_state <= CALIB_OPEN)||(m_state >=CALIB_NUM))
    {
        clearCalibration();
        connect(m_analyzer, &AnalyzerPro::newData, this, &Calibration::on_newData);
        m_state = CALIB_OPEN;
    }

    if(m_analyzer != NULL)
    {
        emit setCalibrationMode(true);
        m_analyzer->on_measureCalib(dotsNumber());
    }
}

void Calibration::on_startCalibrationOpen()
{
    m_state = CALIB_OPEN;
    m_dotsCount = 0;
    m_onlyOneCalib = true;
    m_openData.clear();
    PopUpIndicator::showIndicator();
    if(m_analyzer != NULL)
    {
        connect(m_analyzer, &AnalyzerPro::newData, this, &Calibration::on_newData);
        emit setCalibrationMode(true);
        m_analyzer->on_measureCalib(dotsNumber());
    }
}

void Calibration::on_startCalibrationShort()
{
    m_state = CALIB_SHORT;
    m_dotsCount = 0;
    m_onlyOneCalib = true;
    m_shortData.clear();
    if(m_analyzer != NULL)
    {
        connect(m_analyzer, &AnalyzerPro::newData, this, &Calibration::on_newData);
        emit setCalibrationMode(true);
        m_analyzer->on_measureCalib(dotsNumber());
    }
}

void Calibration::on_startCalibrationLoad()
{
    m_state = CALIB_LOAD;
    m_dotsCount = 0;
    m_onlyOneCalib = true;
    m_loadData.clear();
    if(m_analyzer != NULL)
    {
        connect(m_analyzer, &AnalyzerPro::newData, this, &Calibration::on_newData);
        emit setCalibrationMode(true);
        m_analyzer->on_measureCalib(dotsNumber());
    }
}

bool Calibration::interpolateS(double fq, double &reO, double &imO, double &reS, double &imS, double &reL, double &imL)
{
    if(m_openData.getSize() == 0)
    {
        return false;
    }
    if (m_openData.getSize() != m_shortData.getSize() || m_openData.getSize() != m_loadData.getSize())
    {
        return false;
    }

    double alf = 0;
    int i;
    for(i = 0; i < m_openData.getSize()-1; ++i)
    {
        double fq1 = m_openData.getFq(i);
        double fq2 = m_openData.getFq(i+1);
        if((fq >= fq1) && (fq <= fq2))
        {
            alf = (fq-fq1)/(fq2-fq1);
            break;
        }
        if (fq < fq1)
        {
            alf = 0;
            break;
        }
    }

    if(i >= m_openData.getSize()-2)
    {
        //return false;
        i--;
    }

    reO = m_openData.getRe(i)*(1-alf) + m_openData.getRe(i+1)*alf;
    imO = m_openData.getIm(i)*(1-alf) + m_openData.getIm(i+1)*alf;

    reS = m_shortData.getRe(i)*(1-alf) + m_shortData.getRe(i+1)*alf;
    imS = m_shortData.getIm(i)*(1-alf) + m_shortData.getIm(i+1)*alf;

    reL = m_loadData.getRe(i)*(1-alf) + m_loadData.getRe(i+1)*alf;
    imL = m_loadData.getIm(i)*(1-alf) + m_loadData.getIm(i+1)*alf;
    return true;
}

void Calibration::applyCalibration(double MMR, double MMI, // Measured
                      double MOR, double MOI, double MSR, double MSI, double MLR, double MLI, // Measured parameters of cal standards
                      double SOR, double SOI, double SSR, double SSI, double SLR, double SLI, // Actual parameters of cal standards
                      double& MAR, double& MAI) // Actual
{

        // Calculate coefficients

        double	K1R = MLR - MSR,
                K1I = MLI - MSI,
                K2R = MSR - MOR,
                K2I = MSI - MOI,
                K3R = MOR - MLR,
                K3I = MOI - MLI;

        double	K4R = K1R*(SLR*SSR-SLI*SSI) - K1I*(SLR*SSI+SLI*SSR),
                K4I = K1R*(SLR*SSI+SLI*SSR) + K1I*(SLR*SSR-SLI*SSI);

        double	K5R = K2R*(SOR*SSR-SOI*SSI) - K2I*(SOR*SSI+SOI*SSR),
                K5I = K2R*(SOR*SSI+SOI*SSR) + K2I*(SOR*SSR-SOI*SSI);

        double	K6R = K3R*(SLR*SOR-SLI*SOI) - K3I*(SLR*SOI+SLI*SOR),
                K6I = K3R*(SLR*SOI+SLI*SOR) + K3I*(SLR*SOR-SLI*SOI);

        double	K7R = SOR*K1R - SOI*K1I,
                K7I = SOR*K1I + SOI*K1R;

        double	K8R = SLR*K2R - SLI*K2I,
                K8I = SLR*K2I + SLI*K2R;

        double	K9R = SSR*K3R - SSI*K3I,
                K9I = SSR*K3I + SSI*K3R;

        double	DR = K4R + K5R + K6R,
                DI = K4I + K5I + K6I;

        double	AnumR = MOR*K7R - MOI*K7I + MLR*K8R - MLI*K8I + MSR*K9R - MSI*K9I,
                AnumI = MOR*K7I + MOI*K7R + MLR*K8I + MLI*K8R + MSR*K9I + MSI*K9R;

        double	BnumR = MOR*K4R - MOI*K4I + MLR*K5R - MLI*K5I + MSR*K6R - MSI*K6I,
                BnumI = MOR*K4I + MOI*K4R + MLR*K5I + MLI*K5R + MSR*K6I + MSI*K6R;

        double	CnumR = K7R + K8R + K9R,
                CnumI = K7I + K8I + K9I;

        double	AR = (AnumR*DR + AnumI*DI)/(DR*DR + DI*DI),
                AI = (AnumI*DR - AnumR*DI)/(DR*DR + DI*DI);

        double	BR = (BnumR*DR + BnumI*DI)/(DR*DR + DI*DI),
                BI = (BnumI*DR - BnumR*DI)/(DR*DR + DI*DI);

        double	CR = (CnumR*DR + CnumI*DI)/(DR*DR + DI*DI),
                CI = (CnumI*DR - CnumR*DI)/(DR*DR + DI*DI);

        double	MAnumR = MMR - BR,
                MAnumI = MMI - BI,
                MAdenR = AR + CI*MMI - CR*MMR,
                MAdenI = AI - CR*MMI - CI*MMR;

        MAR = (MAnumR*MAdenR + MAnumI*MAdenI)/(MAdenR*MAdenR + MAdenI*MAdenI);
        MAI = (MAnumI*MAdenR - MAnumR*MAdenI)/(MAdenR*MAdenR + MAdenI*MAdenI);
}

void Calibration::on_enableOSLCalibration(bool enabled)
{
    if(m_OSLCalibrationPerformed)
    {
        m_OSLCalibrationEnabled = enabled;
    }

}

void Calibration::on_crcError()
{
    if (m_state == CALIB_NONE)
        return;
    PopUpIndicator::hideIndicator();
    m_state = CALIB_NONE;
    m_onlyOneCalib = false;
    emit setCalibrationMode(false);
    if (m_analyzer != nullptr) {
        disconnect(m_analyzer,SIGNAL(newData(rawData)),
                   this, SLOT(on_newData(rawData)));
        m_analyzer->setIsMeasuring(false);
    }
    QMessageBox::critical(NULL, tr("CRC Error"),
                          tr("Analyzer error. \nIt is recommended to perform calibration with connection via USB"),
                          QMessageBox::Ok);
}


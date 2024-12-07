#ifndef ANALYZERPARAMETERS
#define ANALYZERPARAMETERS

#include <QVector>
#include <qcustomplot.h>
#include <float.h>
#include "devinfo/redeviceinfo.h"


#pragma warning (disable : 4068 )

//#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
//#pragma GCC diagnostic ignored "-Wtype-limits"
//#pragma GCC diagnostic pop

static double MAX_SWR = 10.0;
static double MIN_SWR = 1.0;
static int SWR_ZOOM_LIMIT = 4;

static double MIN_USER_RANGE = -2000;
static double MAX_USER_RANGE = 2000;

static int ABSOLUTE_MIN_FQ = 0;
static int ABSOLUTE_MAX_FQ = 10000000;
static int MAX_DOTS = 2000;

static double VALUE_LIMIT = 9999; // VALUE_LIMIT +- jVALUE_LIMIT


enum parse{
    WAIT_NO=0,
    WAIT_VER,
    WAIT_DATA,
    WAIT_SLOT_DATA,
    WAIT_ANALYZER_DATA,
    WAIT_SCREENSHOT_DATA,
    WAIT_ANALYZER_UPDATE,
    WAIT_FULLINFO,
    WAIT_USER_DATA,
    WAIT_S21_DATA,
    WAIT_CALFIVEKOHM_START,
    WAIT_CALFIVEKOHM,
    WAIT_MATCH_12,
    WAIT_MATCH_PROFILE_B16
};

struct RawData {
    double fq;
    double r;
    double x;
    void read (const QJsonObject &json)
    {
        fq = json["fq"].toDouble();
        r = json["r"].toDouble();
        x = json["x"].toDouble();
    }
    void write (QJsonObject &json) const
    {
        json["fq"] = fq;
        json["r"] = r;
        json["x"] = x;
    }
    QString toString() { return QString("%1, %2, %3").arg(fq,0,'f',4,QLatin1Char(' ')).arg(r,0,'f',4,QLatin1Char(' ')).arg(x,0,'f',4,QLatin1Char(' ')); }
};

struct S21Data {
    double fq;
    double s21;
    double stage;
    void read (const QJsonObject &json)
    {
        fq = json["fq"].toDouble();
        s21 = json["s21"].toDouble();
        stage = json["stage"].toDouble();
    }
    void write (QJsonObject &json) const
    {
        json["fq"] = fq;
        json["s21"] = s21;
        json["stage"] = stage;
    }
    QString toString() { return QString("%1, %2, %3").arg(fq,0,'f',4,QLatin1Char(' ')).arg(s21,0,'f',4,QLatin1Char(' ')).arg(stage,0,'f',4,QLatin1Char(' ')); }
};

struct GraphData {
    double FQ=DBL_MAX;
    double SWR=DBL_MAX;
    double RhoPhase=DBL_MAX;
    double RhoMod=DBL_MAX;
    double R=DBL_MAX;
    double X=DBL_MAX;
    double Z=DBL_MAX;
    double Rpar=DBL_MAX;
    double Xpar=DBL_MAX;
    double Zpar=DBL_MAX;
    double RL=DBL_MAX;
    double ptX=0; // smith
    double ptY=0; // smith
};


struct UserData {
    double fq;
    QVector<double> values;
};


struct measurement
{
    QString name;
    bool visible = true;
    qint64 qint64From;
    qint64 qint64To;
    qint64 qint64Dots;
    void set(qint64 _qint64From, qint64 _qint64To, qint64 _qint64Dots) {
        qint64From = _qint64From; qint64To = _qint64To; qint64Dots =_qint64Dots;
    }

    QVector <S21Data> dataS21;
    QVector <RawData> dataRX;
    QVector <UserData> dataUser;
    QStringList fieldsUser;
//---------------------------------
    QCPDataMap swrGraph;
    QCPDataMap phaseGraph;
    QCPDataMap rhoGraph;
    QCPDataMap rsrGraph;
    QCPDataMap rsxGraph;
    QCPDataMap rszGraph;
    QCPDataMap rprGraph;
    QCPDataMap rpxGraph;
    QCPDataMap rpzGraph;
    QCPDataMap rlGraph;
    QCPDataMap s21Graph;
    QCPDataMap s21StageGraph;
    QCPDataMap tdrImpGraph;
    QCPDataMap tdrStepGraph;
    QCPDataMap tdrZGraph;
    QCPDataMap tdrImpGraphFeet;
    QCPDataMap tdrStepGraphFeet;
    QCPDataMap tdrZGraphFeet;
    QVector<QCPDataMap*> userGraphs;

    QCPCurve *smithCurve;
    QCPCurveDataMap smithGraph;
    QCPCurveDataMap smithGraphView;
//---------------------------------
//---------------------------------
//---------------------------------
    QVector <RawData> dataRXCalib;
    QCPDataMap swrGraphCalib;
    QCPDataMap phaseGraphCalib;
    QCPDataMap rhoGraphCalib;
    QCPDataMap rsrGraphCalib;
    QCPDataMap rsxGraphCalib;
    QCPDataMap rszGraphCalib;
    QCPDataMap rprGraphCalib;
    QCPDataMap rpxGraphCalib;
    QCPDataMap rpzGraphCalib;
    QCPDataMap rlGraphCalib;
    QCPCurveDataMap smithGraphCalib;
    QCPCurveDataMap smithGraphViewCalib;
};

#ifdef NEW_ANALYZER
#define PREFIX_SERIAL_NUMBER_AA35	1350
#define PREFIX_SERIAL_NUMBER_AA35_ZOOM	1351
#define PREFIX_SERIAL_NUMBER_AA55	1550
#define PREFIX_SERIAL_NUMBER_AA55_ZOOM	1551
#define PREFIX_SERIAL_NUMBER_AA230_ZOOM	1232
#define PREFIX_SERIAL_NUMBER_AA650_ZOOM	1650
#define PREFIX_SERIAL_NUMBER_AA230_STICK 4230
#define PREFIX_SERIAL_NUMBER_AA2000	4002
#define PREFIX_SERIAL_NUMBER_AA3000	4003
#define PREFIX_SERIAL_NUMBER_STICK_PRO 4600
#define PREFIX_SERIAL_NUMBER_AA1500_ZOOM 1015
#define PREFIX_SERIAL_NUMBER_ZEROII	4001
#define PREFIX_SERIAL_NUMBER_TOUCH 4005
#define PREFIX_SERIAL_NUMBER_TOUCH_EINK 4004
#define PREFIX_SERIAL_NUMBER_STICK_XPRO 4999
#define PREFIX_SERIAL_NUMBER_STICK_500 4500
#define PREFIX_SERIAL_NUMBER_WILSON_PRO 1016
#define PREFIX_SERIAL_NUMBER_AA1500SE 4115
#define PREFIX_SERIAL_NUMBER_MACHII 1800


class AnalyzerParameters {
    QString m_name;
    QString m_alias;
    QString m_minFq;
    QString m_maxFq;
    int m_lcdHeight=0;
    int m_lcdWidth=0;
    int m_prefix=-1;
    int m_index=0;
    QString m_serial;
    static QList<AnalyzerParameters*> m_analyzers;
    static AnalyzerParameters* m_current;

public:
    AnalyzerParameters() {}
    AnalyzerParameters(int _index, QString _name, QString _minFq, QString _maxFq, int _height=0, int _width=0, int _prefix=0, QString _alias=QString()) {
        m_index=_index; m_name=_name; m_minFq=_minFq; m_maxFq=_maxFq; m_lcdHeight=_height, m_lcdWidth=_width; m_prefix=_prefix; m_alias=_alias;
    }
    QString name() { return m_name; }
    QString alias() { return m_alias; }
    QString minFq() { return m_minFq; }
    QString maxFq() { return m_maxFq; }
    int height() { return m_lcdHeight; }
    int width() { return m_lcdWidth; }
    int prefix() { return m_prefix; }
    bool isValid() { return (m_prefix!=-1); }
    int index() { return m_index; }
    QString serilal() { return m_serial; }
    void setSerial(QString _serial) { m_serial = _serial; }
    void setMaxFq(QString _fq) { m_maxFq = _fq; }

    static AnalyzerParameters* byPrefix(int _prefix) {
        foreach (AnalyzerParameters* par, m_analyzers) {
            if (par->prefix() == _prefix)
                return par;
        }
        return nullptr;
    }
    static AnalyzerParameters* byName(QString _name) {
        //{ HUCK
        if (_name.contains("AA-3000 ZOOM")) {
            foreach (AnalyzerParameters* par, m_analyzers) {
                if (par->name() == "AA-3000 ZOOM") {
                    return par;
                }
            }
        }
        if (_name.contains("AA-2000 ZOOM")) {
            foreach (AnalyzerParameters* par, m_analyzers) {
                if (par->name() == "AA-2000 ZOOM") {
                    return par;
                }
            }
        }
        //} HUCK
        foreach (AnalyzerParameters* par, m_analyzers) {
            if (!par->alias().isEmpty() &&_name.contains(par->alias()))
                return par;
        }
        foreach (AnalyzerParameters* par, m_analyzers) {
            if (par->name() == _name) {
                return par;
            }
        }
        foreach (AnalyzerParameters* par, m_analyzers) {
            if (_name.contains(par->name())){
                return par;
            }
        }
        return nullptr;
    }

    static AnalyzerParameters* byIndex(int idx) {
        if (idx < 0 || idx >= m_analyzers.size())
            return nullptr;
        return m_analyzers[idx];
    }
    static AnalyzerParameters* byVER(QString ver) {
        foreach (AnalyzerParameters* par, m_analyzers) {
            if (ver.contains(par->name()))
                return par;
        }
        return nullptr;
    }

    static void normalizeFq(double& _from, double& _to) {
        double from = _from;
        double to = _to;
       if (to < from) {
           double tmp = from;
           from = to;
           to = tmp;
       }
       if (AnalyzerParameters::current() != nullptr) {
           double _minfq = (double)AnalyzerParameters::getMinFq().toLongLong();
           double _maxfq = (double)AnalyzerParameters::getMaxFq().toLongLong();
           if (from < _minfq)
               from = _minfq;
           if (to > _maxfq)
               to = _maxfq;
       }
       _from = from;
       _to = to;
    }

    static void normalizeFqRange(double& _center, double& _range) {
        double from = _center - _range;
        double to = _center + _range;
       if (to < from) {
           double tmp = from;
           from = to;
           to = tmp;
       }
       if (AnalyzerParameters::current() != nullptr) {
           double _minfq = (double)AnalyzerParameters::getMinFq().toLongLong();
           double _maxfq = (double)AnalyzerParameters::getMaxFq().toLongLong();
           if (from < _minfq)
               from = _minfq;
           if (to > _maxfq)
               to = _maxfq;
       }
       _range = (to - from) / 2;;
       _center = from + _range;
    }

    static bool supported(const QString& _name) {
       return byName(_name) != nullptr;
    }

    static int prefixFromSerial(QString serial) { return ( serial.length()==9 ? serial.remove(4,5).toInt() : 0); }
    static QString getName() { return m_current==nullptr ? QString() : m_current->name(); }
    static QString getSerial() { return m_current==nullptr ? QString() : m_current->serilal(); }
    static QString getMinFq() { return m_current==nullptr ? QString() : m_current->minFq(); }
    static QString getMaxFq() { return m_current==nullptr ? QString() : m_current->maxFq(); }
    static int getHeight() { return m_current==nullptr ? 0 : m_current->height(); }
    static int getWidth()  { return m_current==nullptr ? 0 : m_current->width(); }
    static AnalyzerParameters* current() { return m_current; }
    static void setCurrent(AnalyzerParameters* _cur) { m_current = _cur; }
    static void fill() {
        int idx=0;
        m_analyzers << new AnalyzerParameters(idx++, "NONE", "NONE", "NONE", 0, 0);
        m_analyzers << new AnalyzerParameters(idx++, "COMPORT", "60", "30000", 0, 0);
        m_analyzers << new AnalyzerParameters(idx++, "AA-3000 ZOOM", "100", "3000000", 480, 746, PREFIX_SERIAL_NUMBER_AA3000);
        m_analyzers << new AnalyzerParameters(idx++, "AA-30 ZERO", "60", "30000", 0, 0);
        m_analyzers << new AnalyzerParameters(idx++, "AA-30.ZERO", "60", "170000", 0, 0);
        m_analyzers << new AnalyzerParameters(idx++, "AA-30", "100", "30000", 64, 133);
        m_analyzers << new AnalyzerParameters(idx++, "AA-35 ZOOM", "60", "35000" , 240, 320, PREFIX_SERIAL_NUMBER_AA35);
        m_analyzers << new AnalyzerParameters(idx++, "AA-35 ZOOM", "60", "35000" , 240, 320, PREFIX_SERIAL_NUMBER_AA35_ZOOM);
        m_analyzers << new AnalyzerParameters(idx++, "AA-54", "100", "54000", 64, 133);
        m_analyzers << new AnalyzerParameters(idx++, "AA-55 ZOOM", "60", "55000", 240, 320, PREFIX_SERIAL_NUMBER_AA55);
        m_analyzers << new AnalyzerParameters(idx++, "AA-55 ZOOM", "60", "55000", 240, 320, PREFIX_SERIAL_NUMBER_AA55_ZOOM);
        m_analyzers << new AnalyzerParameters(idx++, "AA-170", "100", "170000", 64, 133);
        m_analyzers << new AnalyzerParameters(idx++, "AA-200", "100", "200000", 0, 0);
        m_analyzers << new AnalyzerParameters(idx++, "AA-2000 ZOOM", "100", "2000000", 480, 746, PREFIX_SERIAL_NUMBER_AA2000);
        m_analyzers << new AnalyzerParameters(idx++, "AA-230PRO", "100", "230000", 0, 0);
        m_analyzers << new AnalyzerParameters(idx++, "AA-230 ZOOM", "100", "230000", 220, 290, PREFIX_SERIAL_NUMBER_AA230_ZOOM);
        m_analyzers << new AnalyzerParameters(idx++, "AA-230", "100", "230000", 0, 0);
        m_analyzers << new AnalyzerParameters(idx++, "Stick 230", "100", "230000", 200, 200, PREFIX_SERIAL_NUMBER_AA230_STICK);
        m_analyzers << new AnalyzerParameters(idx++, "Stick Pro", "100", "600000", 220, 220, PREFIX_SERIAL_NUMBER_STICK_PRO);
        m_analyzers << new AnalyzerParameters(idx++, "AA-500", "100", "500000", 0, 0);
        m_analyzers << new AnalyzerParameters(idx++, "AA-520", "100", "520000", 0, 0);
        m_analyzers << new AnalyzerParameters(idx++, "AA-600", "100", "600000", 240, 320);
        m_analyzers << new AnalyzerParameters(idx++, "AA-650 ZOOM", "100", "650000", 240, 320, PREFIX_SERIAL_NUMBER_AA650_ZOOM);
        m_analyzers << new AnalyzerParameters(idx++, "AA-700 ZOOM", "100", "700000", 0, 0);
        m_analyzers << new AnalyzerParameters(idx++, "AA-1000", "100", "1000000", 240, 320);
        m_analyzers << new AnalyzerParameters(idx++, "AA-1400", "100", "1400000", 240, 320);
        m_analyzers << new AnalyzerParameters(idx++, "AA-1500 ZOOM", "100", "1500000", 240, 320, PREFIX_SERIAL_NUMBER_AA1500_ZOOM);
        m_analyzers << new AnalyzerParameters(idx++, "NanoVNA", "100", "1000000", 0, 0);

        // 08.06.2022 - not supported except of Android Antscope
        // 13.09.2024 - supported for all platforms
        m_analyzers << new AnalyzerParameters(idx++, "Zero II", "100", "1000000", 0, 0, PREFIX_SERIAL_NUMBER_ZEROII);
        //
        m_analyzers << new AnalyzerParameters(idx++, "Touch", "100", "1000000", 0, 0, PREFIX_SERIAL_NUMBER_TOUCH);
        m_analyzers << new AnalyzerParameters(idx++, "Touch E-Ink", "100", "1000000", 0, 0, PREFIX_SERIAL_NUMBER_TOUCH_EINK);

        m_analyzers << new AnalyzerParameters(idx++, "Stick XPro", "100", "1000000", 220, 220, PREFIX_SERIAL_NUMBER_STICK_XPRO);
        m_analyzers << new AnalyzerParameters(idx++, "Stick 500", "100", "500000", 200, 200, PREFIX_SERIAL_NUMBER_STICK_500);
        m_analyzers << new AnalyzerParameters(idx++, "WilsonPro CAA", "100", "1500000", 240, 320, PREFIX_SERIAL_NUMBER_WILSON_PRO);
        m_analyzers << new AnalyzerParameters(idx++, "AA-1500 ZOOM SE", "100", "1500000", 480, 746, PREFIX_SERIAL_NUMBER_AA1500SE, "AA-1500SE");
        m_analyzers << new AnalyzerParameters(idx++, "Match", "100", "690000", 480, 480, PREFIX_SERIAL_NUMBER_MACHII);
#ifdef _DEBUG
        test();
#endif
    }

    static QList<AnalyzerParameters*>& analyzers() { return m_analyzers; }
    static bool test()
    {
        qDebug() << "AnalyzerParameters::test() START";
        bool res = true;
        foreach (AnalyzerParameters* par, m_analyzers) {
           AnalyzerParameters* found = byName(par->name());
           if (found != nullptr && found->name() == par->name()){
               continue;
           } else {
               res = false;
               qDebug() << "AnalyzerParameters::test() FAILED: name=" << par->name() << "found=" << found->name();
           }
        }
        qDebug() << "AnalyzerParameters::test() FINISH " << (res?"SUCCESS":"FAILED");
        return res;
    }
};

struct SelectionParameters
{
    QString name;
    int modelIndex=-1;
    ReDeviceInfo::InterfaceType type=ReDeviceInfo::WRONG;
    QString id; // COM: port, HID: serial, BLE: address

    static SelectionParameters selected;

    bool valid() { return (type != ReDeviceInfo::WRONG); }
};

#endif



#endif // ANALYZERPARAMETERS

#ifndef ANALYZERPARAMETERS
#define ANALYZERPARAMETERS

#include <QVector>
#include <qcustomplot.h>
#include <float.h>

//#define SETTINGS_PATH "AntScope2.ini"


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
/*
enum {
    AANONE = 0,
    AA30,
    AA30ZERO,
    AA30_ZERO,
    AA35ZOOM,
    AA54,
    AA55ZOOM,
    AA170,
    AA200,
    AA230,
    AA230PRO,
    AA230ZOOM,
    AA230STICK,
    AA500,
    AA520,
    AA600,
    AA650,
    AA700ZOOM,
    AA1000,
    AA1400,
    AA2000,
    QUANTITY
};
*/

// !!! change it if add new model

#define QUANTITY 24

// !!!


static QString names[QUANTITY]={
    "NONE",
    "AA-30",
    "AA-30 ZERO",
    "AA-30.ZERO",
    "AA-35 ZOOM",
    "AA-54",
    "AA-55 ZOOM",
    "AA-170",
    "AA-200",
    "AA-230",
    "AA-230PRO",
    "AA-230 ZOOM",
    "Stick 230",
    "StickPro",
    "AA-500",
    "AA-520",
    "AA-600",
    "AA-650 ZOOM",
    "AA-700 ZOOM",
    "AA-1000",
    "AA-1400",
    "AA-1500 ZOOM",
    "AA-2000",
    "NanoVNA"
};
static QString minFq[QUANTITY]={//in kHz
    "NONE",
    "100",  //AA-30
    "60",  //AA-30 ZERO
    "60",  //AA-30.ZERO
    "60",   //AA-35ZOOM
    "100",  //AA-54
    "60",   //AA-55ZOOM
    "100",  //AA-170
    "100",  //AA-200
    "100",  //AA-230
    "100",  //AA-230PRO
    "100",  //AA-230ZOOM
    "100",  //STICK 230
    "100",  //STICK PRO
    "100",  //AA-500
    "100",  //AA-520
    "100",  //AA-600
    "100",  //AA-650ZOOM
    "100",  //AA-700ZOOM
    "100",  //AA-1000
    "100",  //AA-1400
    "100",  //AA-1500
    "0",   //AA-2000
    "1"    //NanoVNA
};
static QString maxFq[QUANTITY]={//in kHz
    "NONE",
    "30000",    //AA-30
    "30000",    //AA-30 ZERO
    "170000",   //AA-30.ZERO
    "35000",    //AA-35ZOOM
    "54000",    //AA-54
    "55000",    //AA-55ZOOM
    "170000",   //AA-170
    "200000",   //AA-200
    "230000",   //AA-230
    "230000",   //AA-230PRO
    "230000",   //AA-230ZOOM
    "230000",   //STICK 230
                                //"670000",   //STICK PRO
                                "600000",   //STICK PRO
    "500000",   //AA-500
    "520000",   //AA-520
    "600000",   //AA-600
    "650000",   //AA-650ZOOM
    "700000",   //AA-700ZOOM
    "1000000",  //AA-1000
    "1400000",  //AA-1400
    "1500000",  //AA-1500
    "3500000",   //AA-2000
    "1000000"   //NanoVNA
};

static int lcdHeight[QUANTITY]={//in kHz
    0,
    64,    //AA-30
    0,  //AA-30 ZERO
    0,  //AA-30.ZERO
    240,    //AA-35ZOOM
    64,    //AA-54
    240,    //AA-55ZOOM
    64,   //AA-170
    0,   //AA-200
    0,   //AA-230
    0,   //AA-230PRO
    220,   //AA-230ZOOM
    200,   //STICK 230
    220,   //STICK PRO
    0,   //AA-500
    0,   //AA-520
    240,   //AA-600
    240,   //AA-650ZOOM
    0,   //AA-700ZOOM
    240,  //AA-1000
    240,   //AA-1400
    240,   //AA-1500
    480,   //AA-2000
    0     // NanoVNA
};

static int lcdWidth[QUANTITY]={//in kHz
    0,
    133,    //AA-30
    0,  //AA-30 ZERO
    0,  //AA-30.ZERO
    320,    //AA-35ZOOM
    133,    //AA-54
    320,    //AA-55ZOOM
    133,   //AA-170
    0,   //AA-200
    0,   //AA-230
    0,   //AA-230PRO
    290,   //AA-230ZOOM
    200,   //STICK 230
    220,   //STICK PRO
    0,   //AA-500
    0,   //AA-520
    320,   //AA-600
    320,   //AA-650ZOOM
    0,   //AA-700ZOOM
    320,  //AA-1000
    320,   //AA-1400
    320,   //AA-1500
    800,   //AA-2000
    0   // NanoVNA
};

enum parse{
    WAIT_NO=0,
    WAIT_VER,
    WAIT_DATA,
    WAIT_ANALYZER_DATA,
    WAIT_SCREENSHOT_DATA,
    WAIT_ANALYZER_UPDATE,
    WAIT_FULLINFO,
    WAIT_USER_DATA,
    WAIT_LICENSE_LIST,
    WAIT_LICENSE_REQUEST,
    WAIT_LICENSE_APPLY1,
    WAIT_LICENSE_APPLY2,
    WAIT_CALFIVEKOHM_START,
    WAIT_CALFIVEKOHM
};

struct rawData{
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
};


struct UserData {
    double fq;
    QVector<double> values;
};


struct measurement{

    qint64 qint64Fq;
    qint64 qint64Sw;
    qint64 qint64Dots;
    void set(qint64 _qint64Fq, qint64 _qint64Sw, qint64 _qint64Dots) {
        qint64Fq = _qint64Fq; qint64Sw = _qint64Sw; qint64Dots =_qint64Dots;
    }

    QVector <rawData> dataRX;
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
    QCPDataMap tdrImpGraph;
    QCPDataMap tdrStepGraph;
    QCPDataMap tdrImpGraphFeet;
    QCPDataMap tdrStepGraphFeet;
    QVector<QCPDataMap*> userGraphs;

    QCPCurve *smithCurve;
    QCPCurveDataMap smithGraph;
    QCPCurveDataMap smithGraphView;
//---------------------------------
//---------------------------------
//---------------------------------
    QVector <rawData> dataRXCalib;
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

#endif // ANALYZERPARAMETERS

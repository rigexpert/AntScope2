#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QObject>
#include <analyzer/analyzerparameters.h>
#include <analyzer/analyzer.h>
#include <QSettings>
//#include <shlobj.h>

enum {CALIB_NONE = 0, CALIB_OPEN, CALIB_SHORT, CALIB_LOAD, CALIB_NUM};

class CalibData
{
public:
    CalibData()
    {
        //
    }
    ~CalibData()
    {
        //
    }
    void setData(double fq, double re, double im, double r, double x)
    {
        m_fq.append(fq);
        m_re.append(re);
        m_im.append(im);
        m_r.append(r);
        m_x.append(x);
    }
    double getFq(int number)
    {
        return m_fq.at(number);
    }
    double getRe(int number)
    {
        return m_re.at(number);
    }
    double getIm(int number)
    {
        return m_im.at(number);
    }
    double getR(int number)
    {
        return m_r.at(number);
    }
    double getX(int number)
    {
        return m_x.at(number);
    }
    int getSize()
    {
        return m_fq.length();
    }
    bool loadData(QString path, double* Z0)
    {
        clear();

        QString sPathName = path;

        if (sPathName.isEmpty())
        {
            return false;
        }

        QFile ifs(sPathName);

        if (!ifs.open(QFile::ReadWrite))
        {
            return false;
        }
        QTextStream in(&ifs);
        bool bGood = true;

        int iLines=0, iPoints=0;

        double  fqmul = 1000.0; // Default is GHz
        int iUnit = 1; // Default is S
        int iFormat = 1; // Default is MA

        QString line;// Whole string
        char strn[5][100]; // Substrings

        double f, param1, param2; // for reading S11 data lines


        do
        {
            line = in.readLine();
            iLines++;

            if ( (line.length() > 2) && (line[0] == '#')) // Option line
            {
                line.remove(0,1);
                bool bErr = false;

                int ns = sscanf(line.toLocal8Bit(), "%s %s %s %s %s", strn[0], strn[1], strn[2], strn[3], strn[4]);

                for (int i=0; i<ns; i++)
                {
                    // Frequency unit

                    if (!strcmp(strn[i], "GHz"))
                        fqmul = 1000.0;
                    else
                    if (!strcmp(strn[i], "MHz"))
                        fqmul = 1.0;
                    else
                    if (!strcmp(strn[i], "KHz"))
                        fqmul = 0.001;
                    else
                    if (!strcmp(strn[i], "Hz"))
                        fqmul = 0.000001;
                    else

                    // Parameter

                    if (!strcmp(strn[i], "S"))
                        iUnit = 1;
                    else
                    if (!strcmp(strn[i], "Z"))
                        iUnit = 2;
                    else

                    // Format

                    if (!strcmp(strn[i], "MA"))
                        iFormat = 1;
                    else
                    if (!strcmp(strn[i], "RI"))
                        iFormat = 2;
                    else

                    // R n

                    if (!strcmp(strn[i], "R"))
                    {
                        if ( i < (ns-1) )
                        {
                            i++;

                            *Z0 = atof(strn[i]);

                            if ( (*Z0<=0) || (*Z0>10000) )
                            {
                                bErr = true;
                                break;
                            }
                        }
                        else
                        {
                            bErr = true;
                            break;
                        }
                    }
                    else
                    {
                        bErr = true;
                        break;
                    }
                }

                // Check possible combinations

                if ( (iUnit == 1) && (iFormat == 1) ) // S, MA
                {
                }
                else
                if ( (iUnit == 1) && (iFormat == 2) ) // S, RI
                {
                }
                else
                if ( (iUnit == 2) && (iFormat == 2) ) // Z, RI
                {
                }
                else
                {
                    bErr = true;
                }

                if (bErr == true)
                {
                    return false;
                }

                continue;
            }

            if ( (strstr(line.toLocal8Bit(), "!") != NULL) || (strstr(line.toLocal8Bit(), ".") == NULL) ) // Comment or void line
                continue;

            // Scan data lines

            if ( sscanf(line.toLocal8Bit(), "%lf %lf %lf", &f, &param1, &param2) != 3)
            {
                return false;
            }

            double r = 0,x = 0;

            if ( (iUnit == 1) && (iFormat == 1) ) // S, MA
            {
                    // param1=|G|, param2=angle(deg)
                    double Gr = param1 * cos(param2/180.0*M_PI);
                    double Gi = param1 * sin(param2/180.0*M_PI);

                    r = (1-Gr*Gr-Gi*Gi)/((1-Gr)*(1-Gr)+Gi*Gi);
                    x = (2*Gi)/((1-Gr)*(1-Gr)+Gi*Gi);
            }
            else
            if ( (iUnit == 1) && (iFormat == 2) ) // S, RI
            {
                    // param1=Gr, param2=Gi
                    r = (1-param1*param1-param2*param2)/((1-param1)*(1-param1)+param2*param2);
                    x = (2*param2)/((1-param1)*(1-param1)+param2*param2);
            }
            else
            if ( (iUnit == 2) && (iFormat == 2) ) // Z, RI
            {
                    r = param1;
                    x = param2;
            }

            if ( qIsNaN(r) || (r<0) )
                r = 0;
            if ( qIsNaN(x) )
                x = 0;

            double Gre = (r*r-1+x*x)/((r+1)*(r+1)+x*x);
            double Gim = (2*x)/((r+1)*(r+1)+x*x);

            m_fq.append(f*fqmul);
            m_re.append(Gre);
            m_im.append(Gim);
            m_r.append(r*(*Z0));
            m_x.append(x*(*Z0));
            iPoints++;
        }while (!line.isNull());

        if (bGood && (iPoints>1) )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    bool saveData(QString name, double Z0)
    {
        QFile file(name);
        if(file.exists())
        {
            QFile::remove(name);
        }

        if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))//if (!file.open(QFile::ReadWrite))
        {
            return false;
        }

        QTextStream out(&file);

        out << "! Touchstone file generated by AntScope2";

        out << " (calibration data)\n";


        // S, RI
        out << "# MHz S RI R " << QString::number(Z0) << "\n";//Rswr <<
        out << "! Format: Frequency S-real S-imaginary (normalized to " << Z0
            << " Ohm)\n";

        // Data
        for (int i = 0; i < m_fq.length(); ++i)
        {
            QString s;

            s = QString("%1").arg(m_fq.at(i));		// Fq
            out << s << " ";

            // S, RI


            double Gre = m_re.at(i);
            double Gim = m_im.at(i);

            if (!qIsNaN(Gre))
                s = QString("%1").arg(Gre);		// Real
            else
                s = "0";
            out << s << " ";

            if (!qIsNaN(Gim))
                s = QString("%1").arg(Gim);		// Imaginary
            else
                s = "0";
            out << s << "\n";
        }

        out.flush();

        return true;
    }
    void clear()
    {
        m_fq.clear();
        m_re.clear();
        m_im.clear();
        m_r.clear();
        m_x.clear();
    }

private:
    QList<double> m_fq;
    QList<double> m_re;
    QList<double> m_im;
    QList<double> m_r;
    QList<double> m_x;
};

class Calibration : public QObject
{
    Q_OBJECT
public:
    explicit Calibration(QObject *parent = 0);
    ~Calibration();

    void start(void);
    bool getCalibrationPerformed(void);
    bool getCalibrationEnabled(void);
    void setAnalyzer(Analyzer *analyzer);//, Measurements *measurements);
    bool isCalibrationPerformed(){return m_OSLCalibrationPerformed;}
    bool interpolateS(double fq, double &reO, double &imO, double &reS, double &imS, double &reL, double &imL);
    void applyCalibration(double MMR, double MMI, // Measured
                          double MOR, double MOI, double MSR, double MSI, double MLR, double MLI, // Measured parameters of cal standards
                          double SOR, double SOI, double SSR, double SSI, double SLR, double SLI, // Actual parameters of cal standards
                          double& MAR, double& MAI);

    QString getOpenFileName();
    QString getShortFileName();
    QString getLoadFileName();

    double getZ0 () const {return m_Z0;}
    void setZ0 (double _Z0) {m_Z0 = _Z0;}
    int dotsNumber() { return ((m_dotsNumber < 0) ? 500 : m_dotsNumber); }
    void setDotsNumber(int _dots) { m_dotsNumber = (_dots > 2000) ? 2000 : _dots; }

private:
    CalibData m_openData;
    CalibData m_shortData;
    CalibData m_loadData;

    int m_state;
    int m_dotsCount;

    bool m_onlyOneCalib;

    double m_Z0;

    bool m_OSLCalibrationEnabled;
    bool m_OSLCalibrationPerformed;

    Analyzer *m_analyzer;
    QSettings * m_settings;

    QString m_openCalibFilePath;
    QString m_shortCalibFilePath;
    QString m_loadCalibFilePath;

    void clearCalibration(void);
    QString m_calibrationPath;
    int m_dotsNumber;


signals:
    void progress(int, int);
    void setCalibrationMode(bool);

public slots:
    void on_newData(rawData _rawData);
    void on_startCalibration();
    void on_startCalibrationOpen();
    void on_startCalibrationShort();
    void on_startCalibrationLoad();

    void on_openOpenFile(QString path);
    void on_shortOpenFile(QString path);
    void on_loadOpenFile(QString path);
    void on_enableOSLCalibration(bool enabled);

};


#endif // CALIBRATION_H

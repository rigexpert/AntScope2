#ifndef CUSTOMANALYZER_H
#define CUSTOMANALYZER_H

#include <QString>
#include <QMap>
#include <QSettings>

class CustomAnalyzer
{
    const int m_fields=6;

    QString m_prototype;
    QString m_alias;
    QString m_minFq;
    QString m_maxFq;
    int m_lcdHeight;
    int m_lcdWidth;

    static QMap<QString, CustomAnalyzer> m_map;
    static QString m_currentAlias;
    static bool m_useCustomized;

public:
    CustomAnalyzer();
    CustomAnalyzer(QString _csv);
    CustomAnalyzer& operator=(const CustomAnalyzer&);

    static void load(QSettings* settings=nullptr);
    static void save(QSettings* settings=nullptr);
    static CustomAnalyzer* get(QString& _alias);
    static CustomAnalyzer* getCurrent();
    static QString currentAlias() { return m_currentAlias; }
    static QString currentPrototype();
    static const QMap<QString, CustomAnalyzer>& getMap();
    static void setCurrent(QString _alias);
    static void add(CustomAnalyzer& ca);
    static void remove(QString _alias);
    static bool customized() { return m_useCustomized; }
    static void customize(bool _state) { m_useCustomized=_state; }

    void clear();
    bool isEmpty();

    void fromString(QString _csv);
    QString toString();
    void set(QString& _alias, QString& _name, QString& _minFq, QString& _maxFq, int _wd, int _ht);

    QString prototype() const { return m_prototype; }
    QString alias() const { return m_alias; }
    QString minFq() const { return m_minFq; }
    QString maxFq() const { return m_maxFq; }
    int width() const { return m_lcdWidth; }
    int height() const { return m_lcdHeight; }

    void setPrototype(QString _prototype) { m_prototype=_prototype.trimmed(); }
    void setAlias(QString _alias) { m_alias=_alias.trimmed(); }
    void setMinFq(QString _fq) { m_minFq=_fq.trimmed().replace(" ", ""); }
    void setMaxFq(QString _fq) { m_maxFq=_fq.trimmed().replace(" ", ""); }
    void setWidth(int _wd) { m_lcdWidth=_wd; }
    void setHeight(int _ht) { m_lcdHeight=_ht; }
};

#endif // CUSTOMANALYZER_H

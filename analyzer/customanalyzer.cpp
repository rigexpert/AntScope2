#include "customanalyzer.h"
#include "settings.h"
#include <QStringList>

QMap<QString, CustomAnalyzer> CustomAnalyzer::m_map;
QString CustomAnalyzer::m_currentAlias;
bool CustomAnalyzer::m_useCustomized = false;

CustomAnalyzer::CustomAnalyzer()
{
    clear();
}

CustomAnalyzer& CustomAnalyzer::operator=(const CustomAnalyzer& that)
{
    setAlias(that.alias());
    setPrototype(that.prototype());
    setMinFq(that.minFq());
    setMaxFq(that.maxFq());
    setWidth(that.width());
    setHeight(that.height());
    return *this;
}

CustomAnalyzer::CustomAnalyzer(QString _csv)
{
    fromString(_csv);
}

void CustomAnalyzer::load(QSettings* set)
{
    bool no_set = set == nullptr;
    if (no_set) {
        QString path = Settings::setIniFile();
        set = new QSettings(path, QSettings::IniFormat);
    }

    set->beginGroup("CustomAnalyzers");
    m_currentAlias = set->value("current_alias", "").toString();
    m_useCustomized = set->value("use_customized", false).toBool();

    QStringList keys = set->allKeys();
    for (int idx=0; idx<keys.size(); idx++) {
        QString key = keys[idx];
        if (key == "current_alias") {
            m_currentAlias = set->value(key, "").toString();
        } else if (key.indexOf("analyzer_") != -1) {
            QString csv = set->value(key, "").toString();
            if (!csv.isEmpty()) {
                CustomAnalyzer ca(csv);
                if (!ca.isEmpty())
                    m_map.insert(ca.alias(), ca);
            }
        }
    }
    set->endGroup();
    if (no_set)
        delete set;
}

void CustomAnalyzer::save(QSettings* set)
{
    bool no_set = set == nullptr;
    if (no_set) {
        QString path = Settings::setIniFile();
        set = new QSettings(path, QSettings::IniFormat);
    }

    set->remove("CustomAnalyzers");

    set->beginGroup("CustomAnalyzers");

    set->setValue("use_customized", m_useCustomized);
    set->setValue("current_alias", m_currentAlias);
    if (!m_map.isEmpty()) {
        QStringList keys = m_map.keys();
        for (int idx=0; idx<keys.size(); idx++) {
            QString key = keys[idx];
            QString csv = m_map[key].toString();
            if (!csv.isEmpty())
                set->setValue(QString("analyzer_%1").arg(idx), csv);
        }
    }
    set->endGroup();
    if (no_set)
        delete set;
}

void CustomAnalyzer::clear()
{
    m_prototype = "Custom";
    m_alias = "";
    m_minFq = "";
    m_maxFq = "";
    m_lcdWidth = 0;
    m_lcdHeight = 0;
}

bool CustomAnalyzer::isEmpty()
{
    return prototype().isEmpty() && alias().isEmpty() && minFq().isEmpty() && maxFq().isEmpty();
}

void CustomAnalyzer::fromString(QString _csv)
{
    QStringList list = _csv.split(",");
    if (list.size() != m_fields) {
        clear();
        return;
    }
    //m_alias = list[0];
    setAlias(list[0]);
    setPrototype(list[1]);
    setMinFq(list[2]);
    setMaxFq(list[3]);
    m_lcdWidth = list[4].toInt();
    m_lcdHeight = list[5].toInt();
}

QString CustomAnalyzer::toString()
{
    if (isEmpty())
        return QString();

    return QString("%1,%2,%3,%4,%5,%6")
            .arg(alias())
            .arg(prototype())
            .arg(minFq())
            .arg(maxFq())
            .arg(width())
            .arg(height());
}

void CustomAnalyzer::set(QString& _alias, QString& _prototype, QString& _minFq, QString& _maxFq, int _wd, int _ht)
{
    m_alias = _alias.trimmed();
    m_prototype = _prototype.trimmed();
    setMinFq(_minFq);
    setMaxFq(_maxFq);
    m_lcdWidth = _wd;
    m_lcdHeight = _ht;
}

const QMap<QString, CustomAnalyzer>& CustomAnalyzer::getMap()
{
    return m_map;
}


CustomAnalyzer* CustomAnalyzer::get(QString &_alias)
{
    if (m_map.contains(_alias))
        return &m_map[_alias];
    return nullptr;
}

CustomAnalyzer* CustomAnalyzer::getCurrent()
{
    if (m_map.contains(m_currentAlias))
        return &m_map[m_currentAlias];
    return nullptr;
}

void CustomAnalyzer::setCurrent(QString _alias)
{
    m_currentAlias = m_map.contains(_alias) ? _alias : QString();

}

void CustomAnalyzer::add(CustomAnalyzer& ca)
{
    m_map.insert(ca.alias(), ca);
}

void CustomAnalyzer::remove(QString _alias)
{
    if (m_map.contains(_alias))
        m_map.remove(_alias);
    m_currentAlias.clear();
}

QString CustomAnalyzer::currentPrototype()
{
    QString ret = "Custom";
    if (!customized() || m_currentAlias.isEmpty())
        return ret;
    CustomAnalyzer* ca = CustomAnalyzer::getCurrent();
    if (ca != nullptr)
        return ca->prototype();
    return ret;
}


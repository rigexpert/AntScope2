#ifndef PRESETS_H
#define PRESETS_H

#include <QObject>
#include <QTableWidget>
#include <QSettings>
#include <analyzer/analyzerparameters.h>
#include <settings.h>

class Presets : public QObject
{
    Q_OBJECT
public:
    explicit Presets(QObject *parent = 0);
    ~Presets();

    void setTable (QTableWidget * table);
    QStringList getRow(int row);
    void addNewRow (QString fqFrom, QString fqTo);
    void deleteRow (int number);
    void moveRowUp (int number);

private:
    QTableWidget * m_tableWidget;
    QStringList m_fqFromList;
    QStringList m_fqToList;

    QSettings *m_settings;

    bool m_isRange;

    void refresh();

signals:

public slots:
    void on_isRangeChanged (bool isRange);
};

#endif // PRESETS_H

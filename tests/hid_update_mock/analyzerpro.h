// Minimal stand-in for the real analyzer/analyzerpro.h, picked up instead of
// it via include-path ordering (this directory is searched before ../../src).
// hid_analyzer.cpp only ever qobject_casts its parent() to AnalyzerPro* and,
// if that succeeds, emits two signals on it - none of that is reachable in
// this test (HidAnalyzer is constructed with no parent, so the cast always
// yields nullptr), so this only needs to be enough for it to compile and
// link, not behave like the real class. Linking the real analyzerpro.h
// pulls in its entire slot surface (~20 methods) for a class this test
// never actually needs.
#ifndef ANALYZERPRO_H
#define ANALYZERPRO_H

#include <QObject>
#include <QString>
#include "baseanalyzer.h"

class AnalyzerPro : public QObject
{
    Q_OBJECT
public:
    explicit AnalyzerPro(QObject* parent = nullptr) : QObject(parent) {}

signals:
    void updateAutocalibrate5(int, QString);
    void stopAutocalibrate5();
};

#endif // ANALYZERPRO_H

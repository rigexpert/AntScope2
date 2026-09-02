// Minimal definitions for a few static members that HidAnalyzer's
// translation unit references transitively (through analyzerpro.h /
// analyzerparameters.h) but this test never actually exercises. Providing
// these directly avoids pulling in their real owners (selectdevicedialog.cpp,
// analyzerpro.cpp), which carry a large, unrelated UI/application dependency
// graph that has nothing to do with what's under test here.

#include "analyzer/analyzerparameters.h"

SelectionParameters SelectionParameters::selected;
QList<AnalyzerParameters*> AnalyzerParameters::m_analyzers;
AnalyzerParameters* AnalyzerParameters::m_current = nullptr;

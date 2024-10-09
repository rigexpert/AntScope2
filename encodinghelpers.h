#ifndef ENCODINGHELPERS_H
#define ENCODINGHELPERS_H

#include <QString>
#include <QByteArray>

class EncodingHelpers
{
public:
    EncodingHelpers();

    static QString A02_EnCode_strToBase16_v0(QString strIn);
};

#endif // ENCODINGHELPERS_H

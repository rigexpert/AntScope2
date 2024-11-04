#ifndef ENCODINGHELPERS_H
#define ENCODINGHELPERS_H

#include <QString>
#include <QByteArray>

class EncodingHelpers
{
public:
    EncodingHelpers();

    static QString encodeString(QString strIn);
    static QByteArray decodeString(QString strIn);
    static QByteArray sendToMatch(QString serialNumber);
    static QByteArray decodeString_nRaw1(QString inRaw);
};

#endif // ENCODINGHELPERS_H

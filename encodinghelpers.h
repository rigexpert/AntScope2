#ifndef ENCODINGHELPERS_H
#define ENCODINGHELPERS_H

#include <QString>
#include <QByteArray>

class EncodingHelpers
{
public:
    EncodingHelpers();

    static QString encodeString(QString strIn);
    static QString decodeString(QString strIn);
    static QByteArray sendToMatch(QString serialNumber);
};

#endif // ENCODINGHELPERS_H

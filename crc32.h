#ifndef CRC32_H
#define CRC32_H

#include <QtGlobal>
#include <QByteArray>

class CRC32
{
public:
    CRC32();
    ~CRC32();

    static quint32 crc(quint32 crc32, const QByteArray &data);
};

#endif // CRC32_H

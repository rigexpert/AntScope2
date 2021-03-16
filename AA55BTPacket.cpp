#include "AA55BTPacket.h"
#include "crc32.h"
#include <QDebug>
#include <QDataStream>
#include <QtMath>

quint16 AA55BTPacket::m_currentDot = 0;
qreal AA55BTPacket::m_fqStart = 0;
qreal AA55BTPacket::m_fqStep = 1;
bool AA55BTPacket::m_waitForLost = false;
int AA55BTPacket::m_skipAttempts = MAX_ATTEMPTS;
int AA55BTPacket::m_disconnectAttempts = MAX_ATTEMPTS;


AA55BTPacket::AA55BTPacket(QString str)
{
    bool ok;

    quint16 R = (str.mid(2, 2) + str.mid(0, 2)).toInt(&ok, 16);
    quint16 X = (str.mid(6, 2) + str.mid(4, 2)).toInt(&ok, 16);
    quint16 ID = (str.mid(10, 2) + str.mid(8, 2)).toInt(&ok, 16);
    quint8 CRC = str.mid(12, 2).toInt(&ok, 16);

    m_id = ID;

    //qDebug() << str << ID << R << X << QString::number(CRC, 16) << "STEP:" << QString::number(m_fqStep, 'f') << "CUR:" << m_currentDot;

    ble_RXPoint_t packet;
    packet.R = R;//(str.mid(2, 2) + str.mid(0, 2)).toInt(&ok, 16);
    packet.X = X;//(str.mid(6, 2) + str.mid(4, 2)).toInt(&ok, 16);
    packet.id = ID;//(str.mid(10, 2) + str.mid(8, 2)).toInt(&ok, 16);

    QByteArray buf;
    buf.resize(6);
    memcpy(buf.data(), &packet, buf.length());
    packet.crc = CRC32::crc8(buf);


//    packet.R = str.mid(0, 4).toInt(&ok, 16);
//    packet.X = str.mid(4, 4).toInt(&ok, 16);
//    packet.id = str.mid(8, 4).toInt(&ok, 16);
//    memcpy(buf.data(), &packet, buf.length());
//    packet.crc = CRC32::crc8(buf);

    m_valid = packet.crc == CRC;

    m_r = ble_uint16_to_float(R);
    m_x = ble_uint16_to_float(X);

    //qDebug() << m_valid << "ID:" << m_id << "R:" << m_r << "  X:" << m_x;
}

qreal AA55BTPacket::ble_uint16_to_float(quint16 src)
{
    int div_pow = ((src & (3 << 13)) >> 13);
    int sign = (src & (1 << 15));
    int value = (src & 0x1FFF);
    qreal result = value * 123.0 / qPow(10, div_pow + 2);
    if (sign != 0)
        result *= -1;
    return result;
}


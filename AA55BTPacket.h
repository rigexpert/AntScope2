#ifndef AA55BTPACKET_H
#define AA55BTPACKET_H

#include <QObject>
#include <QVector>

#define MAX_ATTEMPTS 5

class AA55BTPacket
{
    typedef struct
    {
        quint16 R;
        quint16 X;
        quint16 id;
        quint8 crc;
    } ble_RXPoint_t;


    quint16 m_id=0;
    qreal m_r=0;
    qreal m_x=0;
    quint8 m_crc=0;
    bool m_valid=false;

    static quint16 m_currentDot;
    static qreal m_fqStart;
    static qreal m_fqStep;
    static bool m_waitForLost;
    static int m_skipAttempts;
    static int m_disconnectAttempts;

public:
    AA55BTPacket(QString _str);

    quint16 id() { return m_id; }
    qreal fq() { return (m_fqStart + m_currentDot*m_fqStep); }
    qreal r() { return m_r; }
    qreal x() { return m_x; }
    bool valid() { return m_valid; }
    void setNext() { ++m_currentDot; }
    bool repeat() {
        if (!waitForLost())
                return false;
        --m_skipAttempts;
        if(m_skipAttempts==0) {
            m_skipAttempts = MAX_ATTEMPTS;
            return true;
        }
        return false;
    }
    bool disconnect() {
        --m_disconnectAttempts;
        return (m_disconnectAttempts <= 0);
    }

    static quint16 dot() { return m_currentDot; }
    static bool waitForLost() { return m_waitForLost; }
    static void setWait(bool _state) {
        m_waitForLost = _state;
        if (_state) {
            m_skipAttempts = MAX_ATTEMPTS;
            m_disconnectAttempts = MAX_ATTEMPTS;
        } else {
            m_skipAttempts=0;
        }
    }

    static void start(qreal _start, qreal _step) {
        m_fqStart = _start;
        m_fqStep = _step;
        m_currentDot = 0;
        m_waitForLost = false;
        m_skipAttempts = 0;
        m_disconnectAttempts = MAX_ATTEMPTS;
    }

protected:
    qreal ble_uint16_to_float(quint16 src);
};

#endif // AA55BTPACKET_H

#include "encodinghelpers.h"

EncodingHelpers::EncodingHelpers()
{

}

QString EncodingHelpers::encodeString(QString strIn)
{   // A02_EnCode_strToBase16_v0
    int ifor1,srtIn_len;
    QString str_Hi,str_Lo;
    quint8 tmpBt1,tmpBt_hi,tmpBt_lo;
    QString str1x_Hi,str1x_L0;
    quint8 CRC8;

    //-------Ord('A')  / Chr($64)---------
    //v--0
    str_Hi = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//37
    str_Lo = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//37

    QString Result;

    srtIn_len = strIn.length();;
    CRC8 = 0x01;
    for (ifor1=0; ifor1 < srtIn_len; ifor1++) { // 900 = 1800 div 2
        tmpBt1 = strIn[ifor1].toLatin1() ;
        CRC8 = CRC8 ^ tmpBt1;

        tmpBt_hi = ((tmpBt1 >> 4) & 0x0F);
        tmpBt_lo = ( tmpBt1 & 0x0F);

        str1x_Hi = str_Hi[tmpBt_hi];
        str1x_L0 = str_Lo[tmpBt_lo];
        Result = Result + str1x_Hi + str1x_L0;
    }
    //add crc byte ---------------------------------------
    tmpBt_hi = ((CRC8 >> 4) & 0x0F);
    tmpBt_lo = (CRC8 & 0x0F);
    //--------------------------
    str1x_Hi = str_Hi[tmpBt_hi];
    str1x_L0 = str_Lo[tmpBt_lo];
    Result = Result + str1x_Hi + str1x_L0;

    return Result;
}

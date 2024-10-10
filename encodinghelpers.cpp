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

    //-------Ord( "A ")  / Chr($64)---------
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

QString EncodingHelpers::decodeString(QString inRaw)
{
quint8 tmpBt1,tmpBt_hi,tmpBt_lo;
quint8 CRC_CALK;

int inLen;
int poz1,poz2,poz3;
QString str1,str2,str3;
QString chHi,chLo,str_Hi,str_Lo;
quint8 pozHi,pozLo;
quint32 Written;
quint8 SendBuff[65];

//-------Ord( "A ")  / Chr($64)---------
 QString Result =  "X1 ";
 inLen = inRaw.length();
// if((inLen<128) || (inLen>170))
 //if((inLen<128) || (inLen>256))
   //return Result;//-----------> ret X1

  poz1 = inRaw.indexOf("&nGet=");
  poz2 = inRaw.indexOf("&nRaw=");
  poz3 = inRaw.indexOf("&raw=");
  if(!((poz1>0)and(poz2>poz1)and(poz3>poz2)))
    return Result;//-----------> ret X1

  str1 = inRaw.mid((poz1 + QString( "&nGet=").length()), poz2 - (poz1 + QString( "&nGet= ").length()));
  str2 = inRaw.mid((poz2 + QString( "&nRaw=").length()), poz3 - (poz2 + QString( "&nRaw= ").length()));
  str3 = inRaw.mid((poz3 + QString( "&raw=").length()), inLen - (poz3 + QString( "&raw= ").length())+1);
  int len = str3.length();

   //Memo1.Lines.Add( "len: "+IntToStr(length(str3)));
 //-----------
 str_Hi =  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ ";//37
 str_Lo =  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ ";//37

 CRC_CALK = 0x02;
 for (int ifor1 = 0; ifor1 < 65; ifor1++) {
    if (ifor1*2 >= len)
        break; //???
//  chHi =  str3[(ifor1*2)+1];
//  chLo =  str3[(ifor1*2) +2];
    chHi =  str3[(ifor1*2)];
    chLo =  str3[(ifor1*2) +1];

  pozHi =   str_Hi.indexOf(chHi);//-1;
  pozLo =   str_Lo.indexOf(chLo);//-1;
  tmpBt1 = ((pozHi << 4) | pozLo) & 0x00FF;
  SendBuff[ifor1] = tmpBt1;
  CRC_CALK = CRC_CALK ^ tmpBt1;
 }
 // TODO
 // ???? str3 size
 QByteArray arr = QByteArray::fromRawData((const char*)SendBuff, 65);
 Result = arr;
 return Result;

 //----------------------
//  chHi =  str3[(64*2)+1];
//  chLo =  str3[(64*2) +2];
 chHi =  str3[(64*2)];
 chLo =  str3[(64*2) +1];

 pozHi = str_Hi.indexOf(chHi);//-1;
  pozLo = str_Lo.indexOf(chLo);//-1;
  tmpBt1 = ((pozHi << 4) | pozLo) & 0x00FF;

  if(tmpBt1 == CRC_CALK) {
    QByteArray arr = QByteArray::fromRawData((const char*)SendBuff, 65);
    Result = arr;
  }

//-------------------------------------------

//    Memo1.Lines.Add( "STR1: "+str1);
//    Memo1.Lines.Add( "STR2: "+str2);
//    Memo1.Lines.Add( "STR3: "+str3);
   return Result;
}

#include "encodinghelpers.h"

EncodingHelpers::EncodingHelpers()
{

}

QString EncodingHelpers::A02_EnCode_strToBase16_v0(QString strIn)
{
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
        //--------------------------
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
/*
QString TForm1.B02_www_Decode_rawMSG(QString inRaw)
{

quint8 tmpBt1,tmpBt_hi,tmpBt_lo;
quint8 CRC_CALK;

int inLen;
int poz1,poz2,poz3;
QString str1,str2,str3;
QString chHi,chLo,str_Hi,str_Lo;
quint8 pozHi,pozLo;
quint32 Written;
quint8 SendBuff[64];

//-------Ord('A')  / Chr($64)---------
 QString Result = "X1";
 inLen = inRaw.length();
 if((inLen<128)or(inLen>170)) {
   return Result;//-----------> ret X1

  poz1 = inRaw.indexOf( "&nGet=");
  poz2 = inRaw.indexOf( "&nRaw=");
  poz3 = inRaw.indexOf( "&raw=");
  if(!((poz1>0) && (poz2>poz1) && (poz3>poz2)))
   return Result;//-----------> ret X1

  str1 = copy(inRaw,(poz1+length("&nGet=")), poz2-(poz1+length("&nGet=")));
  str2 = copy(inRaw,(poz2+length("&nRaw=")), poz3-(poz2+length("&nRaw=")));
  str3 = copy(inRaw,(poz3+length("&raw=")), inLen-(poz3+length("&raw="))+1);
   Memo1.Lines.Add("len:"+IntToStr(length(str3)));
 //-----------
 str_Hi = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//37
 str_Lo = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//37

 CRC_CALK = $02;
 for ( int ifor1=0; ifor<63; ifor++) { // 900 =   1800 div 2
  chHi =  str3[(ifor1*2)+1];
  chLo =  str3[(ifor1*2) +2];

  pozHi =   Pos(chHi,str_Hi)-1;
  pozLo =   Pos(chLo,str_Lo)-1;
  tmpBt1 = ((pozHi shl 4) or pozLo)and $00FF;
  SendBuff[ifor1] = tmpBt1;
  CRC_CALK = CRC_CALK xor tmpBt1;

 }
//----------------------
  chHi =  str3[(64*2)+1];
  chLo =  str3[(64*2) +2];
  pozHi =   Pos(chHi,str_Hi)-1;
  pozLo =   Pos(chLo,str_Lo)-1;
  tmpBt1 = ((pozHi shl 4) or pozLo)and $00FF;

  if(tmpBt1=CRC_CALK)then begin

    if((COM_portopen))then begin   //xx1--------------
       WriteFile(ComHandle,SendBuff,64,Written,nil);
     end;
   Result = "ok";

//-------------------------------------------

    Memo1.Lines.Add("STR1:"+str1);
    Memo1.Lines.Add("STR2:"+str2);
    Memo1.Lines.Add("STR3:"+str3);

    return Result;
}
*/



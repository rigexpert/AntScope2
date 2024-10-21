#include "encodinghelpers.h"

EncodingHelpers::EncodingHelpers() {}

QString EncodingHelpers::encodeString(QString strIn) { // A02_EnCode_strToBase16_v0
  int ifor1, srtIn_len;
  QString str_Hi, str_Lo;
  quint8 tmpBt1, tmpBt_hi, tmpBt_lo;
  QString str1x_Hi, str1x_L0;
  quint8 CRC8;

  //-------Ord( "A ")  / Chr($64)---------
  // v--0
  str_Hi = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // 37
  str_Lo = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // 37

  QString Result;
  srtIn_len = strIn.length();

  CRC8 = 0x01;
  for (ifor1 = 0; ifor1 < srtIn_len; ifor1++) { // 900 = 1800 div 2
    tmpBt1 = strIn[ifor1].toLatin1();
    CRC8 = CRC8 ^ tmpBt1;

    tmpBt_hi = ((tmpBt1 >> 4) & 0x0F);
    tmpBt_lo = (tmpBt1 & 0x0F);

    str1x_Hi = str_Hi[tmpBt_hi];
    str1x_L0 = str_Lo[tmpBt_lo];
    Result = Result + str1x_Hi + str1x_L0;
  }
  // add crc byte ---------------------------------------
  tmpBt_hi = ((CRC8 >> 4) & 0x0F);
  tmpBt_lo = (CRC8 & 0x0F);
  //--------------------------
  str1x_Hi = str_Hi[tmpBt_hi];
  str1x_L0 = str_Lo[tmpBt_lo];
  Result = Result + str1x_Hi + str1x_L0;

  return Result;
}

QString EncodingHelpers::decodeString(QString inRaw) { // B02_www_Decode_rawMSG
  quint8 tmpBt1;
  quint8 CRC_CALK;

  int inLen;
  int poz1, poz2, poz3;
  QString str1, str2, str3;
  QString chHi, chLo, str_Hi, str_Lo;
  quint8 pozHi, pozLo;
  quint8 SendBuff[65];

  QString Result = "X1 ";
  inLen = inRaw.length();

  poz1 = inRaw.indexOf("&nGet=");
  poz2 = inRaw.indexOf("&nRaw=");
  poz3 = inRaw.indexOf("&raw=");
  if (!((poz1 > 0) and (poz2 > poz1) and (poz3 > poz2)))
    return Result; //-----------> ret X1

  str1 = inRaw.mid((poz1 + QString("&nGet=").length()),
                   poz2 - (poz1 + QString("&nGet= ").length()));
  str2 = inRaw.mid((poz2 + QString("&nRaw=").length()),
                   poz3 - (poz2 + QString("&nRaw= ").length()));
  str3 = inRaw.mid((poz3 + QString("&raw=").length()),
                   inLen - (poz3 + QString("&raw= ").length()) + 1);
  int len = str3.length();

  str_Hi = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ "; // 37
  str_Lo = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ "; // 37

  CRC_CALK = 0x02;
  for (int ifor1 = 0; ifor1 < 65; ifor1++) {
    if (ifor1 * 2 >= len)
      break; //???
    //  chHi =  str3[(ifor1*2)+1];
    //  chLo =  str3[(ifor1*2) +2];
    chHi = str3[(ifor1 * 2)];
    chLo = str3[(ifor1 * 2) + 1];

    pozHi = str_Hi.indexOf(chHi); //-1;
    pozLo = str_Lo.indexOf(chLo); //-1;
    tmpBt1 = ((pozHi << 4) | pozLo) & 0x00FF;
    SendBuff[ifor1] = tmpBt1;
    CRC_CALK = CRC_CALK ^ tmpBt1;
  }
  // TODO
  // ???? str3 size
  QByteArray arr = QByteArray::fromRawData((const char *)SendBuff, 65);
  Result = arr;
  return Result;

  //----------------------
  //  chHi =  str3[(64*2)+1];
  //  chLo =  str3[(64*2) +2];
  chHi = str3[(64 * 2)];
  chLo = str3[(64 * 2) + 1];

  pozHi = str_Hi.indexOf(chHi); //-1;
  pozLo = str_Lo.indexOf(chLo); //-1;
  tmpBt1 = ((pozHi << 4) | pozLo) & 0x00FF;

  if (tmpBt1 == CRC_CALK) {
    QByteArray arr = QByteArray::fromRawData((const char *)SendBuff, 65);
    Result = arr;
  }
  return Result;
}
#if 0
QByteArray EncodingHelpers::sendToMatch(QString serialNumber) {// B01_E1_Send_to_Match2
    quint8 CRC_CALK;
    quint8 SendBuff[64];
    quint8 byteChar, byteTmp;
    quint8 rdx, rdy;

    if (serialNumber.length() < 9) {
        return QByteArray();
    }
    for (int ifor1 = 4; ifor1 < 64; ifor1++)
        SendBuff[ifor1] = abs(rand() % 255); // Random(255);

    SendBuff[0] = 0x07;
    SendBuff[1] = 0x3E;
    SendBuff[2] = 0xBD;
    SendBuff[3] = 0xE1;
    //----------
    SendBuff[4] = 0x01;
    rdx = (SendBuff[15] & 0x0F);
    rdy = 16 + (SendBuff[9] & 0x07);

    for (int ifor1 = 0; ifor1 < 4; ifor1++) {
        byteChar = serialNumber[5 + ifor1].toLatin1();
        byteTmp = (SendBuff[10 + ifor1] & 0x3F);
        byteTmp = byteTmp + (ifor1 + 1) * rdx;
        byteTmp = byteTmp + byteChar;

        SendBuff[rdy + ifor1] = byteTmp;
    }
    CRC_CALK = 1; /// for blok 64 =1 //http=2  //data=0
    for (int ifor1 = 3; ifor1 < 63; ifor1++) {
        CRC_CALK = CRC_CALK ^ SendBuff[ifor1];
    }
    SendBuff[63] = CRC_CALK;
    return QByteArray((char *)SendBuff, sizeof(SendBuff));
}
#endif
QByteArray EncodingHelpers::sendToMatch(QString serialNumber) {// B01_E1_Send_to_Match2
  quint8 CRC_CALK;
  quint8 SendBuff[64];
  quint8 byteChar, byteTmp;
  quint8 rdx, rdy;

//  if (serialNumber.length() < 9) {
//    return QByteArray();
//  }
  for (int ifor1 = 4; ifor1 < 64; ifor1++)
    SendBuff[ifor1] = abs(rand() % 255); // Random(255);

  SendBuff[0] = 0x07;
  SendBuff[1] = 0x3E;
  SendBuff[2] = 0xBD;
  SendBuff[3] = 0xE1;
  //----------
  SendBuff[4] = 0x01;
  rdx = (SendBuff[15] & 0x0F);
  rdy = 16 + (SendBuff[9] & 0x07);

  for (int ifor1 = 0; ifor1 < 4; ifor1++) {
    byteChar = serialNumber[5 + ifor1].toLatin1();
    byteTmp = (SendBuff[10 + ifor1] & 0x3F);
    byteTmp = byteTmp + (ifor1 + 1) * rdx;
    byteTmp = byteTmp + byteChar;

    SendBuff[rdy + ifor1] = byteTmp;
  }
  CRC_CALK = 1; /// for blok 64 =1 //http=2  //data=0
  for (int ifor1 = 3; ifor1 < 63; ifor1++) {
    CRC_CALK = CRC_CALK ^ SendBuff[ifor1];
  }
  SendBuff[63] = CRC_CALK;
  return QByteArray((char *)SendBuff, sizeof(SendBuff));
}

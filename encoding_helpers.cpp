QString TForm1.A02_EnCode_bufToBase16_v0(QByteArray bufIn)
{
	int ifor1,srtIn_len:integer;
	QString str_Hi,str_Lo;
	quint8 tmpBt1,tmpBt_hi,tmpBt_lo;
	QString str1x_Hi,str1x_L0;
	quint8 CRC8;

//-------Ord('A')  / Chr($64)---------
//v--0
	str_Hi = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//37
	str_Lo = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//37

	QString Result;

	srtIn_len = 64;
	CRC8 = 0x01;
	for (ifor1=0; ifor1 < (srtIn_len-1); ifor1++) { // 64 => 128(data_16)+ 2(crc) -------
		tmpBt1:=bufIn[ifor1] ;
		CRC8 = CRC8 ^ tmpBt1;

		tmpBt_hi:=((tmpBt1 >> 4) & 0x0F);
		tmpBt_lo:=( tmpBt1 & 0x0F);
		//--------------------------
		str1x_Hi = str_Hi[tmpBt_hi+1];
		str1x_L0 = +str_Lo[tmpBt_lo+1];
		Result = Result + str1x_Hi + str1x_L0;
	}
	//add crc byte ---------------------------------------
	tmpBt_hi = ((CRC8 >> 4) & 0x0F);
	tmpBt_lo = (CRC8 & 0x0F);
	//--------------------------
	str1x_Hi = str_Hi[tmpBt_hi+1];
	str1x_L0 = str_Lo[tmpBt_lo+1];
	Result = Result + str1x_Hi + str1x_L0;

	return Result;
}


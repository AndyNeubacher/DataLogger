#ifndef __TOOLS_H__
#define __TOOLS_H__


#define  EEPROM_START_ADDRESS           0x00
#define  EEPROM_END_ADDRESS             0x03FF    // 1kb

#define  EEPROM_START_ERRORLOGGING      (EEPROM_END_ADDRESS - sizeof(WORD)*MAX_ERROR_LOGS - 1)  // 128*error + index
#define  EEPROM_START_SENSORCONFIG      (0x80)


#define  dec2bcd(dec)  ((((dec)/10)<<4)|((dec)%10))
#define  bcd2dec(bcd)  (((((bcd)>>4) & 0x0F) * 10) + ((bcd) & 0x0F))


#define  UNSIGNED_BYTE     0
#define  SIGNED_BYTE       1



void  CheckSystemAfterPowerLost(void);

void  SetDateTime(void);

void  DisplayErrorLogging(void);

void  ScanKeyPad(void);

BYTE* Date2Hex(s_time* time, BYTE* string_dest, BYTE offset);

BYTE* Time2Hex(s_time* time, BYTE* string_dest, BYTE offset);

CHAR* AddNewLine2Str(CHAR* ptr, BYTE nr);

void  DisplayRevisionString(void);

void  Sleep(WORD time);

BYTE* Ascii2Hex (BYTE *ascii_string, BYTE len);

BYTE  Ascii2Nibble(BYTE chr);

BYTE  Nibble2Ascii(BYTE n);

BYTE* Word2AsciiHex(WORD wrd, BYTE* buf);

BYTE* Long2AsciiHex(LONG wrd, BYTE* buf);

BYTE* Byte2AsciiDec(BYTE val, BYTE* buf, BYTE type);

BYTE* Word2AsciiDec(WORD val, BYTE* buf);

BYTE* Float2AsciiDec(FLOAT val, BYTE* ptr);

LONG  Bcd2Hex(BYTE bcd);

BYTE  StringCompare(BYTE* str1, BYTE* str2, BYTE len);



void  EEPromWriteByte(WORD Address, BYTE data);

void  EEPromWriteData(WORD Address, BYTE* Data, WORD len);

BYTE  EEPromReadByte(WORD Address);

void  EEPromReadData(WORD Address, BYTE *Data, WORD len);

void  SafeErrorsToEEPROM (WORD);

WORD  GetErrorsFromEEPROM (BYTE index);

WORD  SetDecimalValue(WORD min, WORD max, BYTE digits, BYTE pos_x, BYTE pos_y);

LONG  EncodeSystemTime(s_time *str_time);

void  DecodeSystemTime(LONG time, s_time *str_time);

void  ModifyTimestruct2BCD(s_time* str_time);

LONG  GetSecondsOfCodedTime(LONG cod_time);

BYTE* Seconds2TimeString(LONG val, BYTE* buf);

CHAR* getFlashStr(PGM_P flashStr) __ATTR_CONST__;

CHAR* calcPressure(WORD val, CHAR* ptr);


#endif

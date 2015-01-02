#include "main.h"
#include "tools.h"
#include "lcd.h"
#include "i2c.h"
#include "version.h"
#include "menu.h"
#include "usb.h"
#include "errorcodes.h"
#include "sensor.h"
#include "appl.h"



/////////////////////////////////////////////////////////////////////////
// function : complete system-reset should only happen after an        //
//            akku-change -> check RTC, ...   otherwise log errors     //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void CheckSystemAfterPowerLost()
{
   InitRTC();              // set/clear control-registers
   ReadRTC();              // get time from external RTC
   ReadOnboardTemp();      // read the onboard temperature sensor
   GetErrorsFromEEPROM(0); // readout "SystemError.len"


   if(IsBoxOpen())
   {
      InitLCD();
      BoxOpenedLogData2Stick();   // safe pending logged values to usb-stick
      SetDateTime();              // user input-request to correct the RTC

      if(IsBoxOpen())
        StartMenu();              // start user-interface (menu) only if box is already open
   }
   else
   {
      //-- reset caused by suspicious conditions -> do error-logging -//
      // safe MCRCSR register to EEPROM to safe what caused the error
      SafeErrorsToEEPROM((ERROR_SYSTEM | MCUCSR));
   }
}



/////////////////////////////////////////////////////////////////////////
// function : prompts user to correct the system-time -> write to RTC  //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void SetDateTime(void)
{
BYTE i;
BYTE pos = 1;
BYTE tmp[9];
BYTE max[2][9] = {{3,9,0,1,9,0,9,9},{2,9,0,5,9,0,5,9}};
BYTE update = FALSE;


   ReadRTC();         // get time from external RTC

   // display actual system-time and system-date
   ClearScreen();
   PrintLCD(1,1,STRING_DATE);
   PrintLCD(1,2,STRING_TIME);
   PrintLCD(9,1,(CHAR*)Date2Hex((s_time*)(&System.Time), &tmp[0], 0x30));  // display date
   PrintLCD(9,2,(CHAR*)Time2Hex((s_time*)(&System.Time), &tmp[0], 0x30));  // display time
   Cursor(CURSOR_ON, CURSOR_BLINK);
   MoveXY(9, 1);


   for(i=0;i<2;i++)
   {
      if(i==0)
         Date2Hex((s_time*)(&System.Time), &tmp[0], 0x00);    // copy actual date to tmp-buffer
      else
         Time2Hex((s_time*)(&System.Time), &tmp[0], 0x00);    // copy actual time to tmp-buffer


      // promt user for correcting the date or time
      do
      {
         while(!(System.EventID & (EVENT_KEY_CHANGED | EVENT_BOX_CLOSED | EVENT_BOX_OPENED)));

         // if box closed -> return to application without changes
         if(IsEventPending(EVENT_BOX_CLOSED | EVENT_BOX_OPENED))
            return;

         switch(System.Key.Valid)
         {
            case KEY_UP:      // increment value if within limit
                  if(tmp[pos-1] < max[i][pos-1])
                  {
                     tmp[pos-1]++;
                     update = TRUE;
                  }
               break;
            case KEY_DOWN:    // decrement value till zero
                  if(tmp[pos-1] > 0)
                  {
                     tmp[pos-1]--;
                     update = TRUE;
                  }
               break;
            case KEY_LEFT:    // move one digit to left
                  if(pos <= 1) pos = 8;
                  else pos--;
                  if((pos == 3) || (pos == 6)) pos--;
               break;
            case KEY_RIGHT:   // move one digit to right
                  if(pos >= 8) pos = 1;
                  else pos++;
                  if((pos == 3) || (pos == 6)) pos++;
               break;
         }

         if(update)     // only update digit if changed
         {
            write_char((tmp[pos-1]+0x30));   // write new value
            update = FALSE;
         }
         MoveXY((8+pos),(1+i));              // set cursorposition
         ClearEvent(EVENT_KEY_CHANGED);

      }while((System.Key.Valid != KEY_ESCAPE) && (System.Key.Valid != KEY_ENTER));


      // check if user wants to quit without saving
      if(System.Key.Valid == KEY_ESCAPE)
      {
         Cursor(CURSOR_OFF, CURSOR_STEADY);
         return;
      }


      if(i==0)
      {
         pos = 1;
         MoveXY((8+pos),2);              // set cursorposition when switched to time
         System.Time.day   = ((tmp[0]<<4) + tmp[1]);
         System.Time.month = ((tmp[3]<<4) + tmp[4]);  // copy back to date-structure
         System.Time.year  = ((tmp[6]<<4) + tmp[7]);
      }
      else
      {
         System.Time.hour  = ((tmp[0]<<4) + tmp[1]);
         System.Time.min   = ((tmp[3]<<4) + tmp[4]);  // copy back to time-structure
         System.Time.sec   = ((tmp[6]<<4) + tmp[7]);
      }
   }  // for


   WriteRTC();                            // write changed time to external RTC
   Cursor(CURSOR_OFF, CURSOR_STEADY);     // invisible cursor
}



/////////////////////////////////////////////////////////////////////////
// function : displays the logged errorcodes -> SystemError-struct     //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void DisplayErrorLogging(void)
{
   BYTE index = 0;
   BYTE buf[5];


   ClearScreen();                             // clear display
   PrintLCD(1,1,STRING_ERRORLOGGING_ERROR);   // display msg-error

   PrintLCD(7,1, "001 =");                    // display 1st entries
   PrintLCD(7,2, "002 =");
   PrintLCD(13,1, (CHAR*)Word2AsciiHex(GetErrorsFromEEPROM(0), &buf[0]));
   PrintLCD(13,2, (CHAR*)Word2AsciiHex(GetErrorsFromEEPROM(1), &buf[0]));

   do
   {
      while(!(System.EventID & (EVENT_KEY_CHANGED | EVENT_BOX_CLOSED | EVENT_BOX_OPENED)));

      // if box closed -> return to application without changes
      if(IsEventPending(EVENT_BOX_CLOSED | EVENT_BOX_OPENED))
         return;

      switch(System.Key.Valid)
      {
         case KEY_UP:      // show previous error
               if(index > 0)
                  index--;
            break;

         case KEY_DOWN:    // show next error
               if(index < (SystemError.len-2))
                  index++;
            break;
      }

      // show errorlogs
      PrintLCD(7,1, (CHAR*)Byte2AsciiDec(index+1, &buf[0], UNSIGNED_BYTE));
      PrintLCD(7,2, (CHAR*)Byte2AsciiDec(index+2, &buf[0], UNSIGNED_BYTE));
      PrintLCD(13,1, (CHAR*)Word2AsciiHex(GetErrorsFromEEPROM(index), &buf[0]));
      PrintLCD(13,2, (CHAR*)Word2AsciiHex(GetErrorsFromEEPROM(index+1), &buf[0]));

      ClearEvent(EVENT_KEY_CHANGED);
   }while(System.Key.Valid != KEY_ESCAPE);

}



/////////////////////////////////////////////////////////////////////////
// function : scans the keyboard every 1ms and debounce pressed keys   //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void ScanKeyPad(void)
{
   BYTE  inp_key;

   if(System.Flags.LcdUpdate)             // don't read keys if data is written to LCD
      return;

   if(IsEventPending(EVENT_KEY_CHANGED))  // don't read keys if key is still pending
      return;

   SetPortB_Input();
   inp_key = (PINB & 0x3F);               // read keys


   if(inp_key != System.Key.Detected)     // new key pressed ? (neg-edge detection)
   {
      if(inp_key != KEY_NO_KEY_PRESSED)
      {
         System.Key.Detected = inp_key;
         System.Key.DebounceTime = KEY_DEBOUNCE_TIME_MS;
      }
   }

   if(System.Key.DebounceTime > 0)        // debounce pressed key
   {
      System.Key.DebounceTime--;
      if(System.Key.DebounceTime == 0)    // debounce finished
      {
         if(System.Key.Detected == inp_key)  // valid key ?
            System.Key.Valid = System.Key.Detected;
         else
            System.Key.Detected = KEY_NO_KEY_PRESSED;
      }
   }

   // set event only when no key is pressed -> a pressed key modifies
   //  the display-data on PortB
   if((inp_key == KEY_NO_KEY_PRESSED) &&
      (System.Key.DebounceTime == 0) &&
      (System.Key.Detected == System.Key.Valid))
   {
      System.Key.Detected = KEY_NO_KEY_PRESSED;
      SetEvent(EVENT_KEY_CHANGED);
   }

}



/////////////////////////////////////////////////////////////////////////
// function : converts the system-date-structure to a string           //
// given    : pointer where string should be stored                    //
//            offset -> added to value (0x30 for Date2String-function) //
// return   : pointer where string is stored                           //
/////////////////////////////////////////////////////////////////////////
BYTE* Date2Hex(s_time* time, BYTE* string_dest, BYTE offset)
{
BYTE* tmp = string_dest;

  *tmp++ = (time->day >> 4) + offset;
  *tmp++ = (time->day & 0x0F) + offset;
  *tmp++ = '.';
  *tmp++ = (time->month >> 4) + offset;
  *tmp++ = (time->month & 0x0F) + offset;
  *tmp++ = '.';
  *tmp++ = (time->year >> 4) + offset;
  *tmp++ = (time->year & 0x0F) + offset;
  *tmp   = 0x00;

return string_dest;
}



/////////////////////////////////////////////////////////////////////////
// function : converts the system-time-structure to a string           //
// given    : pointer where string should be stored                    //
//            offset -> added to value (0x30 for Time2String-function) //
// return   : pointer where string is stored                           //
/////////////////////////////////////////////////////////////////////////
BYTE* Time2Hex(s_time* time, BYTE* string_dest, BYTE offset)
{
BYTE* tmp = string_dest;

  *tmp++ = (time->hour >> 4) + offset;
  *tmp++ = (time->hour & 0x0F) + offset;
  *tmp++ = ':';
  *tmp++ = (time->min >> 4) + offset;
  *tmp++ = (time->min & 0x0F) + offset;
  *tmp++ = ':';
  *tmp++ = (time->sec >> 4) + offset;
  *tmp++ = (time->sec & 0x0F) + offset;
  *tmp   = 0x00;

return string_dest;
}



/////////////////////////////////////////////////////////////////////////
// function : adds several Carriage Return <CR> to the given pointer   //
// given    : pointer where to add                                     //
//            number of <CR> to insert                                 //
// return   : pointer of 1st CR                                        //
/////////////////////////////////////////////////////////////////////////
CHAR* AddNewLine2Str(CHAR* ptr, BYTE nr)
{
  CHAR* tmp = ptr;

  while(nr--)
  {
    *ptr++ = 0x0D;
    *ptr++ = 0x0A;
  }

  *ptr = 0x00;
  return tmp;
}



/////////////////////////////////////////////////////////////////////////
// function : display revision and date of actual firmware             //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void DisplayRevisionString(void)
{
  PrintLCD(1,1, V_STRING_1);
  PrintLCD(1,2, V_STRING_2);
}



/////////////////////////////////////////////////////////////////////////
// function : sleep for given milliseconds                             //
// given    : milliseconds to wait                                     //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void Sleep(WORD time)
{
  ClearEvent(EVENT_1MS_TICK);
  while(time-- > 0)
  {
    while(!(System.EventID & (EVENT_1MS_TICK)));
    ClearEvent(EVENT_1MS_TICK);
  }
}



/////////////////////////////////////////////////////////////////////////
// function : converts an ASCII-string to HEX-values                   //
// given    : pointer of ASCII string                                  //
//            length of HEX-values
// return   : pointer of calculated HEX values                         //
/////////////////////////////////////////////////////////////////////////
BYTE* Ascii2Hex (BYTE *ascii_string, BYTE len)
{
  BYTE* input_ptr = ascii_string;
  BYTE* work_ptr = ascii_string;

  while(len--)
  {
    *ascii_string    = (Ascii2Nibble(*work_ptr++) << 4);
    *ascii_string++ |= (Ascii2Nibble(*work_ptr++));
  }

  *ascii_string = 0x00;
  return input_ptr;
}



/////////////////////////////////////////////////////////////////////////
// function : converts one ASCII-char to one HEX-nibble                //
// given    : ASCII character                                          //
// return   : value of ASCII-char in HEX                               //
/////////////////////////////////////////////////////////////////////////
BYTE Ascii2Nibble(BYTE chr)
{
  if((chr >= '0') && (chr <= '9'))
    return (chr - '0');

  if((chr >= 'A') && (chr <= 'F'))
    return (chr-'A'+10);

  return 0xFF;      // invalid
}



/////////////////////////////////////////////////////////////////////////
// function : converts one nibble to one ASCII-Hex-Char                //
// given    : nibble (4bits)                                           //
// return   : ASCII-character                                          //
/////////////////////////////////////////////////////////////////////////
BYTE Nibble2Ascii(BYTE n)
{
	n &= 0x0F;              // mask out upper 4 bits

  if(n<10)
		return(n+'0');        // return digits
	else
		return((n-10)+'A');   // return characters
}



/////////////////////////////////////////////////////////////////////////
// function : converts one WORD-value to 4BYTE-Hex-Text                //
// given    : WORD-value                                               //
//            pointer where Ascii-chars should be stored               //
// return   : pointer where Ascii-chars begin                          //
/////////////////////////////////////////////////////////////////////////
BYTE* Word2AsciiHex(WORD wrd, BYTE* buf)
{
  BYTE* ret = buf;

  *buf++ = Nibble2Ascii((BYTE)(wrd >> 12));
  *buf++ = Nibble2Ascii((BYTE)(wrd >> 8));
  *buf++ = Nibble2Ascii((BYTE)(wrd >> 4));
  *buf++ = Nibble2Ascii((BYTE)(wrd));
  *buf   = 0x00;

return ret;
}



/////////////////////////////////////////////////////////////////////////
// function : converts one LONG-value to 8BYTE-Hex-Text                //
// given    : LONG-value                                               //
//            pointer where Ascii-chars should be stored               //
// return   : pointer where Ascii-chars begin                          //
/////////////////////////////////////////////////////////////////////////
BYTE* Long2AsciiHex(LONG wrd, BYTE* buf)
{
  BYTE* ret = buf;

  *buf++ = Nibble2Ascii((BYTE)(wrd >> 28));
  *buf++ = Nibble2Ascii((BYTE)(wrd >> 24));
  *buf++ = Nibble2Ascii((BYTE)(wrd >> 20));
  *buf++ = Nibble2Ascii((BYTE)(wrd >> 16));
  *buf++ = Nibble2Ascii((BYTE)(wrd >> 12));
  *buf++ = Nibble2Ascii((BYTE)(wrd >> 8));
  *buf++ = Nibble2Ascii((BYTE)(wrd >> 4));
  *buf++ = Nibble2Ascii((BYTE)(wrd));
  *buf   = 0x00;

return ret;
}



/////////////////////////////////////////////////////////////////////////
// function : converts one BYTE-value to decimal-string                //
// given    : BYTE-value                                               //
//            pointer where Ascii-chars should be stored               //
// return   : pointer where Ascii-chars begin                          //
/////////////////////////////////////////////////////////////////////////
BYTE* Byte2AsciiDec(BYTE val, BYTE* buf, BYTE type)
{
   BYTE i;
   BYTE* ret = buf;

   if(type == SIGNED_BYTE)    // check if given byte is signed
   {
      if(val >= 0x80)         // is value negative
      {
         val = 0 - val;       // remove minus
         buf += 4;
      }
      else
      {
         type = UNSIGNED_BYTE;   // no '-' at beginning of value
         buf += 3;
      }
   }
   else
      buf   += 3;

   *buf-- = 0x00;             // mak end of buffer
   for(i=0;i<3;i++)
   {
      *buf-- = (val % 10) + '0';
      val  /= 10;
   }

   if(type == SIGNED_BYTE)    // add minus
      *buf = '-';

return ret;
}



/////////////////////////////////////////////////////////////////////////
// function : converts one WORD-value to decimal-string                //
// given    : WORD-value                                               //
//            pointer where Ascii-chars should be stored               //
// return   : pointer where Ascii-chars begin                          //
/////////////////////////////////////////////////////////////////////////
BYTE* Word2AsciiDec(WORD val, BYTE* buf)
{
   BYTE i;
   BYTE* ret = buf;

   buf += 5;
   *buf-- = 0x00;             // mak end of buffer
   for(i=0;i<5;i++)
   {
      *buf-- = (val % 10) + '0';
      val  /= 10;
   }


return ret;
}




/////////////////////////////////////////////////////////////////////////
// function : converts one FLOAT-value to decimal-string (XXXX.XX)     //
// given    : FLOAT-value                                              //
//            pointer where Ascii-chars should be stored               //
// return   : pointer where Ascii-chars begin                          //
/////////////////////////////////////////////////////////////////////////
BYTE* Float2AsciiDec(FLOAT val, BYTE* buf)
{
   BYTE  i;
   LONG  x;
   BYTE* ret = buf;

   x = (LONG)(val * 100);       // 2digits right of decimal point

   buf += 7;
   *buf-- = 0x00;               // mak end of buffer
   for(i=0;i<6;i++)
   {
      if(i==2)                  // add decimal-point
         *buf-- = ',';

      *buf-- = (x % 10) + '0';
      x  /= 10;
   }

   return ret;
}



/////////////////////////////////////////////////////////////////////////
// function : convert BCD-coded value to HEX-value                     //
// given    : BCD                                                      //
// return   : HEX-value                                                //
/////////////////////////////////////////////////////////////////////////
LONG Bcd2Hex(BYTE bcd)
{
   return (((bcd >> 4) * 10) + (bcd & 0x0F));
}



/////////////////////////////////////////////////////////////////////////
// function : compare to given strings                                 //
// given    : pointer of 1st and 2nd string                            //
//            length of string to compare                              //
// return   : TRUE if equal, FALSE if not equal                        //
/////////////////////////////////////////////////////////////////////////
BYTE StringCompare(BYTE* str1, BYTE* str2, BYTE len)
{
  while(len--)
  {
    if(*str1++ != *str2++)
      return FALSE;
  }
  return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : write byte to EEPROM                                     //
// given    : address where byte should be written                     //
//            value                                                    //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void EEPromWriteByte(WORD Address, BYTE data)
{
  EEARH = (Address >> 8);         // set dest-address
  EEARL = (Address & 0x00ff);

  if (EEARH > 0x03)               // is address within range of EEPROM-space (max 1k)
    return;

  EECR = 0x01;                    // enable EEPROM-read
  if (EEDR == data)               // is already stored data equal to given value ?
    return;                       // -> don't write to

  // write data to EEPROM
  GetMutex();
  EEDR = data;                    // data => EEDR
  EECR = 0x04;                    // set Master_Write_Enable
  EECR = 0x02;                    // set Write_Enable
  ReleaseMutex();
  while (EECR & 0x02);            // wait till byte-write finished
}



/////////////////////////////////////////////////////////////////////////
// function : write many bytes to internal EEPROM                      //
// given    : address where byte should be written                     //
//            pointer to start of bytes                                //
//            length of data                                           //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void EEPromWriteData(WORD Address, BYTE* Data, WORD len)
{
  while(len-- > 0)
  {
    EEPromWriteByte(Address, *Data);
    Address++;
    Data++;
  }
}



/////////////////////////////////////////////////////////////////////////
// function : read byte from given address                             //
// given    : address where byte should be read                        //
// return   : value of requested address                               //
/////////////////////////////////////////////////////////////////////////
BYTE EEPromReadByte(WORD Address)
{
  EEARH = (Address >> 8);         // set address
  EEARL = (Address & 0x00ff);

  if(EEARH > 3)                   // given address bigger than EEPROM
    return 0x00;

  EECR = 0x01;                    // start readcycle
  return EEDR;
}



/////////////////////////////////////////////////////////////////////////
// function : read many bytes of given address                         //
// given    : address where byte should be read                        //
//            pointer where data should be stored                      //
//            number of bytes to read                                  //
// return   : value of requested address                               //
/////////////////////////////////////////////////////////////////////////
void EEPromReadData(WORD Address, BYTE *Data, WORD len)
{
  while(len-- > 0)
  {
    *Data = EEPromReadByte(Address);
    Address++;
    Data++;
  }
}



/////////////////////////////////////////////////////////////////////////
// function : safe errorlogging to internal EEPROM                     //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void SafeErrorsToEEPROM (WORD error)
{
  union union_w_b tmp;

  if(SystemError.len < MAX_ERROR_LOGS)   // prevent overflow
  {
    tmp.w = error;

    EEPromWriteData(EEPROM_START_ERRORLOGGING + (2*SystemError.len) + 1, &tmp.b[0], sizeof(WORD));
    if(error != ERROR_NO_ERROR)
      SystemError.len++;
    EEPromWriteByte(EEPROM_START_ERRORLOGGING, SystemError.len);  // write index
  }
}



/////////////////////////////////////////////////////////////////////////
// function : get errorlogging-structure of internal EEPROM            //
// given    : index of safed error                                     //
// return   : errorcode (WORD)                                         //
/////////////////////////////////////////////////////////////////////////
WORD GetErrorsFromEEPROM (BYTE index)
{
  union union_w_b tmp;
  BYTE len;
  
  len = EEPromReadByte(EEPROM_START_ERRORLOGGING);                  // get length of errors
  if(len == 0xFF)
  {
    SystemError.len  = 0x00;                                        // fill error-struct with 0
    EEPromWriteByte(EEPROM_START_ERRORLOGGING, SystemError.len);    // write index
    EEPromWriteByte(EEPROM_START_ERRORLOGGING + 1, 0);              // clear
    EEPromWriteByte(EEPROM_START_ERRORLOGGING + 2, 0);              //  first
    EEPromWriteByte(EEPROM_START_ERRORLOGGING + 3, 0);              //   two
    EEPromWriteByte(EEPROM_START_ERRORLOGGING + 4, 0);              //    errors
    return 0x0000;
  }
  
  SystemError.len = len;
  EEPromReadData(EEPROM_START_ERRORLOGGING + 1 + (2*index), &tmp.b[0], 2);  // get errorcode

  return tmp.w;
}



/////////////////////////////////////////////////////////////////////////
// function : set decimalvalue of max 0...65535                        //
// given    : min ...... minimal value                                 //
//            max ...... maximum value                                 //
//            digits ... number of decimal-digits  0...6               //
//            pos_x .... position where input-box should be drawn      //
//            pox_y .... position where input-bus should be drawn      //
// return   : selected value                                           //
/////////////////////////////////////////////////////////////////////////
WORD SetDecimalValue(WORD min, WORD max, BYTE digits, BYTE pos_x, BYTE pos_y)
{
  BYTE  i;
  WORD  value = 0;
  WORD  tmp[5] = {0,0,0,0,0};
  BYTE  digit_sel = 0;
  BYTE  val_ascii[6] = {'0','0','0','0','0', 0x00};
  BYTE  error[6] = {'X','X','X','X','X', 0x00};

  do                                    // do till value is within ranges (min,max)
  {
    // print number of requested digits
    error[digits] = 0x00;
    val_ascii[digits] = 0x00;
    PrintLCD(pos_x, pos_y, (CHAR*)(&val_ascii[0]));
    MoveXY(pos_x, pos_y);
    Cursor(CURSOR_ON, CURSOR_BLINK);

    do
    {
      while(!(System.EventID & (EVENT_KEY_CHANGED | EVENT_BOX_CLOSED | EVENT_BOX_OPENED)));

      // if box closed -> return to application without changes
      if(IsEventPending(EVENT_BOX_CLOSED | EVENT_BOX_OPENED))
        return 0;

      switch(System.Key.Valid)
      {
        case KEY_UP:      // inc value
            if(val_ascii[digit_sel] < '9')
              val_ascii[digit_sel]++;
          break;

        case KEY_DOWN:    // dec value
            if(val_ascii[digit_sel] > '0')
              val_ascii[digit_sel]--;
          break;

        case KEY_RIGHT:   // select next digit
            if(++digit_sel >= digits)
              digit_sel = 0;
          break;

        case KEY_LEFT:    // select previous digit
            if(digit_sel == 0)
              digit_sel = digits-1;
            else
              digit_sel--;
          break;
      }
      // display new value
      PrintLCD(pos_x, pos_y, (CHAR*)(&val_ascii[0]));
      MoveXY(pos_x+digit_sel, pos_y);
      ClearEvent(EVENT_KEY_CHANGED);

    }while((System.Key.Valid != KEY_ESCAPE) && (System.Key.Valid != KEY_ENTER));

    Cursor(CURSOR_OFF, CURSOR_STEADY);        // hide cursor

    // check if user wants to quit without saving
    if(System.Key.Valid == KEY_ESCAPE)
       return 0;

    // copy to buffer of type "WORD"
    for(i=0;i<digits;i++)
    {
      if(val_ascii[i] != 0)
         tmp[5-digits+i] = (val_ascii[i]-'0');
    }

    //calculate value of full digits (left from dec-point)
    value = tmp[0]*10000 + tmp[1]*1000 + tmp[2]*100 + tmp[3]*10 + tmp[4];


    // display error-msg if value out of range
    if((value < min) || (value > max))
    {
      PrintLCD(pos_x, pos_y, (CHAR*)&error[0]);
      WaitEventTimeout(EVENT_KEY_CHANGED, 1000);
      ClearEvent(EVENT_KEY_CHANGED);
    }

  }while((value < min) || (value > max));   // loop until value is within ranges


  return value;
}



/////////////////////////////////////////////////////////////////////////
// function : code time-structure to 32bit value                       //
// given    : nothing                                                  //
// return   : LONG = coded systemtime                                  //
/////////////////////////////////////////////////////////////////////////
LONG EncodeSystemTime(s_time* str_time)
{
   LONG time;

   ReadRTC();
   time  = Bcd2Hex(str_time->year + 0x20) << 26;
   time |= Bcd2Hex(str_time->month) << 22;
   time |= Bcd2Hex(str_time->day) << 17;
   time |= Bcd2Hex(str_time->hour) << 12;
   time |= Bcd2Hex(str_time->min) << 6;
   time |= Bcd2Hex(str_time->sec);

return time;
}



/////////////////////////////////////////////////////////////////////////
// function : decode given 32bit value to given time-structure         //
// given    : LONG coded time                                          //
//            timestruct where decoded time is stored                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void DecodeSystemTime(LONG time, s_time* str_time)
{
   str_time->year  = ((time >> 26) & 0x7F) - 20;
   str_time->month = (time >> 22)  & 0x0F;
   str_time->day   = (time >> 17)  & 0x1F;
   str_time->hour  = (time >> 12)  & 0x1F;
   str_time->min   = (time >> 6)   & 0x3F;
   str_time->sec   = (time)        & 0x3F;
}



/////////////////////////////////////////////////////////////////////////
// function : modify values of timestruct to BCD-codes stimestruct     //
// given    : pinter to timestruct                                     //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void ModifyTimestruct2BCD(s_time* str_time)
{
   str_time->year  = dec2bcd(str_time->year);
   str_time->month = dec2bcd(str_time->month);
   str_time->day   = dec2bcd(str_time->day);
   str_time->hour  = dec2bcd(str_time->hour);
   str_time->min   = dec2bcd(str_time->min);
   str_time->sec   = dec2bcd(str_time->sec);
}



/////////////////////////////////////////////////////////////////////////
// function : decode given 32bit value and calculate nr of seconds     //
// given    : LONG coded time                                          //
// return   : LONG number of seconds                                   //
/////////////////////////////////////////////////////////////////////////
LONG GetSecondsOfCodedTime(LONG cod_time)
{
   LONG sec;
   s_time tmp_time;

   DecodeSystemTime(cod_time, &tmp_time);
   sec  = tmp_time.sec;
   sec += (tmp_time.min * 60);
   sec += (tmp_time.hour * 60 * 60);
   sec += (tmp_time.day * 24 * 60 * 60);

return sec;
}



/////////////////////////////////////////////////////////////////////////
// function : convert given seconds to timestring "xx:xx:xx"           //
// given    : WORD number of seconds                                   //
//            BYTE* pointer to buf where string should be written      //
// return   : LONG number of seconds                                   //
/////////////////////////////////////////////////////////////////////////
BYTE* Seconds2TimeString(LONG val, BYTE* buf)
{
  BYTE* ret_ptr = buf+1;

  Byte2AsciiDec(val/3600, buf, UNSIGNED_BYTE);    // write hours
  val = val%(60*60);                              // get nr_of_seconds without hours
  Byte2AsciiDec(val/60, buf+3, UNSIGNED_BYTE);    // write minutes
  val = val%60;                                   // get nr_of_seconds without hours+minutes
  Byte2AsciiDec(val, buf+6, UNSIGNED_BYTE);       // write minutes

  *(buf+3) = ':';       // insert time separators
  *(buf+6) = ':';

  return ret_ptr;
}



char FlashStrBuf [20];
/////////////////////////////////////////////////////////////////////////
// function : returns a ptr to a string stored in the FLASH memory     //
// given    : string to 'copy' to temporary buffer                     //
// return   : ptr to temporary string in RAM (copied from flash)       //
/////////////////////////////////////////////////////////////////////////
CHAR* getFlashStr(PGM_P flashStr)
{
   size_t len = strlen_P(flashStr);   
   
   if (len > sizeof(FlashStrBuf) - 1)
      len = sizeof(FlashStrBuf) - 1; 
   
   memcpy_P (FlashStrBuf, flashStr, len);
   FlashStrBuf[len] = 0;

   return FlashStrBuf;
}


/////////////////////////////////////////////////////////////////////////
// function : calculates the real pressure from the AD-Values          //
// given    : 16bit value from the AD-Converter                        //
// return   : ptr to readable string                                   //
/////////////////////////////////////////////////////////////////////////
CHAR* calcPressure(WORD val, CHAR* ptr)
{
   FLOAT pressure;
   
   pressure = (((float)val)/39.3216 - 5);      // -> ( (val/1024 * 5Volt)/240Ohm - 4mA) * 1000 * 20bar/16mA
   return (CHAR*)Float2AsciiDec(pressure, (BYTE*)ptr);
}   

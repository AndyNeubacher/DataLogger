#include "main.h"
#include "usb.h"
#include "init.h"
#include "errorcodes.h"
#include "string.h"
#include "i2c.h"
#include "tools.h"
#include "sensor.h"

CHAR buf[20];


/////////////////////////////////////////////////////////////////////////
// function : receive data from UART -> this function is called by     //
//             the RECEIVE-COMPLETE-INTERRUPT                          //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void ReceiveUSB_Byte(void)
{
  BYTE data;
  BYTE error;


  error = UCSRA;
  data  = UDR;

  if(error & 0x18)                          // check overrun or frameerror ?
    while(UCSRA & 0x80) error = UDR;        // flush databuffer by reading the UART register
  else
  {
    if(data == uALFAT_EOT)                  // end of message received
    {
      if(USB.Rx.len > 1)                    // only set event if msg-length bigger than 0
      {
        USB.Rx.data[USB.Rx.len++] = data;   // safe data
        USB.Rx.msg_count++;
        SetEvent(EVENT_USB_MSG);
      }
    }
    else
    {
      if(USB.Rx.len < MAX_USB_BUFFER_LEN)
        USB.Rx.data[USB.Rx.len++] = data;   // safe data
      else
        StopDebugger();
    }
  }
}



/////////////////////////////////////////////////////////////////////////
// function : transmit one byte via RS232 to the USB-device            //
// given    : databyte                                                 //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void TransmitUSB_Byte(CHAR data)
{
  while ((UCSRA & 0x20) == 0x00);
  UDR = data;                 // write data
}



/////////////////////////////////////////////////////////////////////////
// function : transmit string to USB                                   //
// given    : databyte                                                 //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void USB_TransmitString(CHAR* data)
{
  while(*data != 0)
    TransmitUSB_Byte(*data++);

  TransmitUSB_Byte(uALFAT_EOT);
}



/////////////////////////////////////////////////////////////////////////
// function : wait for a msg till received via serial/usb              //
// given    : timeout                                                  //
// return   : length of received message                               //
/////////////////////////////////////////////////////////////////////////
BYTE* USB_ReceiveString(WORD timeout)
{
  BYTE rx_cnt;

  // wait till the usb-message has finished
  if(WaitEventTimeout(EVENT_USB_MSG, timeout) == EVENT_RESULT_TIMEOUT)
    return NULL;

  ClearEvent(EVENT_USB_MSG);                // clear event
  MutexFunc(rx_cnt = USB.Rx.msg_count);     // get actual number of msg's

  // get pointer of received message
  return USB_FindStartOfRxMessage(rx_cnt);
}



/////////////////////////////////////////////////////////////////////////
// function : send 1msg and wait for 1answer                           //
// given    : timeout in ms                                            //
// return   : pointer to returned answer                               //
/////////////////////////////////////////////////////////////////////////
BYTE* USB_SendQuery(CHAR* string, WORD timeout)
{
  BYTE  rx_len;
  BYTE* last_ptr;


  MutexFunc(rx_len = USB.Rx.len);   // get rx-len with mutex

  // get the pointer where the new message will be written
  last_ptr = (BYTE*)(&USB.Rx.data[rx_len]);
  USB_TransmitString(string);       // send msg

  // wait for the usb-answer-message
  if(WaitEventTimeout(EVENT_USB_MSG, timeout) == EVENT_RESULT_TIMEOUT)
    return NULL;
  ClearEvent(EVENT_USB_MSG);        // clear event

  return last_ptr;
}



/////////////////////////////////////////////////////////////////////////
// function : search receive-buffer for start of message               //
// given    : number of requested message                              //
// return   : pointer of start-of-message                              //
/////////////////////////////////////////////////////////////////////////
BYTE* USB_FindStartOfRxMessage(BYTE index)
{
  BYTE  i;
  BYTE  found = 1;
  BYTE  rx_len;
  BYTE* msg_ptr = &USB.Rx.data[0];  // point to start of rx-buffer;

  if(index == 1)                    // if first selected msg
    return msg_ptr;

  MutexFunc(rx_len = USB.Rx.len);   // get rx-len with mutex

  for(i=0; i<rx_len; i++)           // search msg-buffer for "uALFAT_EOT"
  {
    if(*msg_ptr == uALFAT_EOT)      // found ?
    {
      if(++found == index)          // 1st uALFAT_EOT is the end of the previous msg
      {                             //  -> should return the beginning of the msg
        msg_ptr++;                  // inc ptr to beginning of msg
        return msg_ptr;             // return start-ptr of msg
      }
    }
    msg_ptr++;
  }

  return NULL;                      // no uALFAT_EOT found !
}



/////////////////////////////////////////////////////////////////////////
// function : flush received USB data                                  //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void USB_FlushData(void)
{
  GetMutex();              // disable global int

  USB.Rx.len = 0;
  USB.Rx.event = 0;
  USB.Rx.msg_count = 0;

  USB.Tx.len = 0;
  USB.Tx.event = 0;
  USB.Tx.msg_count = 0;

  ReleaseMutex();          // enable global int
}



/////////////////////////////////////////////////////////////////////////
// function : flush received USB data                                  //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void USB_AddMsg2TxBuffer(CHAR* data)
{
  while(*data != 0)
  {
    if(USB.Tx.len < MAX_USB_BUFFER_LEN)
      USB.Tx.data[USB.Tx.len++] = (BYTE)(*data++);
    else
      StopDebugger();
  }      
  USB.Tx.data[USB.Tx.len] = 0x00;
}



/////////////////////////////////////////////////////////////////////////
// function : receive errorcode of uALFAT                              //
// given    : timeout for msg-receiption                               //
// return   : TRUE = everything ok, FALSE = store errorlog to EEPROM   //
/////////////////////////////////////////////////////////////////////////
BYTE USB_GetMsgErrorcode (WORD timeout)
{
  BYTE* answer;
  BYTE request[3] = {"!00"};


  answer = USB_ReceiveString(timeout);        // wait for returned errorcode
  if(answer == NULL)
    return FALSE;

  if(StringCompare(&request[0], answer, 3))   // compare versionstring
    return TRUE;                              // no error

  answer = Ascii2Hex(++answer,1);             // convert ARM-errorcode to 1-HEX-Byte
  return USB_Error(ERROR_ARM_ERRORS | *answer);   // error + errorcode of ARM
}



/////////////////////////////////////////////////////////////////////////
// function : safe given errorcode to internal EEPROM                  //
// given    : error = contains given errorcode where failure happens   //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
BYTE USB_Error(WORD error)
{
  StopUSB_Device();
  SafeErrorsToEEPROM(error);    // safe error to EEPROM

  return FALSE;
}


/////////////////////////////////////////////////////////////////////////
// function : startup ARM-controller and check connection              //
// given    : nothing                                                  //
// return   : TRUE, FALSE                                              //
/////////////////////////////////////////////////////////////////////////
BYTE InitUSB_Device(void)
{
  BYTE i;

  InitUART(BAUDRATE_9600);      // init ATMEGA-Uart
  ARM_PowerEnable();            // power up ARM-Controller
  USB_StickPowerEnable();       // power up USB-Stick

  // ARM-controller sends 4msg after power up
  for(i=0; i<5; i++)
  {
    if(USB_ReceiveString(100) == NULL)
      return USB_Error(ERROR_ARM_NOT_RESPONDING_AFTER_POWER_UP);
  }

  //--------------- init connection to ARM-controller -----------------//
  // check bidirectional connection
  if(!USB_GetArmVersion())
    return USB_Error(ERROR_ARM_NO_CORRECT_VERSION_RECEIVED);

  // switch to higher baudrate (115200 baud)
  if(!USB_SwitchBaudrate())
    return USB_Error(ERROR_ARM_ERROR_SWITCHING_BAUDRATE);

  // recheck connection at high baudrate
  if(!USB_GetArmVersion())
    return USB_Error(ERROR_ARM_NO_CORRECT_VERSION_RECEIVED);

  Sleep(1000);                        // wait to charge the USB-Pwr-Cap

  //-------------------- init connection to USB-Stick -----------------//
  USB_TransmitString("J");            // detect USB device
  if(!(USB_GetMsgErrorcode(1000)))    // wait for ACK with 5sec timeout
    return USB_Error(ERROR_ARM_CANT_MOUNT_USB_STICK);
  USB_ReceiveString(100);             // wait for device-detected-code
  USB_ReceiveString(100);             // wait for errorcode

  //---- mounting of USB-Stick is very buggy!! retry several times ----//
  USB_TransmitString("U");            // mount USB-stick
  if(!(USB_GetMsgErrorcode(5000)))    // wait for ACK with 5sec timeout
    return USB_Error(ERROR_ARM_CANT_MOUNT_USB_STICK);
    
  USB_SetUALFAT_RTC();                // set RTC of uALFAT
  USB_FlushData();
  return TRUE;                        // everything went fine
}



/////////////////////////////////////////////////////////////////////////
// function : close ARM-controller connection                          //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void StopUSB_Device(void)
{
  USB_StickPowerDisable();      // USB-Stick power off
  ARM_PowerDisable();           // ARM-controller power off
  USB_FlushData();

  UCSRB  = 0x00;                // disable UART of ATMEGA
  DDRD  |= 0x03;                // set RxD + TxD to output
  PORTD &= ~0x03;               // set RxD + TxD to LOW -> prevent ARM latchup
}



/////////////////////////////////////////////////////////////////////////
// function : get firmware-revision of ARM-controller                  //
// given    : databyte                                                 //
// return   : TRUE if correct version-string received                  //
/////////////////////////////////////////////////////////////////////////
BYTE USB_GetArmVersion(void)
{
  BYTE i;
  BYTE request[6] = {"uALFAT"};
  BYTE* answer;


  ClearEvent(EVENT_USB_MSG);            // clear possible pending event
  answer = USB_SendQuery("V", 100);     // wait for "uALFAT 2.05"
  if(answer == NULL)
    return FALSE;

  if(!(StringCompare(&request[0], answer, 6)))  // compare versionstring
    return FALSE;

  for(i=0;i<4;i++)
    USB.uALFAT_version[i] = *(answer+7+i);      // copy string to usb-struct
  USB.uALFAT_version[4] = 0x00;                 // mark end of string

  return USB_GetMsgErrorcode(100);  // wait for errorcode of uALFAT
}



/////////////////////////////////////////////////////////////////////////
// function : updates the firmware of the uALFAT direct from USB-stick //
// given    : databyte                                                 //
// return   : TRUE if update was successfull                           //
/////////////////////////////////////////////////////////////////////////
BYTE USB_UpdateUALFAT_firmware(void)
{
  BYTE  i;
  BYTE  request[3] = {"!00"};
  BYTE* answer;


  //--------------- init connection to ARM-controller -----------------//
  USB_FlushData();
  InitUART(BAUDRATE_9600);      // init ATMEGA-Uart
  ARM_PowerEnable();            // power up ARM-controller

  // ARM-controller sends 4msg after power up
  for(i=0; i<5; i++)
  {
    if(USB_ReceiveString(100) == NULL)
      return USB_Error(ERROR_ARM_NOT_RESPONDING_AFTER_POWER_UP);
  }

  // check bidirectional connection
  if(!USB_GetArmVersion())
    return USB_Error(ERROR_ARM_NO_CORRECT_VERSION_RECEIVED);

  USB_StickPowerEnable();               // power-up usb-stick
  USB_FlushData();


  //--------------- begin firmware-update of uALFAT -------------------//
  USB_TransmitString("X U");            // load firmware from USB-Stick

  do
  {
    answer = USB_ReceiveString(5000);   // wait for 1msg; timeout 5sec (normal-msg-rec every ~200ms)
    if(answer == NULL)                  // answer timed out -> error
      return FALSE;
  }
  while(*answer != '!');                // wait till error-msg "!xx" received

  StopUSB_Device();                     // power down uALFAT and USB-stick

  // if errorcode "!00" received -> everything was ok !!
  if(!(StringCompare(&request[0], answer, 3)))  // compare errorcode "!00"
  {
    answer = Ascii2Hex(++answer,1);     // convert ARM-errorcode to 1-HEX-Byte
    return USB_Error(ERROR_ARM_ERRORS | *answer);   // error + errorcode of ARM
  }

  return TRUE;                          // update was successful
}



/////////////////////////////////////////////////////////////////////////
// function : switch the baudrate of the uALFAT-connection             //
// given    : nothing                                                  //
// return   : TRUE = everything ok, FALSE = store errorlog to EEPROM   //
/////////////////////////////////////////////////////////////////////////
BYTE USB_SwitchBaudrate (void)
{
  USB_TransmitString("B 1EF4");         // switch baudrate to 115200
  if(!USB_GetMsgErrorcode(100))
    return USB_Error(ERROR_ARM_ERROR_SWITCHING_BAUDRATE);

  InitUART(BAUDRATE_115200);            // config ATMEL-Uart to 115200
  if(!USB_GetMsgErrorcode(500))
    return USB_Error(ERROR_ARM_ERROR_SWITCHING_BAUDRATE);

  return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : set RTC of uALFAT to Systemtime                          //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void USB_SetUALFAT_RTC(void)
{
  LONG time;
  //CHAR buf[11] = {"S "};
  buf[0] = 'S';
  buf[1] = ' ';

  time  = Bcd2Hex(System.Time.year + 0x20) << 25;
  time |= Bcd2Hex(System.Time.month) << 21;
  time |= Bcd2Hex(System.Time.day) << 16;
  time |= Bcd2Hex(System.Time.hour) << 11;
  time |= Bcd2Hex(System.Time.min) << 5;
  time |= Bcd2Hex(System.Time.sec) / 2;
  Long2AsciiHex(time, (BYTE*)&buf[2]);

  USB_SendQuery("T S", 100);          // uALFAT-RTC clocksource from internal timer
  USB_SendQuery(&buf[0], 100);        // set given time to uALFAT
}



/////////////////////////////////////////////////////////////////////////
// function : opens the given filehandle                               //
// given    : filename -> defined in usb.h                             //
// return   : TRUE = everything ok, FALSE = something failed           //
/////////////////////////////////////////////////////////////////////////
BYTE OpenFile(BYTE file)
{
  //CHAR buf[20];

  switch(file)
  {
     case SYSTEM_LOG   : strcpy(&buf[0], "O 0A>SYSTEM.LOG");   break;
     case SSETTING_LOG : strcpy(&buf[0], "O 1A>SSETTING.LOG"); break;

     case SENSOR_1_LOG : strcpy(&buf[0], "O 2A>SENSOR_1.LOG"); break;
     case SENSOR_2_LOG : strcpy(&buf[0], "O 3A>SENSOR_2.LOG"); break;
     case SENSOR_3_LOG : strcpy(&buf[0], "O 2A>SENSOR_3.LOG"); break;
     case SENSOR_4_LOG : strcpy(&buf[0], "O 3A>SENSOR_4.LOG"); break;
     
     case SENSOR_FAST_LOG: strcpy(&buf[0], "O 0A>S_FAST.CSV"); break;
  }

  USB_TransmitString(&buf[0]);             // open file
  if(!(USB_GetMsgErrorcode(2000)))         // wait for ACK with 2sec timeout
    return FALSE;

  return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : close the given filehandle                               //
// given    : filename -> defined in usb.h                             //
// return   : TRUE = everything ok, FALSE = something failed           //
/////////////////////////////////////////////////////////////////////////
BYTE CloseFile(BYTE file)
{
  CHAR tmp[4] = {'C',' ','x', 0x00};

  switch(file)
  {
     case SENSOR_FAST_LOG:
     case SYSTEM_LOG   : tmp[2] = '0'; break;
     case SSETTING_LOG : tmp[2] = '1'; break;

     case SENSOR_1_LOG : tmp[2] = '2'; break;
     case SENSOR_2_LOG : tmp[2] = '3'; break;
     case SENSOR_3_LOG : tmp[2] = '2'; break;
     case SENSOR_4_LOG : tmp[2] = '3'; break;
  }

  USB_TransmitString(&tmp[0]);                     // close file
  if(!(USB_GetMsgErrorcode(1000)))                 // wait for ACK with 1sec timeout
    return FALSE;

  return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : write stored data in USB.Tx-buffer to the given filehandle/
// given    : file                                                     //
// return   : TRUE = everything ok, FALSE = something failed           //
/////////////////////////////////////////////////////////////////////////
BYTE USB_WriteBuffer2File(BYTE file)
{
  BYTE i;
  //CHAR buf[10];

  strcpy(&buf[0], "W x>");                         // build write-command : x=filehandle
  switch(file)                                     // insert number of filehandle
  {
     case SENSOR_FAST_LOG:
     case SYSTEM_LOG   : buf[2] = '0'; break;
     case SSETTING_LOG : buf[2] = '1'; break;

     case SENSOR_1_LOG : buf[2] = '2'; break;
     case SENSOR_2_LOG : buf[2] = '3'; break;
     case SENSOR_3_LOG : buf[2] = '2'; break;
     case SENSOR_4_LOG : buf[2] = '3'; break;
  }

  Word2AsciiHex(USB.Tx.len, (BYTE*)&buf[4]);              // insert length of bytes to transmit

  USB_TransmitString(&buf[0]);                     // send write-command
  if(!(USB_GetMsgErrorcode(500)))                  // wait for ACK with 500ms timeout
    return FALSE;

  i = 0xFF;
  while(USB.Tx.data[++i] != 0)                     // transmit user data without <CR>
    TransmitUSB_Byte(USB.Tx.data[i]);

  USB_ReceiveString(1000);                         // get number of written bytes; 1sec timeout
  if(!(USB_GetMsgErrorcode(500)))                  // wait for ACK with 500ms timeout
    return FALSE;

  USB_FlushData();                                 // flush internal USB-Buffer of the datalogger

  return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : sensorsettings changed -> report to USB-Stick            //
//            FILE : "SSETTING.LOG"                                    //
// given    : nothing                                                  //
// return   : TRUE = everything ok                                     //
/////////////////////////////////////////////////////////////////////////
BYTE USB_LogSensorSettings(void)
{
  BYTE   i;
  //CHAR   buf[10];


  if(!InitUSB_Device())                               // pwrup uALFAT
    return USB_Error(ERROR_ARM_INIT_FAILED);

  if(!OpenFile(SSETTING_LOG))                         // open file
    return USB_Error(ERROR_ARM_DOLOG_FOPEN_ERROR);


  //-------------- add headline to internal usb-tx-buffer ------------//
  USB_AddMsg2TxBuffer("-->> sensorsettings changed : ");
  USB_AddMsg2TxBuffer((CHAR*)Date2Hex((s_time*)(&System.Time), (BYTE*)&buf[0], 0x30));
  USB_AddMsg2TxBuffer(" - ");
  USB_AddMsg2TxBuffer((CHAR*)Time2Hex((s_time*)(&System.Time), (BYTE*)&buf[0], 0x30));
  USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 2));

  if(!(USB_WriteBuffer2File(SSETTING_LOG)))                // write buffer to file
  {
    CloseFile(SSETTING_LOG);
    return USB_Error(ERROR_ARM_SSETTING_FAPPEND_ERROR);
  }

  //-------------- report setting for each sensor (1...4) ------------//
  for(i=0; i<NUM_SENSOR; i++)
  {
    USB_AddMsg2TxBuffer("sensor ");
    buf[0] = i + '1';
    buf[1] = ':';
    buf[2] = 0x00;
    USB_AddMsg2TxBuffer(&buf[0]);
    USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 1));

    // status : "enabled" or "disabled"
    USB_AddMsg2TxBuffer("  status      = ");
    switch(Sensor.Nr[i].Enabled)
    {
      case Sensor_Disable : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_DISABLE); break;
      case Sensor_Enable : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_ENABLE); break;
      default : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_NOT_SET); break;
    }
    USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 1));

    // type of sensor
    USB_AddMsg2TxBuffer("  type        = ");
    switch(Sensor.Nr[i].Type)
    {
      //case Sensor_0_10VDC : USB_AddMsg2TxBuffer(STRING_MENU_SENSOR_0_10VDC); break;
      //case Sensor_0_20mA  : USB_AddMsg2TxBuffer(STRING_MENU_SENSOR_0_20MA);  break;
      case Sensor_4_20mA  : USB_AddMsg2TxBuffer(STRING_MENU_SENSOR_4_20MA);  break;
      case Sensor_Impulse : USB_AddMsg2TxBuffer(STRING_MENU_SENSOR_IMPULSE); break;
      default : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_NOT_SET); break;
    }
    USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 1));


    // tx-buffer becomes full -> transmit to stick
    if(!(USB_WriteBuffer2File(SSETTING_LOG)))       // write buffer to file
    {
      CloseFile(SSETTING_LOG);
      return USB_Error(ERROR_ARM_SSETTING_FAPPEND_ERROR);
    }

    // unit of sensor
    USB_AddMsg2TxBuffer("  unit        = ");
    switch(Sensor.Nr[i].Unit)
    {
      case Unit_mbar : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_UNIT_MBAR); break;
      case Unit_bar  : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_UNIT_BAR);  break;
      case Unit_C    : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_UNIT_C);    break;
      case Unit_l    : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_UNIT_L);    break;
      case Unit_hl   : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_UNIT_HL);   break;
      case Unit_m3   : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_UNIT_M3);   break;
      default : USB_AddMsg2TxBuffer(STRING_SET_SENSOR_NOT_SET); break;
    }
    USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 1));

    // timeinterval between 2 measurements
    USB_AddMsg2TxBuffer("  interval    = ");
    USB_AddMsg2TxBuffer((CHAR*)Seconds2TimeString(Sensor.Nr[i].MeasureInterval, (BYTE*)&buf[0]));
    USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 1));

    // multiplication-factor (example : 10impulses per m3 -> factor=0.1 )
    USB_AddMsg2TxBuffer("  multifactor = ");
    USB_AddMsg2TxBuffer((CHAR*)Float2AsciiDec(Sensor.Nr[i].MultiplyFactor, (BYTE*)&buf[0]));
    if(i == 3)
      USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 4));// more spaces at end of config
    else
      USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 2));


    // tx-buffer becomes full -> transmit to stick
    if(!(USB_WriteBuffer2File(SSETTING_LOG)))         // write buffer to file
    {
      CloseFile(SSETTING_LOG);
      return USB_Error(ERROR_ARM_SSETTING_FAPPEND_ERROR);
    }
  }


  if(!CloseFile(SSETTING_LOG))                        // close file
    return USB_Error(ERROR_ARM_SSETTING_FCLOSE_ERROR);


  Sleep(500);                           // wait till uALFAT has finished datatransfer
  StopUSB_Device();                     // power down uALFAT and USB-stick

  return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : safe logged sensor-value from external EEPROM to USB-Stick/
// given    : nothing                                                  //
// return   : TRUE = everything ok, FALSE = something went wrong       //
/////////////////////////////////////////////////////////////////////////
BYTE USB_LogSensorValues(void)
{
  if(!LogValues_USB())
  {
    CloseFile(SYSTEM_LOG);      // close all four (possible open) file-handles
    CloseFile(SSETTING_LOG);
    CloseFile(SENSOR_1_LOG);
    CloseFile(SENSOR_2_LOG);

    return FALSE;
  }

  return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : safe logged sensor-value from external EEPROM to USB-Stick/
//            FILE : "SENSOR_1.LOG"  ->  values of sensor 1            //
//                   "SENSOR_2.LOG"  ->  values of sensor 2            //
//                   "SENSOR_3.LOG"  ->  values of sensor 3            //
//                   "SENSOR_4.LOG"  ->  values of sensor 4            //
//                   "SYSTEM.LOG"    ->  values of onboard tempsensor  //
// given    : nothing                                                  //
// return   : TRUE = everything ok                                     //
/////////////////////////////////////////////////////////////////////////
BYTE LogValues_USB(void)
{
  WORD   i;
  //CHAR   buf[20];
  BYTE   x, sens_nr, two_turns, safe_log;
  BYTE   fhandle[4] = {SENSOR_1_LOG, SENSOR_2_LOG, SENSOR_3_LOG, SENSOR_4_LOG};
  s_time tmp_time;
  union  union_w_b  num_logs;
  s_eeprom_logging  eeprom_log;




  //---------------- get number of safed logs --------------------------//
  EEPROM_Bulk_Read(EXT_EEPROM_NUM_LOGS_POS, sizeof(WORD), &num_logs.b[0]);
  if(num_logs.w == 0x00)                  // no sensor-data safed ! -> return
    return TRUE;

  if(!InitUSB_Device())                   // init uALFAT
    return USB_Error(ERROR_ARM_INIT_FAILED);

  if(!OpenFile(SYSTEM_LOG))                           // safe system-parameters
    return USB_Error(ERROR_ARM_DOLOG_FOPEN_ERROR);



  two_turns = FALSE;
  for(x=0; x<2; x++)            // 2 turns (file 1+2, file 3+4)
  {
    //-------------- open sensor-log-files for append ---------------//
    if(x == 0)
    {
      if(!OpenFile(SENSOR_1_LOG))                         // sensor1-log
        return USB_Error(ERROR_ARM_DOLOG_FOPEN_ERROR);
      if(!OpenFile(SENSOR_2_LOG))                         // sensor2-log
        return USB_Error(ERROR_ARM_DOLOG_FOPEN_ERROR);
    }
    else
    {
      if(!OpenFile(SENSOR_3_LOG))                         // sensor3-log
        return USB_Error(ERROR_ARM_DOLOG_FOPEN_ERROR);
      if(!OpenFile(SENSOR_4_LOG))                         // sensor4-log
        return USB_Error(ERROR_ARM_DOLOG_FOPEN_ERROR);
    }



    //----------- safe logs from external EEPROM to USB-Stick ------------//
    for(i=0; i<num_logs.w; i++)
    {
      safe_log = TRUE;    // safe log to file is default true

      // get one measurement-structure from eeprom
      EEPROM_Bulk_Read(EXT_EEPROM_START_OF_LOGS + (i * sizeof(s_eeprom_logging)),
                       sizeof(s_eeprom_logging),
                       (BYTE*)(&eeprom_log));

      sens_nr = (eeprom_log.sensorvalue >> 14);    // get number of sensor


      //---- check if found sensorvalue can be safed to open filehandle ----//
      if(sens_nr > 1)
      {
        two_turns = TRUE;     // mark that sensorvalue from sensor3+4 in EEPROM stored
        if(x==0)
          safe_log = FALSE;   // found sensorvalue3 oder 4 -> but still safeing 1+2
      }
      else
      {
        if(x==1)
          safe_log = FALSE;   // found sensorvalue1 oder 2 -> but still safeing 3+4
      }



      //------------------ if filehandle open -> safe to file -----------------//
      if(safe_log)
      {
        // write timestamp
        DecodeSystemTime(eeprom_log.timestamp, (s_time*)(&tmp_time));
        ModifyTimestruct2BCD((s_time*)(&tmp_time));
        USB_AddMsg2TxBuffer((CHAR*)Date2Hex((s_time*)(&tmp_time), (BYTE*)&buf[0], 0x30));
        USB_AddMsg2TxBuffer(" - ");
        USB_AddMsg2TxBuffer((CHAR*)Time2Hex((s_time*)(&tmp_time), (BYTE*)&buf[0], 0x30));
        USB_AddMsg2TxBuffer(" : ");


        //----->>>>>   write sensor-value to "SENSOR_x.LOG"
        if(Sensor.Nr[sens_nr].Type == Sensor_4_20mA)
        {
           if((eeprom_log.sensorvalue&0xFFF) > 0)
              USB_AddMsg2TxBuffer(calcPressure(Sensor.Nr[i].LastMeasurement, &buf[0]));
           else
              USB_AddMsg2TxBuffer("0");
        }
        else
          USB_AddMsg2TxBuffer((CHAR*)Word2AsciiDec((eeprom_log.sensorvalue&0xFFF), (BYTE*)&buf[0]));
        
        USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 1));
        if(!(USB_WriteBuffer2File(fhandle[sens_nr])))     // write buffer to "SENSOR.LOG"
          return USB_Error(ERROR_ARM_DOLOG_FAPPEND_ERROR);


        //----->>>>>   write boardtemp and supply-voltage to "SYSTEM.LOG"
        USB.Tx.len = 22;                                  // len = "21.07.07 - 13:24:25 : "
        USB_AddMsg2TxBuffer("temp = ");
        USB_AddMsg2TxBuffer((CHAR*)Byte2AsciiDec(eeprom_log.boardtemp, (BYTE*)&buf[0], SIGNED_BYTE));
        USB_AddMsg2TxBuffer(", supply = ");
        USB_AddMsg2TxBuffer((CHAR*)Word2AsciiDec(eeprom_log.supplyvoltage, (BYTE*)&buf[0]));
        USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 1));

        if(!(USB_WriteBuffer2File(SYSTEM_LOG)))           // write buffer to "SYSTEM.LOG"
          return USB_Error(ERROR_ARM_DOLOG_FAPPEND_ERROR);
      }
    }


    if(x == 0)
    {
      //----------------- close sensor-log-files -------------------------//
      if(!CloseFile(SENSOR_1_LOG))                      // close file 1
       return USB_Error(ERROR_ARM_DOLOG_FCLOSE_ERROR);
      if(!CloseFile(SENSOR_2_LOG))                      // close file 2
       return USB_Error(ERROR_ARM_DOLOG_FCLOSE_ERROR);

      if(two_turns == FALSE)    // no sensor3+4 values -> exit for-'x'-loop
        break;
    }
    else
    {
      if(!CloseFile(SENSOR_3_LOG))                      // close file 3
        return USB_Error(ERROR_ARM_DOLOG_FOPEN_ERROR);
      if(!CloseFile(SENSOR_4_LOG))                      // close file 4
        return USB_Error(ERROR_ARM_DOLOG_FOPEN_ERROR);
    }


  } // for(x)




  if(!CloseFile(SYSTEM_LOG))                            // close system-file
    return USB_Error(ERROR_ARM_DOLOG_FCLOSE_ERROR);

  //--------------- wait for till USB-Stick has finished --------------//
  Sleep(500);                           // wait till uALFAT has finished datatransfer
  StopUSB_Device();                     // power down uALFAT and USB-stick


  num_logs.w = 0x0000;                  // set EEPROM-data-index to ZERO
  EEPROM_Bulk_Write(EXT_EEPROM_NUM_LOGS_POS, sizeof(WORD), &num_logs.b[0]);

  return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : safe logged sensor-value from internal RAM to USB-Stick  //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void USB_LogSensorValuesFast(void)
{
   if(USB.usb_init_done == FALSE)
   {
      if(!InitUSB_Device())                              // init uALFAT
      {
         USB_Error(ERROR_ARM_INIT_FAILED);
         return;
      }         
      
      USB.usb_init_done = TRUE;
   }

   if(!LogValuesFast_USB(2))
      CloseFile(SENSOR_FAST_LOG);  
}  



/////////////////////////////////////////////////////////////////////////
// function : safe logged sensor-value from internal RAM to USB-Stick  //
// given    : nothing                                                  //
// return   : TRUE = everything ok, FALSE = something went wrong       //
/////////////////////////////////////////////////////////////////////////
BYTE LogValuesFast_USB(BYTE chl)
{
   BYTE i;
   WORD val;

   
   EncodeSystemTime((s_time*)(&System.Time));         // get actual time

   if(!OpenFile(SENSOR_FAST_LOG))                     // safe system-parameters
      return USB_Error(ERROR_ARM_DOLOG_FOPEN_ERROR);

   // write timestamp
   USB_AddMsg2TxBuffer((CHAR*)Date2Hex(&System.Time, (BYTE*)&buf[0], 0x30));
   USB_AddMsg2TxBuffer(";");
   USB_AddMsg2TxBuffer((CHAR*)Time2Hex(&System.Time, (BYTE*)&buf[0], 0x30));
   USB_AddMsg2TxBuffer(";");


   //----->>>>>   write sensor-values to "S_FAST.LOG"
   for(i=0; i<FAST_LOG_BUF_SIZE; i++)
   {
      val = Sensor.Nr[chl].FastLog.buf[Sensor.Nr[chl].FastLog.buf_idx_ready_for_usb][i];
      if(i !=0 )                         // skip intent for line with timestamp
         USB_AddMsg2TxBuffer(";;");
      
      if(val == 0)
         USB_AddMsg2TxBuffer("0");
      else
         USB_AddMsg2TxBuffer(calcPressure(val, &buf[0]));
      USB_AddMsg2TxBuffer(AddNewLine2Str(&buf[0], 1));
   }

   if(!(USB_WriteBuffer2File(SENSOR_FAST_LOG)))     // write buffer to "S_FAST.LOG"
      return USB_Error(ERROR_ARM_DOLOG_FAPPEND_ERROR);

   if(!CloseFile(SENSOR_FAST_LOG))                       // close system-file
      return USB_Error(ERROR_ARM_DOLOG_FCLOSE_ERROR);

   return TRUE;
}

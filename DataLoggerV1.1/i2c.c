#include "main.h"
#include "i2c.h"
#include "errorcodes.h"
#include "tools.h"


s_timestamp    CodedTimestamp;


/////////////////////////////////////////////////////////////////////////
// function : read system-time and system-alarmtime of external RTC    //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void ReadRTC(void)
{
   PCF8563_Bulk_Read(0x02, sizeof(s_time), (BYTE*)(&System.Time));
   PCF8563_Bulk_Read(0x09, sizeof(s_timealert), (BYTE*)(&System.Alert));

   System.Time.sec      &= 0x7F;    // mask unused bits
   System.Time.min      &= 0x7F;    // -> not used bits of RTC
   System.Time.hour     &= 0x3F;    //     may not always be 0
   System.Time.day      &= 0x3F;
   System.Time.weekday  &= 0x07;
   System.Time.month    &= 0x1F;

   System.Alert.min     &= 0x7F;
   System.Alert.hour    &= 0x3F;
   System.Alert.day     &= 0x3F;
   System.Alert.weekday &= 0x07;
}



/////////////////////////////////////////////////////////////////////////
// function : write system-time to external RTC                        //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void WriteRTC(void)
{
   PCF8563_Bulk_Write(0x02, sizeof(s_time), (BYTE*)(&System.Time));
}



/////////////////////////////////////////////////////////////////////////
// function : write system-alarmtime to external RTC                   //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void WriteRTC_Alert(void)
{
   PCF8563_Bulk_Write(0x09, sizeof(s_timealert), (BYTE*)(&System.Alert));
}



/////////////////////////////////////////////////////////////////////////
// function : write alarmtimer to external RTC                         //
// given    : val   = countdown-value                                  //
//            clock = seconds or minutes                               //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void LoadAlarmTimer(BYTE val, BYTE clock)
{
   PCF8563_Bulk_Write(0x0E, 1, &clock);   // write control register
   PCF8563_Bulk_Write(0x0F, 1, &val);     // write timer register
}



/////////////////////////////////////////////////////////////////////////
// function : write alarmtimer to external RTC                         //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void StopAlarmTimer(void)
{
   BYTE val = 0x00;
   PCF8563_Bulk_Write(0x01, 1, &val);     // clear timer-flag + disable int
   PCF8563_Bulk_Write(0x0E, 1, &val);     // write timer-register
}



/////////////////////////////////////////////////////////////////////////
// function : write alarmtimer to external RTC                         //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void StartAlarmTimer(void)
{
   BYTE val;

   PCF8563_Bulk_Read(0x0E, 1, &val);      // get control register
   val |= 0x80;                           // set startbit
   val &= 0x83;                           // clear unused bits
   PCF8563_Bulk_Write(0x0E, 1, &val);     // start timer
   val  = 0x01;
   PCF8563_Bulk_Write(0x01, 1, &val);     // enable interrupt
}



/////////////////////////////////////////////////////////////////////////
// function : read the onboard temperature sensor                      //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void ReadOnboardTemp(void)
{
  LM75_Write(0x01, 0x00);                           // start temp-conversion
  Sleep(300);                                       // wait for 300ms
  System.BoardTemp = (SBYTE)(LM75_Read(0x00) >> 8); // read register at 0x00
  LM75_Write(0x01, 0x01);                           // set sensor to shutdownmode
}



/////////////////////////////////////////////////////////////////////////
// function : hw-specifc I2C write using ATmega's TWI                  //
// given    : data = send data to I2C slave                            //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void I2C_Write_Byte(BYTE data)
{
   TWDR = data;
   TWCR = (TWCR & 0x0F) | (1<<TWINT) | (1<<TWEN);
}



/////////////////////////////////////////////////////////////////////////
// function : hw-specifc I2C read using ATmega's TWI                   //
// given    : ack = should ATmega send an ACK                          //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void I2C_Read_Byte(BYTE ack)
{
   if(ack)
      TWCR = ((TWCR & 0x0F) | (1<<TWINT) | (1<<TWEA) );
   else
      TWCR = ((TWCR & 0x0F) | (1<<TWINT) );
}



/////////////////////////////////////////////////////////////////////////
// function : write given data to external EEPROM                      //
// given    : RegAddr = address where data should be written           //
//            len     = number of bytes                                //
//            data    = data                                           //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
WORD EEPROM_Bulk_Write(WORD RegAddr, BYTE len, BYTE* data)
{
  BYTE rest;

  if((RegAddr & 0xFF80) == ((RegAddr + len - 1) & 0xFF80))  // check if page-write is possible
    return EEPROM_Bulk_Write_Page(RegAddr, len, data);
  else
  {
    rest = 0x0080 - (RegAddr & 0x007F);        // calc rest of 128bytes (see datasheet)

    if(!EEPROM_Bulk_Write_Page(RegAddr, rest, data))
      return FALSE;

    data    += rest;
    RegAddr += rest;
    return (EEPROM_Bulk_Write_Page(RegAddr, len-rest, data));
  }
}




/////////////////////////////////////////////////////////////////////////
// function : write given data to external EEPROM  with the            //
//            page-write-sequence : neccessary because addressbit_15   //
//                                  to addressbit_7 have to be the same//
//                                  for page-write : see datasheet     //
//                                                   "3.8 page write"  //
// given    : RegAddr = address where data should be written           //
//            len     = number of bytes                                //
//            data    = data                                           //
// return   : TRUE if OK - FALSE if write failed                       //
/////////////////////////////////////////////////////////////////////////
WORD EEPROM_Bulk_Write_Page(WORD RegAddr, BYTE len, BYTE* data)
{
  EepromWriteEnable();                     // disable write protection

  I2C_SendStart();                         // set startcondition
  I2C_Wait();
  if((TWSR & 0xF8) != I2C_FLAG_START)      // check if error occoured at startcondition
    return I2C_Error(ERROR_EEPROM_WRITE_NO_STARTCOND_SENT);

  I2C_Write_Byte(HW_ADDRESS_EEPROM);       // write device-address of EEPROM
  I2C_Wait();
  if((TWSR & 0xF8) != I2C_FLAG_ADDR_TX_ACK_OK)
    return I2C_Error(ERROR_EEPROM_WRITE_NO_ADDR_ACK_RECEIVED);

  I2C_Write_Byte(RegAddr >> 8);
  I2C_Wait();
  if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
    return I2C_Error(ERROR_EEPROM_WRITE_NO_DATA_ACK_RECEIVED);

  I2C_Write_Byte(RegAddr & 0xff);
  I2C_Wait();
  if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
    return I2C_Error(ERROR_EEPROM_WRITE_NO_DATA_ACK_RECEIVED);

  while(len--)
  {
    I2C_Write_Byte( *data++ );
    I2C_Wait();
    if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
      return I2C_Error(ERROR_EEPROM_WRITE_NO_DATA_ACK_RECEIVED);
  }

  I2C_SendStop();
  while(!(1<<TWSTO));

  EepromWriteDisable();          // enable write protection
  Sleep(10);                     // wait till all data is written (see datasheet)

  return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : read data from external EEPROM                           //
// given    : RegAddr = address where data should be written           //
//            len     = number of bytes                                //
//            data    = target-dataspace                               //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
WORD EEPROM_Bulk_Read(WORD RegAddr, BYTE len, BYTE *data)
{
   I2C_SendStart();
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_START)
     return I2C_Error(ERROR_EEPROM_READ_NO_STARTCOND_SENT);

   I2C_Write_Byte(HW_ADDRESS_EEPROM);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_ADDR_TX_ACK_OK)
     return I2C_Error(ERROR_EEPROM_READ_NO_ADDR_ACK_RECEIVED);

   I2C_Write_Byte(RegAddr >> 8);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
     return I2C_Error(ERROR_EEPROM_READ_NO_DATA_ACK_RECEIVED);

   I2C_Write_Byte(RegAddr & 0xff);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
     return I2C_Error(ERROR_EEPROM_READ_NO_DATA_ACK_RECEIVED);

   I2C_SendStart();
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_START_REPEATED)
     return I2C_Error(ERROR_EEPROM_READ_NO_STARTCOND_SENT);

   I2C_Write_Byte( HW_ADDRESS_EEPROM | 0x01 );
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_ADDR_TX_AGAIN_ACK_OK)
     return I2C_Error(ERROR_EEPROM_READ_NO_ADDR_ACK_AGAIN_RECEIVED);

   while(len > 1)
   {
      I2C_Read_Byte(ACK);
      I2C_Wait();
      *data++ = TWDR;
      len--;
   }

   I2C_Read_Byte(NACK);
   I2C_Wait();
   *data = TWDR;

   I2C_SendStop();
   _delay_us(5);

   return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : write data to external temperature sensor (alerts, ...)  //
// given    : RegAddr = address where data should be written           //
//            data    = data                                           //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
WORD LM75_Write(BYTE RegAddr, BYTE data)
{
   I2C_SendStart();
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_START)
     return I2C_Error(ERROR_EEPROM_WRITE_NO_STARTCOND_SENT);

   I2C_Write_Byte(HW_ADDRESS_LM75);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_ADDR_TX_ACK_OK)
     return I2C_Error(ERROR_BOARDTEMP_WRITE_NO_ADDR_ACK_RECEIVED);

   I2C_Write_Byte(RegAddr);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
     return I2C_Error(ERROR_BOARDTEMP_WRITE_NO_DATA_ACK_RECEIVED);

   I2C_Write_Byte(data);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
     return I2C_Error(ERROR_BOARDTEMP_WRITE_NO_DATA_ACK_RECEIVED);

   I2C_SendStop();
   while(!(1<<TWSTO));

   return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : read value of external temperature sensor                //
// given    : RegAddr = address where data should be read              //
// return   : data                                                     //
/////////////////////////////////////////////////////////////////////////
WORD LM75_Read(BYTE RegAddr)
{
   WORD data;

   I2C_SendStart();
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_START)
     return I2C_Error(ERROR_BOARDTEMP_READ_NO_STARTCOND_SENT);

   I2C_Write_Byte(HW_ADDRESS_LM75);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_ADDR_TX_ACK_OK)
     return I2C_Error(ERROR_BOARDTEMP_READ_NO_ADDR_ACK_RECEIVED);

   I2C_Write_Byte(RegAddr);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
     return I2C_Error(ERROR_BOARDTEMP_READ_NO_DATA_ACK_RECEIVED);

   I2C_SendStart();
   I2C_Wait();

   I2C_Write_Byte(HW_ADDRESS_LM75 | 0x01);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_ADDR_TX_AGAIN_ACK_OK)
     return I2C_Error(ERROR_BOARDTEMP_READ_NO_ADDR_ACK_AGAIN_RECEIVED);

   I2C_Read_Byte(ACK);
   I2C_Wait();
   data = TWDR;
   data = data << 8;

   I2C_Read_Byte(NACK);
   I2C_Wait();
   data |= TWDR;
   I2C_SendStop();

return data;
}



/////////////////////////////////////////////////////////////////////////
// function : write given data to external RTC                         //
// given    : RegAddr = address where data should be written           //
//            len     = number of bytes                                //
//            data    = data                                           //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
WORD PCF8563_Bulk_Write(BYTE RegAddr, BYTE len, BYTE* data)
{
   I2C_SendStart();
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_START)
     return I2C_Error(ERROR_RTC_WRITE_NO_STARTCOND_SENT);

   I2C_Write_Byte(HW_ADDRESS_PCF8563_WRITE);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_ADDR_TX_ACK_OK)
     return I2C_Error(ERROR_RTC_WRITE_NO_ADDR_ACK_RECEIVED);

   I2C_Write_Byte(RegAddr);
   I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
     return I2C_Error(ERROR_RTC_WRITE_NO_DATA_ACK_RECEIVED);

   while(len)
   {
      I2C_Write_Byte( *data++ );
      I2C_Wait();
      if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
        return I2C_Error(ERROR_RTC_WRITE_NO_DATA_ACK_RECEIVED);

      len--;
   }

   I2C_SendStop();
   while(!(1<<TWSTO));

   return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : read data of external RTC                                //
// given    : RegAddr = address where data should be read              //
//            len     = number of bytes                                //
//            data    = data                                           //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
WORD PCF8563_Bulk_Read(BYTE RegAddr, BYTE len, BYTE *data)
{
   I2C_SendStart();                              // set startcondition
 	 I2C_Wait();
   if((TWSR & 0xF8) != I2C_FLAG_START)
     return I2C_Error(ERROR_RTC_READ_NO_STARTCOND_SENT);

 	 I2C_Write_Byte(HW_ADDRESS_PCF8563_WRITE);     // write address of RTC
	 I2C_Wait();
    if((TWSR & 0xF8) != I2C_FLAG_ADDR_TX_ACK_OK)
      return I2C_Error(ERROR_RTC_READ_NO_ADDR_ACK_RECEIVED);

	 I2C_Write_Byte(RegAddr);                      // set index of selected register
	 I2C_Wait();
    if((TWSR & 0xF8) != I2C_FLAG_DATA_TX_ACK_OK)
      return I2C_Error(ERROR_RTC_READ_NO_DATA_ACK_RECEIVED);

	 I2C_SendStart();                              // set startcondition again
	 I2C_Wait();

	 I2C_Write_Byte(HW_ADDRESS_PCF8563_READ);      // write address of RTC
	 I2C_Wait();
    if((TWSR & 0xF8) != I2C_FLAG_ADDR_TX_AGAIN_ACK_OK)
      return I2C_Error(ERROR_RTC_READ_NO_ADDR_ACK_AGAIN_RECEIVED);

	 while(len > 1)                                // receive databytes
	 {
	 	 I2C_Read_Byte(ACK);
		 I2C_Wait();
		 *data++ = TWDR;
		 len--;
	 }

	 I2C_Read_Byte(NACK);                          // read last byte with no ACK
	 I2C_Wait();
	 *data = TWDR;

	 I2C_SendStop();

   return TRUE;
}



/////////////////////////////////////////////////////////////////////////
// function : set some controlbits to 0                                //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void InitRTC(void)
{
BYTE tmp = 0x00;

   PCF8563_Bulk_Write(0x00, 1, &tmp);  // init CONTROL/STATUS1
   PCF8563_Bulk_Write(0x01, 1, &tmp);  // init CONTROL/STATUS2
   PCF8563_Bulk_Write(0x0D, 1, &tmp);  // init CLKOUT control
   PCF8563_Bulk_Write(0x0E, 1, &tmp);  // init TIMER control
   PCF8563_Bulk_Write(0x0F, 1, &tmp);  // init TIMER

   PCF8563_Bulk_Read( 0x02, 1, &tmp);  // get seconds-register
   tmp &= ~0x80;                       // clear VL-bit
   PCF8563_Bulk_Write(0x02, 1, &tmp);  // write back to RTC
}



/////////////////////////////////////////////////////////////////////////
// function : safe given errorcode to internal EEPROM                  //
// given    : error = contains given errorcode where failure happens   //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
WORD I2C_Error(WORD error)
{
  EepromWriteDisable();          // enable write protection

  I2C_SendStop();
  I2C_DisableI2C();

  SafeErrorsToEEPROM(error);

return FALSE;
}

#ifndef __I2C_H__
#define __I2C_H__


#define  HW_ADDRESS_LM75            0x90
#define  HW_ADDRESS_PCF8563_READ    0xA3
#define  HW_ADDRESS_PCF8563_WRITE   0xA2
#define  HW_ADDRESS_EEPROM          0xA0

#define  ACK      1
#define  NACK     0

#define  I2C_FLAG_START                 0x08
#define  I2C_FLAG_START_REPEATED        0x10
#define  I2C_FLAG_ADDR_TX_ACK_OK        0x18
#define  I2C_FLAG_NO_ADDR_TX_ACK        0x20
#define  I2C_FLAG_DATA_TX_ACK_OK        0x28
#define  I2C_FLAG_ADDR_TX_AGAIN_ACK_OK  0x40



#define  I2C_SendStart()    (TWCR |= ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN)) )
#define  I2C_SendStop()     (TWCR |= ((1<<TWINT) | (1<<TWSTO) | (1<<TWEN)) )
#define  I2C_Wait()         while(! (TWCR & (1<<TWINT)) )
#define  I2C_DisableI2C()   TWCR = 0x00


#define  RTC_TIMER_SEC     0x02
#define  RTC_TIMER_MIN     0x03


typedef struct
{
   LONG  CodedTime;
} s_timestamp;


extern s_timestamp    CodedTimestamp;






// prototypes

void I2C_Write_Byte(BYTE data);

void I2C_Read_Byte(BYTE ack);

void ReadRTC(void);

void WriteRTC(void);

void WriteRTC_Alert(void);

void LoadAlarmTimer(BYTE val, BYTE clock);

void StopAlarmTimer(void);

void StartAlarmTimer(void);

void ReadOnboardTemp(void);


//inline void I2C_Write_Byte(BYTE data);

//inline void I2C_Read_Byte(BYTE ack);


WORD EEPROM_Bulk_Write(WORD RegAddr, BYTE len, BYTE* data);

WORD EEPROM_Bulk_Write_Page(WORD RegAddr, BYTE len, BYTE* data);

WORD EEPROM_Bulk_Read(WORD RegAddr, BYTE len, BYTE *data);


WORD LM75_Write(BYTE RegAddr, BYTE data);

WORD LM75_Read(BYTE RegAddr);


WORD PCF8563_Bulk_Write(BYTE RegAddr, BYTE len, BYTE* data);

WORD PCF8563_Bulk_Read(BYTE RegAddr, BYTE len, BYTE *data);

void InitRTC(void);

WORD I2C_Error (WORD error);


#endif

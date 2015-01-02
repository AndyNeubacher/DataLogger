#ifndef __MAIN_H__
#define __MAIN_H__

#define F_CPU           1843200     // 1.8432MHz


typedef  char           CHAR;
typedef  signed char    SBYTE;
typedef  unsigned char  BYTE;
typedef  signed int     SWORD;
typedef  unsigned int   WORD;
typedef  unsigned long  LONG;
typedef  float          FLOAT;

union union_w_b
{
  WORD w;
  BYTE b[2];
};

union union_l_b
{
  LONG l;
  WORD w[2];
  BYTE b[4];
};


#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <string.h>
#include "language.h"

#define  TRUE                           0x01
#define  FALSE                          0x00
#define  nop()                          asm volatile ("nop")
//#define  StopDebugger()                 nop()
#define  StopDebugger()                 {__asm__ __volatile__ ("break");}
#define  soft_reset()                   do{wdt_enable(WDTO_15MS); for(;;) {} } while(0)


#define  EVENT_NO_EVENT                 0x00
#define  EVENT_1MS_TICK                 0x01
#define  EVENT_TIME_CALLBACK            0x02
#define  EVENT_BOX_OPENED               0x04
#define  EVENT_BOX_CLOSED               0x08
#define  EVENT_KEY_CHANGED              0x10
#define  EVENT_RTC_INTERRUPT            0x20
#define  EVENT_USB_MSG                  0x40
#define  EVENT_IMPULSE_INPUT_TRIGGERED  0x80

#define  EVENT_CALLBACK_CLR_IMP_SIGNAL  0x01


#define  EVENT_TIMER_NO_TICK            0x00
#define  EVENT_TIMER_100MS_TICK         0x01
#define  EVENT_TIMER_200MS_TICK         0x02
#define  EVENT_TIMER_500MS_TICK         0x04
#define  EVENT_TIMER_1S_TICK            0x08
#define  EVENT_TIMER_2S_TICK            0x10
#define  EVENT_TIMER_5S_TICK            0x20
#define  EVENT_UPDATE_DISPLAY_VALUE     0x40
#define  EVENT_FASTLOGGING_SAFE2_USB    0x80


#define  EVENT_RESULT_NOT_IMPLEMENTED   0x00
#define  EVENT_RESULT_TIMEOUT           0x01
#define  EVENT_RESULT_MSG               0x02
#define  EVENT_RESULT_SUCCESS           0x04



#define  MAX_USB_BUFFER_LEN             200
#define  MAX_ERROR_LOGS                 127


typedef struct
{
   BYTE len;
} s_system_error;


typedef struct
{
   BYTE data[MAX_USB_BUFFER_LEN];
   BYTE len;
   BYTE event;
   BYTE msg_count;
} s_serial_buf;


typedef struct
{
   BYTE  sec;
   BYTE  min;
   BYTE  hour;
   BYTE  day;
   BYTE  weekday;
   BYTE  month;
   BYTE  year;
} s_time;


typedef struct
{
   BYTE  min;
   BYTE  hour;
   BYTE  day;
   BYTE  weekday;
} s_timealert;


typedef struct
{
   BYTE  Detected;
   BYTE  DebounceTime;
   BYTE  Valid;
} s_key;


typedef struct
{
   CHAR           uALFAT_version[5];
   BYTE           usb_init_done;
   s_serial_buf   Rx;
   s_serial_buf   Tx;
} s_usb;


typedef struct
{
   BYTE  LcdUpdate;
} s_flags;


typedef struct
{
   WORD   msTimer;
   WORD   secTimer;
   WORD   callbackTimer;

   volatile BYTE EventID;
   volatile BYTE EventTimer;
   BYTE          EventMsg;
   BYTE          CallbackEvent;

   SBYTE         BoardTemp;
   WORD          SupplyVoltage;

   s_flags       Flags;
   s_key         Key;
   s_time        Time;
   s_timealert   Alert;
} s_system;


extern   s_system          System;
extern   s_system_error    SystemError;
extern   s_usb             USB;


// global int
#define  DisableGlobalInterrupt()     cli()
#define  EnableGlobalInterrupt()      sei()

#define  wdt_reset()                  __asm__ __volatile__ ("wdr")

// Mutex -> enable/disable global int
#define  MutexFunc(x)                 ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {x;}
#define  GetMutex();                  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
#define  ReleaseMutex();              }




#define  SetEvent(event)              MutexFunc(System.EventID |= (event);)
#define  SetEventMsg(event, msg)      MutexFunc((System.EventID |= (event); System.EventMsg = (msg));)
#define  ClearEvent(event)            MutexFunc((System.EventID &= ~(event));)
#define  IsEventPending(event)        (System.EventID & (event))

#define  SetTimeCallback(cb_event, time)    MutexFunc(System.CallbackEvent |= (cb_event); System.callbackTimer = (time);)
#define  ClearTimeCallback(cb_event)        MutexFunc(System.CallbackEvent &= ~(cb_event);)

#define  SetTimerEvent(event)         MutexFunc(System.EventTimer |= (event);)
#define  ClearTimerEvent(event)       MutexFunc((System.EventTimer &= ~(event));)


#define  PowerSaveModeEnable()
#define  PowerSaveModeDisable()

#define  DisableRTC_Int()             GICR &= ~0x80
#define  EnableRTC_Int()              GICR |=  0x80

#define  DisableBoxSwitchInt()        GICR &= ~0x40
#define  EnableBoxSwitchInt()         {GIFR |= 0x40; GICR |=  0x40;}


//************** hardware defines **************//
#define  GetImpulseInput()       (PINC & 0x40)
#define  IsBoxOpen()             (~PIND & 0x04)
#define  SetPortB_Input()        {DDRB = 0x00; PORTB = 0xFF;}  // Z+PUD
#define  SetPortB_Output()       {DDRB = 0xFF; PORTB = 0xFF;}  // all OUT_HIGH

#define  EnableImpulseInput()    {PORTC &= 0x40; DDRC  &= 0x40;}

#define  DisplayDataEnable()     {System.Flags.LcdUpdate = TRUE; DDRB = 0xFF; PORTB = 0x00; DDRD |= 0xE0; PORTD &= ~0xE0; PORTA &= ~0x80;}
#define  DisplayDataDisable()    {PORTB = 0x00; PORTA |= 0x80; PORTB = 0xFF; PORTD &= ~0xE0; System.Flags.LcdUpdate = FALSE;}

#define  SensorPwrEnLatch()      (PORTA &= ~0x40)
#define  SensorPwrDiLatch()      (PORTA |=  0x40)

#define  ARM_PowerEnable()       (PORTA |=  0x20)
#define  ARM_PowerDisable()      (PORTA &= ~0x20)

#define  EepromWriteEnable()     (PORTD &= ~0x10)
#define  EepromWriteDisable()    (PORTD |=  0x10)

#define  USB_StickPowerEnable()  (PORTC &= ~0x80)
#define  USB_StickPowerDisable() (PORTC |=  0x80)

//#define  USB_StickPowerEnable()  {System.Flags.LcdUpdate = TRUE; DDRB = 0xFF; PORTB = 0x04; PORTA &= ~0x40;}
//#define  USB_StickPowerDisable() {PORTB = 0x00; PORTA |= 0x40; PORTB = 0xFF; System.Flags.LcdUpdate = FALSE;}

#define  SensorPowerEnable(x)    {System.Flags.LcdUpdate = TRUE; DDRB = 0xFF; PORTB = (1 << x); PORTA &= ~0x40;}
#define  SensorPowerDisable(x)   {PORTA |= 0x40; DDRB = 0xFF; PORTB = 0xFF; System.Flags.LcdUpdate = FALSE;}

// display defines
#define  LCD_DATA                (PORTB)
#define  LCD_RS_CONTROL()        (PORTD &= ~0x20)
#define  LCD_RS_DATA()           (PORTD |=  0x20)
#define  LCD_RW_LOW()            (PORTD &= ~0x40)
#define  LCD_RW_HIGH()           (PORTD |=  0x40)
#define  LCD_E_LOW()             (PORTD &= ~0x80)
#define  LCD_E_HIGH()            (PORTD |=  0x80)


// keyboard defines
#define  KEY_UP                         0x3D
#define  KEY_DOWN                       0x2F
#define  KEY_LEFT                       0x1F
#define  KEY_RIGHT                      0x37
#define  KEY_ENTER                      0x3E
#define  KEY_ESCAPE                     0x3B

#define  KEY_NO_KEY_PRESSED             0x3F
#define  KEY_DEBOUNCE_TIME_MS           50

#define  PULSE_INPUT_DEBOUNCE_TIME_MS   50


// uart defines
#define  BAUDRATE_9600                  1
#define  BAUDRATE_115200                2

#define  UART_TX_EN                     0x08
#define  UART_RX_EN                     0x10
#define  UART_RX_INT_EN                 0x80
#define  UART_URSEL                     0x80
#define  UART_8BIT                      0x06

#define  EnableUartInterrupt()          (UCSRB |= UART_RX_INT_EN)
#define  DisableUartInterrupt()         (UCSRB &= ~UART_RX_INT_EN)




//**************** prototypes ******************//
int main(void);

void InstallInterrupts(void);

BYTE WaitEventTimeout(BYTE event, WORD timeout);


#endif

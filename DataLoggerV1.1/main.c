#include "main.h"
#include "appl.h"
#include "init.h"
#include "tools.h"
#include "usb.h"
#include "sensor.h"
#include "i2c.h"



s_usb            USB;          // struct of usb-data
s_system_error   SystemError;  // safed Errorcodes in EEPROM
s_system         System;       // global system-struct



/////////////////////////////////////////////////////////////////////////
// function : entry-point after reset -> boot system                   //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
int main (void)
{
  InitHW();
  InitTimer0();           // systemtimer with 1ms slot
  InitADC();
  InitI2C();
  InstallInterrupts();    
  InitVariables();
  LoadSensorConfig();

  EnableGlobalInterrupt();


  CheckSystemAfterPowerLost();
  Application();

  return 1;
}



/////////////////////////////////////////////////////////////////////////
// function : timer interrupt 0 -> systemtimer : called every 1ms      //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
ISR(TIMER0_OVF_vect)
{
  TCNT0 = 0xFF - 230;         // reload time for 1ms timetick
  if(TIFR & 0x02)             // this int-routine was blocked by other INT's
    TIFR |= 0x02;

  SetEvent(EVENT_1MS_TICK);

  if(System.msTimer++ > 999)  // increment systemtimer
  {
    System.msTimer = 0;
    System.secTimer++;
    SensorServiceFast();      // check if a fast-service is necessary
  }

  if(System.callbackTimer > 0)        // service callbacktimer
  { 
    System.callbackTimer--;
    if(System.callbackTimer == 0)
      SetEvent(EVENT_TIME_CALLBACK);  // set event if callback is detected
  }

  if(IsBoxOpen())            // check keys if something happens
    ScanKeyPad();

  ImpulseInputService();     // detect if pulse-input has changed
}



/////////////////////////////////////////////////////////////////////////
// function : if box is opened or closed this function is called(INT0) //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
ISR(INT0_vect)
{
  DisableBoxSwitchInt();

  if(IsBoxOpen())
    SetEvent(EVENT_BOX_OPENED)
  else
    SetEvent(EVENT_BOX_CLOSED)
}



/////////////////////////////////////////////////////////////////////////
// function : service-interruptroutine of RTC -> wakeup from sleepmode //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
ISR(INT1_vect)
{
  // wakeup system
  DisableRTC_Int();
  PowerSaveModeDisable();
  SetEvent(EVENT_RTC_INTERRUPT);
}



/////////////////////////////////////////////////////////////////////////
// function : uart receive interrupt                                   //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
ISR(USART_RXC_vect)
{
  ReceiveUSB_Byte();
}



/////////////////////////////////////////////////////////////////////////
// function : install all interal an external int-requests             //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void InstallInterrupts(void)
{
  MCUCR |=  0x01;         // INT at any change on PD2/INT0
  GICR  |=  0x40;         // enable external INT0
  GICR  |=  0x80;         // enable external INT1
  GIFR  &= ~0xE0;         // clear pending ext-int-flags
  TIMSK  =  0x01;         // enable timer0 overflow-interrupt
}



/////////////////////////////////////////////////////////////////////////
// function : wait for the given event with a given timeout            //
// given    : mask of events, timeout in ms                            //
// return   : event timeout, or given event received                   //
/////////////////////////////////////////////////////////////////////////
BYTE WaitEventTimeout (BYTE event, WORD timeout)
{
  while(timeout--)
  {
    ClearEvent(EVENT_1MS_TICK);
    while(!(System.EventID & (EVENT_1MS_TICK | event)));
    
    if(IsEventPending(event))      // if given event received -> return success
      return EVENT_RESULT_SUCCESS;
  }

  return EVENT_RESULT_TIMEOUT;
}

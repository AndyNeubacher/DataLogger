#include "main.h"
#include "init.h"
#include "i2c.h"
#include "usb.h"
#include "sensor.h"
#include "tools.h"


/////////////////////////////////////////////////////////////////////////
// function : init variables necessary for the operating-system        //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void InitVariables(void)
{
  SetPortB_Input();
  System.Key.Detected = KEY_NO_KEY_PRESSED;
  USB_FlushData();
}



/////////////////////////////////////////////////////////////////////////
// function : reload sensor-parameter from internal EEPROM             //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void LoadSensorConfig(void)
{
    memset(&Sensor, 0x00, sizeof(Sensor));          // disable all sensor-parameters
/*
  // load sensorconfig from internal EEPROM
  EEPromReadData(EEPROM_START_SENSORCONFIG, (BYTE*)(&Sensor), sizeof(s_sensor));

  if(Sensor.Nr[0].Enabled == 0xFF)    // clear sensorvariables if 0xFF by default
  {
    // write 0x00 to internal EEPROM when no sensor was configured previously
    memset((BYTE*)&Sensor, 0x00, sizeof(s_sensor));
    EEPromWriteData(EEPROM_START_SENSORCONFIG, (BYTE*)(&Sensor), sizeof(s_sensor));
  }

  Sensor.Impulse.Pulses = GetImpulseInput();
  SensorService();
*/
}



/////////////////////////////////////////////////////////////////////////
// function : safe sensor-config to internal EEPROM                    //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void SafeSensorConfig(void)
{
  EEPromWriteData(EEPROM_START_SENSORCONFIG, (BYTE*)(&Sensor), sizeof(s_sensor));
}



/////////////////////////////////////////////////////////////////////////
// function : init timer0 for system-timebase -> 1ms timeslot          //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void InitTimer0 (void)
{
   TCCR0 = 0x02;            // prescale clock by 8 -> 1.8432MHz / 8
   TCNT0 = 0xFF - 230;
   OCR0  = 0xFF - 230 - 2;  // to prevent output-compare-int-flag
}



/////////////////////////////////////////////////////////////////////////
// function : init timer1                                              //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void InitTimer1 (void)
{

}



/////////////////////////////////////////////////////////////////////////
// function : init timer2                                              //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void InitTimer2 (void)
{

}



/////////////////////////////////////////////////////////////////////////
// function : init internal ADC                                        //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void InitADC (void)
{

}



/////////////////////////////////////////////////////////////////////////
// function : init I2C interface                                       //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void InitI2C (void)
{
   TWSR &= 0x03;           // no prescaler
   TWBR =  0x00;           // fastest speed (~330kHz)
   I2C_SendStop();         // stop possible started transactions

   I2C_DisableI2C();       // stop TWI -> start is done again at communication
}



/////////////////////////////////////////////////////////////////////////
// function : init internal UART with 9600 or 115200 baud              //
// given    : baudrate                                                 //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void InitUART (BYTE baudrate)
{

   UCSRB = 0x00;      // disable uart
   UBRRH = 0x00;

   if(baudrate == BAUDRATE_9600)
      UBRRL = 11;     // 9600 baud
   else
      UBRRL = 0;      // 115200 baud

   UCSRB = (UART_RX_INT_EN | UART_RX_EN | UART_TX_EN);
   UCSRC = (UART_URSEL | UART_8BIT);
}



/////////////////////////////////////////////////////////////////////////
// function : init internal µC registers and IO-ports                  //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void InitHW (void)
{
   MCUCR   = 0x00;      // disable sleepmodes, interrupt sensitivity
   WDTCR   = 0x00;      // disable watchdog
   GICR    = 0x00;      // select int-vector table
   GIFR    = 0x00;      // clear int-flags
   SFIOR   = 0x00;      // adc-mode, portpin-pullups, timer resets

   DDRA    = 0xE0;      // portA : 0..4 tristate (Z), 5=OUT_L, 6+7 (OUT_H)
   PORTA   = 0xC0;
   DDRB    = 0x00;      // portB : 0..7 input with pullup (Z+PUD)
   PORTB   = 0xFF;
   DDRC    = 0x80;      // portC : 0..6 = Z, 7=OUT_H
   PORTC   = 0x80;
   DDRD    = 0x13;      // portD : 0+1=OUT_L, 2+3=Z+PUD, 4=OUT_H, 5+6+7=Z
   PORTD   = 0x1C;

   TCCR0   = 0x00;      // clear timer/counter0
   TCCR2   = 0x00;      // clear timer/counter2
   TIMSK   = 0x00;      // clear int-mask-register
   TIFR    = 0x00;      // clear timerflags

   TCCR1A  = 0x00;      // no output-compare1 function
   TCCR1B  = 0x00;      // no output-compare2 function

   SPCR    = 0x00;      // no spi
   SPSR    = 0x00;

   TWBR    = 0x00;      // clear I2C baudrate
   TWCR    = 0x00;      // clear I2C interrupt-flags
}

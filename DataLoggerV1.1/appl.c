#include "main.h"
#include "appl.h"
#include "init.h"
#include "lcd.h"
#include "menu.h"
#include "sensor.h"
#include "usb.h"
#include "i2c.h"
#include "tools.h"
#include "menu.h"


/////////////////////////////////////////////////////////////////////////
// function : main application -> dispense different events            //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void Application (void)
{
   BYTE i;
   CHAR tmp[9];
   
   while(1)
   {
      // event was set ....
      if(System.EventID & EVENT_BOX_OPENED)
      {
         InitLCD();                     // init display
         BoxOpenedLogData2Stick();      // safe logged measurements from external EEPROM to USb-Stick
         StartMenu();                   // display menu
         ClearEvent(EVENT_BOX_OPENED);
         GIFR |= 0x40;                  // clear int-flag
         EnableBoxSwitchInt();          // enable int by box-switch
      }


      if(System.EventID & EVENT_BOX_CLOSED)
      {
         ClearEvent(EVENT_BOX_CLOSED);
         EnableBoxSwitchInt();          // enable int by box-switch
      }


      if(System.EventID & EVENT_IMPULSE_INPUT_TRIGGERED)          // impulse-input was triggered
      {
         if(IsBoxOpen())
           PrintLCD(16,1,"*");

         ClearEvent(EVENT_IMPULSE_INPUT_TRIGGERED);
         SetTimeCallback(EVENT_CALLBACK_CLR_IMP_SIGNAL, 1000);    // clear written "*" after 1000ms
      }


      if(System.EventID & EVENT_TIME_CALLBACK)
      {
        if(System.CallbackEvent & EVENT_CALLBACK_CLR_IMP_SIGNAL)
        {
          if(IsBoxOpen())
            PrintLCD(16,1," ");                                    // clear written "*" NOW

          ClearTimeCallback(EVENT_CALLBACK_CLR_IMP_SIGNAL);
        }

        ClearEvent(EVENT_TIME_CALLBACK);
      }


      if(System.EventID & EVENT_KEY_CHANGED)                      // update menu after key pressed
      {
         DoMenu(System.Key.Valid);
         ClearEvent(EVENT_KEY_CHANGED);
      }


      if(System.EventID & EVENT_RTC_INTERRUPT)                    // event of external RTC triggered
      {
         // measure configured sensor-values + reload RTC-timer-int
         StopAlarmTimer();                   // stop timer of external RTC
         SensorService();
         ClearEvent(EVENT_RTC_INTERRUPT);
         EnableRTC_Int();
         PowerSaveModeEnable();              // set power-down-mode
      }


      if(System.EventID & EVENT_1MS_TICK)                         // 1ms timetick
      {
         ClearEvent(EVENT_1MS_TICK);
      }


      if(System.EventTimer & EVENT_FASTLOGGING_SAFE2_USB)
      {
         ClearTimerEvent(EVENT_FASTLOGGING_SAFE2_USB);
         USB_LogSensorValuesFast();
      }
      
      
      if(System.EventTimer & EVENT_UPDATE_DISPLAY_VALUE)
      {
         ClearTimerEvent(EVENT_UPDATE_DISPLAY_VALUE);
         if(IsBoxOpen())
         {
            for(i=0; i<NUM_SENSOR; i++)
            {
               if(Sensor.Nr[i].Enabled == Sensor_Enable)
               {
                  if(Sensor.Nr[i].Type == Sensor_4_20mA_FastSample)
                  {
                     if(Sensor.Nr[i].LastMeasurement > 0)
                        calcPressure(Sensor.Nr[i].LastMeasurement, &tmp[0]);
                     else
                        strcpy(&tmp[2], " -,--");
                     strcpy(&main_menu.entry[i].name[9], &tmp[2]);         // add measurement
                     main_menu.entry[i].name[8] = ' ';                     // add spaces
                     main_menu.entry[i].name[14] = 'b';                    // add unit
                     MenuUpdateDisplay();
                  }
                  
                  if(Sensor.Nr[i].Type == Sensor_Impulse)
                  {
                     Byte2AsciiDec(Sensor.Nr[i].LastMeasurement, (BYTE*)&tmp[0], UNSIGNED_BYTE);
                     strcpy(&main_menu.entry[i].name[11], &tmp[0]);
                     main_menu.entry[i].name[8] = ' ';                     // add spaces
                     main_menu.entry[i].name[9] = ' ';                     // add spaces
                     main_menu.entry[i].name[10] = ' ';                    // add spaces
                     main_menu.entry[i].name[14] = 0;                      // add spaces
                     MenuUpdateDisplay();
                  }                     
               }                  
            }
         }
      }
   }
}



/////////////////////////////////////////////////////////////////////////
// function : main application -> dispense different events            //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void BoxOpenedLogData2Stick(void)
{
  ClearScreen();
  Cursor(CURSOR_ON, CURSOR_BLINK);
  PrintLCD(1,1,STRING_SAFE_DATA_TO_STICK1);
  PrintLCD(1,2,STRING_SAFE_DATA_TO_STICK2);

  if(USB_LogSensorValues())
    PrintLCD(15,2,STRING_DONE);
  else
    PrintLCD(15,2,STRING_ERROR);

  Sleep(1000);
  Cursor(CURSOR_OFF, CURSOR_STEADY);
}

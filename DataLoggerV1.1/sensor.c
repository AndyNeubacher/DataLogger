#include "main.h"
#include "i2c.h"
#include "lcd.h"
#include "string.h"
#include "sensor.h"
#include "tools.h"
#include "usb.h"
#include "init.h"



s_sensor    Sensor;


/////////////////////////////////////////////////////////////////////////
// function : configures the type and property of the sensors          //
// given    : number of selected sensor (1...4)                        //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void SetSensorSettings(BYTE sensor_nr)
{
   //FLOAT float1, float2;
   BYTE  max=0;
   BYTE  main_selection;
   BYTE  index;
   CHAR  buf[4][16];


   for(main_selection = SENSOR_ENABLE_DISABLE;
       main_selection < SENSOR_SELECTION+1;
       main_selection++)
   {
      switch(main_selection)
      {
         // enables / disables sensor
         case SENSOR_ENABLE_DISABLE:
               max = 2;
               strcpy(&buf[0][0], STRING_SET_SENSOR_SENSOR_ONOFF);
               strcpy(&buf[Sensor_Disable][0], STRING_SET_SENSOR_DISABLE);
               strcpy(&buf[Sensor_Enable][0], STRING_SET_SENSOR_ENABLE);
            break;

         // copy strings for sensor selection
         case SENSOR_SELECTION:
               max = 1;
               strcpy(&buf[0][0], STRING_SET_SENSOR);
               //strcpy(&buf[Sensor_0_10VDC][0], STRING_MENU_SENSOR_0_10VDC);
               //strcpy(&buf[Sensor_0_20mA][0], STRING_MENU_SENSOR_0_20MA);
               strcpy(&buf[Sensor_4_20mA][0], STRING_MENU_SENSOR_4_20MA);
               if(sensor_nr == 1)   // only sensor 1 has impulse-input
               {
                  strcpy(&buf[Sensor_Impulse][0], STRING_MENU_SENSOR_IMPULSE);
                  max++;
               }
            break;

         // copy strings for sensor units
         //case SENSOR_UNITS:
         //      max = 6;
         //      strcpy(&buf[0][0], STRING_SET_UNIT);
         //      strcpy(&buf[Unit_mbar][0], STRING_SET_SENSOR_UNIT_MBAR);
         //      strcpy(&buf[Unit_bar][0], STRING_SET_SENSOR_UNIT_BAR);
         //      strcpy(&buf[Unit_C][0], STRING_SET_SENSOR_UNIT_C);
         //      strcpy(&buf[Unit_l][0], STRING_SET_SENSOR_UNIT_L);
         //      strcpy(&buf[Unit_hl][0], STRING_SET_SENSOR_UNIT_HL);
         //      strcpy(&buf[Unit_m3][0], STRING_SET_SENSOR_UNIT_M3);
         //   break;
      }


      // display texts and prompt user for input
      index = 1;
      ClearScreen();
      PrintLCD(1,1,&buf[0][0]);
      PrintLCD(1,2,&buf[1][0]);
      MoveXY(1, 2);
      Cursor(CURSOR_ON, CURSOR_BLINK);

      // promt user for setting the correct sensor
      do
      {
         while(!(System.EventID & (EVENT_KEY_CHANGED | EVENT_BOX_CLOSED | EVENT_BOX_OPENED)));

         // if box closed -> return to application without changes
         if(IsEventPending(EVENT_BOX_CLOSED | EVENT_BOX_OPENED))
            return;

         switch(System.Key.Valid)
         {
            case KEY_UP:      // select previous sensortype
                  if(index < max) index++;
               break;

            case KEY_DOWN:    // select next sensortype
                  if(index > 1) index--;
               break;
         }
         PrintLCD(1,2,&buf[index][0]);
         MoveXY(1, 2);
         ClearEvent(EVENT_KEY_CHANGED);
      }while((System.Key.Valid != KEY_ESCAPE) && (System.Key.Valid != KEY_ENTER));


      // quit without changing sensor-parameters
      if(System.Key.Valid == KEY_ESCAPE)
         return Cursor(CURSOR_OFF, CURSOR_STEADY);


      // copy values to sensor-struct
      switch(main_selection)
      {
         case SENSOR_ENABLE_DISABLE :
               Sensor.Nr[sensor_nr-1].Enabled = index;
            break;

         case SENSOR_SELECTION :
               Sensor.Nr[sensor_nr-1].Type = index;
            break;

         //case SENSOR_UNITS :
         //      Sensor.Nr[sensor_nr-1].Unit = index;
         //   break;
      }


      // check if selected sensor is disabled
      if(Sensor.Nr[sensor_nr-1].Enabled == Sensor_Disable)
         return;



   } // for "main_selection"


   // if user selected pulse-input -> display pulse settings
   if(Sensor.Nr[sensor_nr-1].Type == Sensor_Impulse)
   {
      // clear screen and prompt user for maximum value
      //ClearScreen();
      //PrintLCD(1,1,">");
      //PrintLCD(8,1,STRING_SET_PULSES);                  // print "equals"
      //PrintLCD(1,2,">00000");
      //PrintLCD(8,2,&buf[Sensor.Nr[sensor_nr-1].Unit][1]);  // print "unit"
      //Cursor(CURSOR_ON, CURSOR_BLINK);


      // prompt user for pulses
      //float1 = SetDecimalValue(1, 65535, 5, 2, 1);
      //if(float1 == 0)
        //return Cursor(CURSOR_OFF, CURSOR_STEADY);


      // prompt user -> how many pulses for xxx-value
      //float2 = SetDecimalValue(1, 65535, 5, 2, 2);
      //if(float2 == 0)
        //return Cursor(CURSOR_OFF, CURSOR_STEADY);

      // safe multiplyfactor to sensor-struct
      //Sensor.Nr[sensor_nr-1].MultiplyFactor = (float2 / float1);
      EnableImpulseInput();
   }

   SetMeasurementInterval(sensor_nr);
   SafeSensorConfig();
   Cursor(CURSOR_OFF, CURSOR_STEADY);
}



/////////////////////////////////////////////////////////////////////////
// function : set interval between two measurements                    //
// given    : number of sensor 1...4                                   //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void SetMeasurementInterval(BYTE sensor_nr)
{
  ClearScreen();
  PrintLCD(1,1,STRING_SET_SENSOR_INTERVAL);
  PrintLCD(1,2,STRING_TIME);
  PrintLCD(9,2,"00:00:00");

  Sensor.Nr[sensor_nr-1].MeasureInterval =  (((LONG)(SetDecimalValue(0, 23, 2, 9,  2))) * 60 * 60);    // get hours
  Sensor.Nr[sensor_nr-1].MeasureInterval += (((LONG)(SetDecimalValue(0, 59, 2, 12, 2))) * 60);         // get minutes
  Sensor.Nr[sensor_nr-1].MeasureInterval +=  SetDecimalValue(0, 59, 2, 15, 2);                         // get seconds
  
  if((Sensor.Nr[sensor_nr-1].Type == Sensor_4_20mA) &&            // if sensor-interval slower than 1min
     (Sensor.Nr[sensor_nr-1].MeasureInterval < 60))               //  -> switch to fastservice
     Sensor.Nr[sensor_nr-1].Type = Sensor_4_20mA_FastSample;
}



/////////////////////////////////////////////////////////////////////////
// function : checks for negative edge at PC6 -> inpulse-input         //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void ImpulseInputService(void)
{
  BYTE in;


  // only check input if sensortype == IMPULSE_INPUT + ENABLED
  if((Sensor.Nr[0].Type != Sensor_Impulse) || (Sensor.Nr[0].Enabled != Sensor_Enable))
    return;

  in = GetImpulseInput();                 // get input

  //----------------------- debounce input --------------------------//
  if(Sensor.Impulse.DebounceTimer > 0)
  {
    Sensor.Impulse.DebounceTimer--;       // decrement debouncetime
    if(Sensor.Impulse.DebounceTimer == 0)
    {
      if(Sensor.Impulse.OldValue != in)   // check if status is the same as detected
      {
        Sensor.Impulse.OldValue = in;     // store new 'old-value'

        if(Sensor.Impulse.Pulses < 0xFFFF)// prevent overflow
        {
          Sensor.Impulse.Pulses++;        // increment detected pulses
          Sensor.Nr[0].LastMeasurement = Sensor.Impulse.Pulses;
          SetEvent(EVENT_IMPULSE_INPUT_TRIGGERED);
          SetTimerEvent(EVENT_UPDATE_DISPLAY_VALUE);
        }
      }
    }

    return;                               // if debounce-timer is running -> return
  }

  if(in != Sensor.Impulse.OldValue)
  {
    //----------------------- detect edges --------------------------//
    if(in == 0)       // negative edge
      Sensor.Impulse.DebounceTimer = PULSE_INPUT_DEBOUNCE_TIME_MS;
    else              // positive edge -> store new 'old-value' for neg-edge-detection
      Sensor.Impulse.OldValue = in;
  }
}



/////////////////////////////////////////////////////////////////////////
// function : read analog value of given ADC-channel (0...3)           //
// given    : number of sensor                                         //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
WORD GetSensorAD_Value(BYTE channel)
{
  BYTE i;
  WORD mean_value = 0;

  ADMUX   = channel;                      // select channel of AD-MUX
  ADCSRA  = 0x84;                         // init AD-converter

  // get eight measurements ...
  for(i=0; i<8; i++)
  {
    ADCSRA |= 0x40;                       // start conversion
    while(!(ADCSRA & 0x10));              // wait until conversion is completed
    ADCSRA |= 0x10;                       // clear flag

    mean_value += (ADCL | (ADCH << 8));   // get value
  }

  ADCSRA = 0x00;                          // disable ADC
  ADMUX  = 0x00;

  return (mean_value / 8);                // calculate mean-value and return
}



/////////////////////////////////////////////////////////////////////////
// function : search minimumvalue of "MeasureInterval" and return sec  //
// given    : nothing                                                  //
// return   : LONG number of seconds for next wakeup/measurement       //
/////////////////////////////////////////////////////////////////////////
LONG GetNextMeasurementTime(void)
{
  BYTE i;
  LONG ret = 0xFFFFFFFF;

  for(i=0; i<NUM_SENSOR; i++)
  {
    if(Sensor.Nr[i].Enabled == Sensor_Enable)   // only get timer-reload if sensor is enabled
    {
      if(ret > Sensor.Nr[i].MeasureIntervalWorkTimer)    // get number of pending seconds for next measurement
         ret = Sensor.Nr[i].MeasureIntervalWorkTimer;    // safe minimum
    }
  }

  return ret;                                   // return smallest value
}



/////////////////////////////////////////////////////////////////////////
// function : check sensors every second                               //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void SensorServiceFast(void)
{
   BYTE i, idx, pos;

   for(i=0; i<NUM_SENSOR; i++)
   {
      // ignore if sensor is disabled
      if(Sensor.Nr[i].Enabled != Sensor_Enable)
         continue;
      
      // ignore if not fast-sampling-mode
      if(Sensor.Nr[i].Type != Sensor_4_20mA_FastSample)
         continue;
      
      // ignore if no measurement-interval is already set
      if(Sensor.Nr[i].MeasureInterval == 0)
         continue;
      
      // check if time expired -> do measurement
      if(Sensor.Nr[i].MeasureIntervalWorkTimer > 1)
         Sensor.Nr[i].MeasureIntervalWorkTimer--;
      else
      {
         Sensor.Nr[i].FastLog.pos_write++;
         if((Sensor.Nr[i].FastLog.pos_write % FAST_LOG_BUF_SIZE) == 0)
         {
            Sensor.Nr[i].FastLog.buf_idx_ready_for_usb = (Sensor.Nr[i].FastLog.buf_select & 0x01);
            SetTimerEvent(EVENT_FASTLOGGING_SAFE2_USB);
            Sensor.Nr[i].FastLog.buf_select++;
         }            

         idx = (Sensor.Nr[i].FastLog.buf_select & 0x01);                // select 1st or 2nd buffer
         pos = (Sensor.Nr[i].FastLog.pos_write & (FAST_LOG_BUF_SIZE-1));
         
         Sensor.Nr[i].FastLog.buf[idx][pos] = GetSensorAD_Value(i);           // do a measurement
         Sensor.Nr[i].LastMeasurement = Sensor.Nr[i].FastLog.buf[idx][pos];   // temp-safe for displayupdate
         Sensor.Nr[i].MeasureIntervalWorkTimer = Sensor.Nr[i].MeasureInterval;
         
         SetTimerEvent(EVENT_UPDATE_DISPLAY_VALUE);
      }
   }
}



/////////////////////////////////////////////////////////////////////////
// function : set timer/counter2 to asyncronous operation              //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void SensorService(void)
{
   BYTE i;
   LONG time = FALSE;

   //------------- re-calc the sensor-interval-times ---------------//
   for(i=0; i<NUM_SENSOR; i++)
   {
      // ignore if sensor is disabled
      if(Sensor.Nr[i].Enabled != Sensor_Enable)
         continue;
         
      if(Sensor.Nr[i].Type == Sensor_4_20mA_FastSample)  // skip (done in SensorServiceFast)
         continue;

      time = TRUE;                                      // interval still running

      // check if time expired -> do measurement
      if(Sensor.Nr[i].MeasureIntervalWorkTimer == 0)
      {
         DoSensorMeasurement(i);
         Sensor.Nr[i].MeasureIntervalWorkTimer = Sensor.Nr[i].MeasureInterval;
      }
   }

   if(!time)          // return if no sensor is active
      return;


   //------------- re-load the external RTC-counter ----------------//
   time = GetNextMeasurementTime();          // get next timer-value
   time = time / 60;    // get minutes
   if(time > 0xFF)      // prevent overflow
      time = 0xFF;

   LoadAlarmTimer((BYTE)(time), RTC_TIMER_MIN); // load external RTC-timer
   time = time * 60;                            // convert to seconds

   // recalc value for next reload
   for(i=0; i<NUM_SENSOR; i++)
      Sensor.Nr[i].MeasureIntervalWorkTimer -= time;

   StartAlarmTimer();                  // start timer of external RTC
}



/////////////////////////////////////////////////////////////////////////
// function : measure value of given sensor  (0...3)                   //
// given    : number of sensor                                         //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void DoSensorMeasurement(BYTE sensor_nr)
{
  LONG          tmp;

  // get actual system-parameters
  ReadOnboardTemp();                                      // read board-temperature
  tmp = GetSensorAD_Value(ADCHL_SUPPLY_VOLTAGE);
  System.SupplyVoltage = (WORD)((tmp * 13880) / 1024);    // calculate mV-value


  // get sensorvalues and store to external EEPROM
  if(Sensor.Nr[sensor_nr].Type == Sensor_Impulse)
  {
    LogValues2EEprom(Sensor.Impulse.Pulses, sensor_nr);         // safe data to external EEPROM
    Sensor.Impulse.Pulses = 0x00;
    Sensor.Nr[sensor_nr].LastMeasurement = 0x00;
  }
  else
  {
    tmp = GetSensorAD_Value(sensor_nr);
    Sensor.Nr[sensor_nr].LastMeasurement = tmp;
    LogValues2EEprom(tmp, sensor_nr);  // safe data to external EEPROM
  }
  
  SetTimerEvent(EVENT_UPDATE_DISPLAY_VALUE);
}



/////////////////////////////////////////////////////////////////////////
// function : safe measured values to external EEPROM  size=9Bytes     //
// given    : WORD - sensorvalue, BYTE number of sensor                //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
BYTE LogValues2EEprom(WORD val, BYTE num)
{
  union union_w_b   num_logs;
  s_eeprom_logging  eeprom_log;


  //---------------- get index where log should be safed ---------------//
  EEPROM_Bulk_Read(EXT_EEPROM_NUM_LOGS_POS, sizeof(WORD), &num_logs.b[0]);
  if(num_logs.w >= EXT_EERPOM_MAX_LOGS)             // max storagesize reached
  {
    if(USB_LogSensorValues())                       // -> safe logs to USB-Stick
      num_logs.w = 0x0000;                          //  if safed to stick, start from zero in EEPROM
  }

  //---------------- get data which should be logged ------------------//
  eeprom_log.timestamp     = EncodeSystemTime((s_time*)(&System.Time));
  eeprom_log.boardtemp     = System.BoardTemp;
  eeprom_log.supplyvoltage = System.SupplyVoltage;

  eeprom_log.sensorvalue   = ((num & 0x03) << 14);                      // bits 15 .. 14  -> number of sensor
  eeprom_log.sensorvalue  |= (((Sensor.Nr[num].Type-1) & 0x03) << 12);  // bits 13 .. 12  -> type of sensor
  eeprom_log.sensorvalue  |= (val & 0x0FFF);                            // bits 11 .. 0   -> sensorvalue

  EEPROM_Bulk_Write(EXT_EEPROM_START_OF_LOGS + (num_logs.w * sizeof(s_eeprom_logging)),
                    sizeof(s_eeprom_logging),
                    (BYTE*)(&eeprom_log));

  //---------------- write startindex of next sensor-log ---------------//
  num_logs.w++;                                   // point to start of next log
  EEPROM_Bulk_Write(EXT_EEPROM_NUM_LOGS_POS, sizeof(WORD), &num_logs.b[0]);


  return TRUE;
}


/////////////////////////////////////////////////////////////////////////
// function : set sensordefaults for "Anarehbühel"                     //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void setSensordefaultAnarehbuehel(void)
{
    Sensor.Nr[0].Enabled         = Sensor_Enable;
    Sensor.Nr[0].Type            = Sensor_Impulse;
    Sensor.Nr[0].MeasureInterval = 10*60;				// 10 minutes
}


/////////////////////////////////////////////////////////////////////////
// function : set sensordefaults for "Auli"                            //
// given    : nothing                                                  //
// return   : nothing                                                  //
/////////////////////////////////////////////////////////////////////////
void setSensordefaultAuli(void)
{
    Sensor.Nr[0].Enabled         = Sensor_Enable;
    Sensor.Nr[0].Type            = Sensor_Impulse;
    Sensor.Nr[0].MeasureInterval = 20*60;				// 20 minutes
}

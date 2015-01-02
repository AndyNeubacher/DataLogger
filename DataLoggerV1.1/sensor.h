#ifndef __SENSOR_H__
#define __SENSOR_H__



#define  ADCHL_SENSOR_1          0
#define  ADCHL_SENSOR_2          1
#define  ADCHL_SENSOR_3          2
#define  ADCHL_SENSOR_4          3
#define  ADCHL_SUPPLY_VOLTAGE    4



#define  NUM_SENSOR              4

#define  SENSOR_ENABLE_DISABLE   0
#define  SENSOR_SELECTION        1
#define  SENSOR_UNITS            2


// defines for logging measurements to external EEPROM
#define  EXT_EEPROM_SIZEOF_ONE_LOG  9
#define  EXT_EEPROM_NUM_LOGS_POS    0x0000
#define  EXT_EEPROM_START_OF_LOGS   0x0010
//#define  EXT_EERPOM_MAX_LOGS        7000
#define  EXT_EERPOM_MAX_LOGS        1000


enum
{
   Sensor_Disable = 1,
   Sensor_Enable
};

enum
{
   //Sensor_0_10VDC = 1,
   //Sensor_0_20mA,
   Sensor_4_20mA = 1,
   Sensor_Impulse,
   Sensor_4_20mA_FastSample,
};

enum
{
   Unit_mbar = 1,
   Unit_bar,
   Unit_C,
   Unit_l,
   Unit_hl,
   Unit_m3,
};


#define FAST_LOG_BUF_SIZE     16     //valid = 4,8,16,32,64,128
typedef struct  
{
   WORD buf[2][FAST_LOG_BUF_SIZE];
   BYTE buf_select;
   BYTE pos_write;
   BYTE buf_idx_ready_for_usb;
} s_fast_log;

typedef struct
{
   BYTE  Enabled;
   BYTE  Type;
   BYTE  Unit;
   LONG  MeasureInterval;
   LONG  MeasureIntervalWorkTimer;
   FLOAT MultiplyFactor;
   s_fast_log FastLog;
   WORD  LastMeasurement;
} s_sensor_config;


typedef struct
{
   LONG  timestamp;
   SBYTE boardtemp;
   WORD  supplyvoltage;
   WORD  sensorvalue;
} s_eeprom_logging;


typedef struct
{
   WORD  Pulses;
   BYTE  OldValue;
   BYTE  DebounceTimer;
} s_impulse;


typedef struct
{
   WORD              NumEEpromLoggedValues;
   s_impulse         Impulse;
   s_sensor_config   Nr[NUM_SENSOR];
} s_sensor;

extern s_sensor  Sensor;






void SetSensorSettings(BYTE sensor_nr);

void SetMeasurementInterval(BYTE sensor_nr);

void ImpulseInputService(void);

LONG GetNextMeasurementTime(void);

WORD GetSensorAD_Value(BYTE channel);

void SensorServiceFast(void);

void SensorService(void);

void DoSensorMeasurement(BYTE sensor_nr);

BYTE LogValues2EEprom(WORD val, BYTE num);

void setSensordefaultAnarehbuehel(void);

void setSensordefaultAuli(void);


#endif

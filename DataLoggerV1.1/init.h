#ifndef __INIT_H__
#define __INIT_H__



void InitVariables (void);

void LoadSensorConfig (void);

void SafeSensorConfig (void);

void InitTimer0 (void);

void InitTimer1 (void);

void InitTimer2 (void);

void InitADC (void);

void InitI2C (void);

void InitUART (BYTE baudrate);

void InitHW (void);

#endif

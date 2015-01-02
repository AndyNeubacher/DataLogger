#ifndef __USB_H__
#define __USB_H__


#define   uALFAT_EOT                0x0D

#define   ASCII_TABULATOR           0x09



#define SENSOR_1_LOG                0x01
#define SENSOR_2_LOG                0x02
#define SENSOR_3_LOG                0x04
#define SENSOR_4_LOG                0x08
#define SSETTING_LOG                0x10
#define SYSTEM_LOG                  0x20
#define SENSOR_FAST_LOG             0x40




#define IsFileOpen(file)            (File.handles_open & (file))




void ReceiveUSB_Byte(void);

void TransmitUSB_Byte(CHAR data);

void USB_TransmitString(CHAR* data);

BYTE* USB_ReceiveString(WORD timeout);

BYTE* USB_SendQuery(CHAR* string, WORD timeout);

BYTE* USB_FindStartOfRxMessage(BYTE index);

void USB_FlushData(void);

void USB_AddMsg2TxBuffer(CHAR* data);

BYTE USB_GetMsgErrorcode (WORD timout);

BYTE USB_Error(WORD error);



BYTE InitUSB_Device(void);

void StopUSB_Device(void);

BYTE USB_GetArmVersion(void);

BYTE USB_UpdateUALFAT_firmware(void);

BYTE USB_SwitchBaudrate (void);

void USB_SetUALFAT_RTC(void);


BYTE OpenFile(BYTE file);

BYTE CloseFile(BYTE file);

BYTE USB_WriteBuffer2File(BYTE filehandle);


BYTE USB_LogSensorSettings(void);

BYTE USB_LogSensorValues(void);

BYTE LogValues_USB(void);

void USB_LogSensorValuesFast(void);

BYTE LogValuesFast_USB(BYTE chl);


#endif

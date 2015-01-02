#ifndef __ERRORCODE_H__
#define __ERRORCODE_H__


#define ERROR_NO_ERROR                                    0x0000


// errorcodes for RTC
#define ERROR_RTC_WRITE_NO_STARTCOND_SENT                 0x0101
#define ERROR_RTC_WRITE_NO_ADDR_ACK_RECEIVED              0x0102
#define ERROR_RTC_WRITE_NO_DATA_ACK_RECEIVED              0x0103

#define ERROR_RTC_READ_NO_STARTCOND_SENT                  0x0111
#define ERROR_RTC_READ_NO_ADDR_ACK_RECEIVED               0x0112
#define ERROR_RTC_READ_NO_DATA_ACK_RECEIVED               0x0113
#define ERROR_RTC_READ_NO_ADDR_ACK_AGAIN_RECEIVED         0x0114



// errorcodes for external EEPROM
#define ERROR_EEPROM_WRITE_NO_STARTCOND_SENT              0x0121
#define ERROR_EEPROM_WRITE_NO_ADDR_ACK_RECEIVED           0x0122
#define ERROR_EEPROM_WRITE_NO_DATA_ACK_RECEIVED           0x0123

#define ERROR_EEPROM_READ_NO_STARTCOND_SENT               0x0131
#define ERROR_EEPROM_READ_NO_ADDR_ACK_RECEIVED            0x0132
#define ERROR_EEPROM_READ_NO_DATA_ACK_RECEIVED            0x0133
#define ERROR_EEPROM_READ_NO_ADDR_ACK_AGAIN_RECEIVED      0x0134



// errorcodes for onboard temperature-sensor
#define ERROR_BOARDTEMP_WRITE_NO_STARTCOND_SENT           0x0141
#define ERROR_BOARDTEMP_WRITE_NO_ADDR_ACK_RECEIVED        0x0142
#define ERROR_BOARDTEMP_WRITE_NO_DATA_ACK_RECEIVED        0x0143

#define ERROR_BOARDTEMP_READ_NO_STARTCOND_SENT            0x0151
#define ERROR_BOARDTEMP_READ_NO_ADDR_ACK_RECEIVED         0x0152
#define ERROR_BOARDTEMP_READ_NO_DATA_ACK_RECEIVED         0x0153
#define ERROR_BOARDTEMP_READ_NO_ADDR_ACK_AGAIN_RECEIVED   0x0154



// errorcodes for ARM-controller (USB)
#define ERROR_ARM_NOT_RESPONDING_AFTER_POWER_UP           0x0201
#define ERROR_ARM_NO_CORRECT_VERSION_RECEIVED             0x0202

#define ERROR_ARM_NO_ERRORCODE_NULL_RECEIVED              0x0203
#define ERROR_ARM_NO_VERSION_RECEIVED                     0x0204
#define ERROR_ARM_ERROR_SWITCHING_BAUDRATE                0x0205

#define ERROR_ARM_SSETTING_FOPEN_ERROR                    0x0207
#define ERROR_ARM_SSETTING_FCLOSE_ERROR                   0x0208
#define ERROR_ARM_SSETTING_FAPPEND_ERROR                  0x0209

#define ERROR_ARM_DOLOG_FOPEN_ERROR                       0x020A
#define ERROR_ARM_DOLOG_FCLOSE_ERROR                      0x020B
#define ERROR_ARM_DOLOG_FAPPEND_ERROR                     0x020C



#define ERROR_ARM_CANT_MOUNT_USB_STICK                    0x0285
#define ERROR_ARM_INIT_FAILED                             0x0286




// errorcodes of uALFAT -> 0x03xx  ... xx  is inserted by uALFAT
#define ERROR_ARM_ERRORS                                  0x0300



// errorcode of system
#define ERROR_SYSTEM                                      0x9900

#endif

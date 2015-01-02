#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

//#define __LANG_ENGLISH__
#define __LANG_GERMAN__


// makro to store and get the strings out of the flash-memory
#define f_str(x)                            getFlashStr(PSTR(x))





#define STRING_SET_SENSOR_UNIT_MBAR         f_str(">mbar")
#define STRING_SET_SENSOR_UNIT_BAR          f_str(">bar ")
#define STRING_SET_SENSOR_UNIT_C            f_str(">°C  ")
#define STRING_SET_SENSOR_UNIT_L            f_str(">l   ")
#define STRING_SET_SENSOR_UNIT_HL           f_str(">hl  ")
#define STRING_SET_SENSOR_UNIT_M3           f_str(">m3  ")

#define STRING_DONE                         f_str("OK")
#define STRING_ERROR                        f_str("ER")


#ifdef  __LANG_ENGLISH__          //   ################

// text for SetDateTime-function
#define STRING_DATE                         f_str("date :")
#define STRING_TIME                         f_str("time :")

// text for main-menu
//#define STRING_MENU_FW_UPDATE               "firmware update"
#define STRING_MENU_UALFAT_FW_UPDATE        "uALFAT update"
#define STRING_MENU_UALFAT_VERSION          "uALFAT version"
#define STRING_MENU_USB_INIT_FAILED         f_str("com failed!")
#define STRING_MENU_SYSTEM_SETTINGS         "system settings"
#define STRING_MENU_SET_DATETIME            "set date&time"
#define STRING_MENU_DISPLAY_VERSION         "show version"
#define STRING_MENU_SENSOR_1                "sensor 1"
#define STRING_MENU_SENSOR_2                "sensor 2"
#define STRING_MENU_SENSOR_3                "sensor 3"
#define STRING_MENU_SENSOR_4                "sensor 4"

// text for sensor-configuration
#define STRING_SET_SENSOR                   f_str("set sensortype :")
#define STRING_SET_UNIT                     f_str("set unit :")
#define STRING_SET_PULSES                   f_str("pulses =")

#define STRING_SET_SENSOR_SENSOR_ONOFF      f_str("sensor :")
#define STRING_SET_SENSOR_ENABLE            f_str("enable ")
#define STRING_SET_SENSOR_DISABLE           f_str("disable")
#define STRING_SET_SENSOR_NOT_SET           f_str("not set")

#define STRING_SET_SENSOR_INTERVAL          f_str("measure interval")


#define STRING_MENU_SENSOR_IMPULSE          f_str(">impulse input")
#define STRING_MENU_SENSOR_0_10VDC          f_str(">0...10VDC    ")
#define STRING_MENU_SENSOR_0_20MA           f_str(">0...20mA     ")
#define STRING_MENU_SENSOR_4_20MA           f_str(">4...20mA     ")
#define STRING_MENU_SENSOR_0_20MA_FAST      f_str(">0...20mA fast")



#define STRING_MENU_ERASE_SENSORLOG         "erase measures"


// display-msg : "safe data to usb-stick"
#define STRING_SAFE_DATA_TO_STICK1          f_str("safe sensordata")
#define STRING_SAFE_DATA_TO_STICK2          f_str(" to USB-stick...")


// text for display error logging
#define STRING_MENU_ERROR                   f_str("errors")
#define STRING_MENU_SHOW_ERRORLOG           "show errorlog"
#define STRING_MENU_ERASE_ERRORLOG          "erase errors"

#define STRING_ERRORLOGGING_ERROR           f_str("error")
#define STRING_ERROR_ERASE_1ST              f_str("   erase all")
#define STRING_ERROR_ERASE_2ND              f_str(" errors ...")
#define STRING_ERASE_SENSORLOG              f_str(" logs ...")

#define STRING_SAFE_PARAMETERS_1ST          f_str("storing")
#define STRING_SAFE_PARAMETERS_2ND          f_str("parameters ...")
#define STRING_DEFAULTS                     f_str("defaults ...")


// sensor-defaults-text
#define STRING_MENU_SENSOR_PROFILES         "sensor profile"
#define STRING_SENSOR_DEFAULT_ANAREHBUEHEL  "Anarehla l/s"
#define STRING_SENSOR_DEFAULT_AULI          "Auli l/s"

#define STRING_SENSOR_SET_PROFILE_ANAREH    f_str("set Anarehla")
#define STRING_SENSOR_SET_PROFILE_AULI      f_str("set Auli")

#endif // __LANG_ENGLISH__





#ifdef  __LANG_GERMAN__           //   ################

// text für Funktion : "Datum + Uhrzeit setzen"
#define STRING_DATE                         f_str("Datum :")
#define STRING_TIME                         f_str("Zeit  :")

// text für hauptmenü
//#define STRING_MENU_FW_UPDATE               "Firmware Update"
#define STRING_MENU_UALFAT_FW_UPDATE        "uALFAT Update"
#define STRING_MENU_UALFAT_VERSION          "uALFAT Version"
#define STRING_MENU_USB_INIT_FAILED         f_str("ComFehler!")
#define STRING_MENU_SYSTEM_SETTINGS         "Systemkonfig."
#define STRING_MENU_SET_DATETIME            "Systemzeit"
#define STRING_MENU_DISPLAY_VERSION         "Version"
#define STRING_MENU_SENSOR_1                "Sensor 1"
#define STRING_MENU_SENSOR_2                "Sensor 2"
#define STRING_MENU_SENSOR_3                "Sensor 3"
#define STRING_MENU_SENSOR_4                "Sensor 4"

// text for sensor-configuration
#define STRING_SET_SENSOR                   f_str("Sensortype :")
#define STRING_SET_UNIT                     f_str("Einheit :")
#define STRING_SET_PULSES                   f_str("entspr. =>")


#define STRING_SET_SENSOR_SENSOR_ONOFF      f_str("Sensor :")
#define STRING_SET_SENSOR_ENABLE            f_str("ein")
#define STRING_SET_SENSOR_DISABLE           f_str("aus")
#define STRING_SET_SENSOR_NOT_SET           f_str("nicht konfiguriert")

#define STRING_SET_SENSOR_INTERVAL          f_str("setze Interval")

#define STRING_MENU_SENSOR_IMPULSE          f_str(">Impuls Eingang")
#define STRING_MENU_SENSOR_0_10VDC          f_str(">0...10VDC     ")
#define STRING_MENU_SENSOR_0_20MA           f_str(">0...20mA      ")
#define STRING_MENU_SENSOR_4_20MA           f_str(">4...20mA      ")
#define STRING_MENU_SENSOR_0_20MA_FAST      f_str(">0...20mA  fast")


#define STRING_MENU_ERASE_SENSORLOG         "Messungen losch"


// display-msg : "safe data to usb-stick"
#define STRING_SAFE_DATA_TO_STICK1          f_str("Sensordaten auf")
#define STRING_SAFE_DATA_TO_STICK2          f_str("USB sichern ...")


// text zur anzeige der fehler-aufzeichung
#define STRING_MENU_ERROR                   f_str("FEHLER")
#define STRING_MENU_SHOW_ERRORLOG           "Fehlermeldungen"
#define STRING_MENU_ERASE_ERRORLOG          "Fehler loeschen"

#define STRING_SAFE_PARAMETERS_1ST          f_str("Sicherung der")
#define STRING_SAFE_PARAMETERS_2ND          f_str("Parameter ...")
#define STRING_DEFAULTS                     f_str("Defaults ...")

#define STRING_ERRORLOGGING_ERROR           f_str("Fehler")
#define STRING_ERROR_ERASE_1ST              f_str(" loesche alle")
#define STRING_ERROR_ERASE_2ND              f_str(" Fehler ...")
#define STRING_ERASE_SENSORLOG              f_str("Messungen..")

// sensor-defaults-text
#define STRING_MENU_SENSOR_PROFILES         "Sensor Profile"
#define STRING_SENSOR_DEFAULT_ANAREHBUEHEL  "Anarehla l/s"
#define STRING_SENSOR_DEFAULT_AULI          "Auli l/s"

#define STRING_SENSOR_SET_PROFILE_ANAREH    f_str("setzte Anarehla")
#define STRING_SENSOR_SET_PROFILE_AULI      f_str("setzte Auli")


#endif // __LANG_GERMAN__



#endif

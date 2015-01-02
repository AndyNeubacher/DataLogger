#include "main.h"
#include "menu.h"
#include "lcd.h"
#include "language.h"
#include "tools.h"
#include "errorcodes.h"
#include "sensor.h"
#include "usb.h"
#include "i2c.h"


/*
In defining menus, main important things:

- top_entry/current_entry should usually be 0 (although they will be changed
   w/user interaction to match the most recently used values, so if a user
   enters a submenu and returns, the value will be preserved).

- flags for entries are:
   MENU_FLAG_HIDDEN - this can be set to indicate an entry that is
   conditionally available in a menu, but is not displayed or selectable
   currently.
   MENU_FLAG_SUBMENU - this means that when the item is selected, instead
   of calling the select function, it calls menu_enter with the contents
   of the "value" data element.  "value" should be a pointer to the submenu
   for that selection (see the definition of main_menu below for example).
   For submenus, the value of the "select" data item is ignored.

- for regular entries, "select" should point to a function that takes
   2 arguments and returns nothing.  The arguments passed to it are the
   contents of the "value" data element, and the name of the current
   menu entry.  This allows the same function to be used for multiple
   menu items, with different actions based on the context.  It is not
   necessary to do anything with these items, but they're sometimes
   useful.

- the current menu_context gives the bounds for the current menu, so you
   can control the size of a display/the areas on a display that the menu
   is allowed to be written to.
   menu routines do not store contents below a given menu; if you want
   open/close window functionality, the caller must save the contents of
   display memory where the menu is being placed and restore when the
   use of the menu has finished (note: menu_exit the way it's coded
   does not exit a top menu; this can be modified in the source to allow
   some kind of flag to be set, etc. if a user tries to exit the topmost
   menu, or other things may be done to handle this.  this is to make
   control loops easier to implement.)

- the code is small and relatively clever while being mostly pretty easy
   to use.  but it is a good idea to try to understand what's going on
   in the menuing code -- it's pretty simple pointer tricks mostly, which
   allow one to navigate a pretty large menuing system from a simple,
   small loop.  as long as menus/entries are set up properly and the
   hardware interface is configured right, things should work pretty
   smoothly and without needing much hacking.  but to understand the full
   range of what's possible with what's given here, at least a rudimentary
   understanding of the base code is necessary.

- config options:
    CONFIG_TINYMENU_HAS_INVERSE - indicates that the tinymenu_hw.h has
      definitions to allow switching to/from inverse text for highlighting
      the currently selected menu entry (otherwise an asterisk is placed
      on the display before the entry, taking up 1 more text position,
      to indicate currently selected menu entry)
    CONFIG_TINYMENU_USE_CLEAR - indicates that tinymenu_hw.h has a
      definition of a display clear routine, *and* it's ok to use that
      routine to clear the menu workspace.  this may not be desired in
      applications that use small menus on a larger workspace; you may
      wish to only affect the menu area of a display, and not clear the
      whole display.  alternatively, the menu area not containing menu
      text is filled with spaces.
    CONFIG_TINYMENU_COMPACT - disable use of HIDDEN menu entries, decrease
      compiled size of menuing system by a little bit (in most cases
      they're probably not needed).
*/



/********************************************************************/
menu_t sub_system_settings =
{
   .top_entry = 0,
   .current_entry = 0,
   .entry =
   {
      // edit date and time
      {.flags = 0,
       .select = m_select_set_date_time,
       .name = STRING_MENU_SET_DATETIME,
       .value = 0,
      },
      // display version of firmware and hardware
      {.flags = 0,
       .select = m_select_display_version,
       .name = STRING_MENU_DISPLAY_VERSION,
       .value = 0,
      },
      // display version of uALFAT
      {.flags = 0,
       .select = m_select_display_uALFAT_version,
       .name = STRING_MENU_UALFAT_VERSION,
       .value = 0,
      },
      // load new firmware of uALFAT from USB-stick
      {.flags = 0,
       .select = m_select_uALFAT_update_firmware,
       .name = STRING_MENU_UALFAT_FW_UPDATE,
       .value = 0,
      },
      // load new datalogger-firmware
      //{.flags = 0,
      //   .select = m_select_update_firmware,
      //   .name = STRING_MENU_FW_UPDATE,
      //   .value = 0,
      //},
      // show error-logs
      {.flags = 0,
       .select = m_select_show_errorlog,
       .name = STRING_MENU_SHOW_ERRORLOG,
       .value = 0,
      },
      // erase errorlogs
      {.flags = 0,
       .select = m_select_erase_errorlog,
       .name = STRING_MENU_ERASE_ERRORLOG,
       .value = 0,
      },
      // erase sensorlogs
      {.flags = 0,
       .select = m_select_erase_sensorlog,
       .name = STRING_MENU_ERASE_SENSORLOG,
       .value = 0,
      },

   },
   .num_entries = 7,
   .previous = NULL,
};


#define SENSOR_DEFAULT_AULI         0
#define SENSOR_DEFAULT_ANAREHLA     1
BYTE default_idx[2] = {SENSOR_DEFAULT_AULI, SENSOR_DEFAULT_ANAREHLA};
menu_t sub_sensor_profiles =
{
    .top_entry = 0,
    .current_entry = 0,
    .entry =
    {
        // set profile for "Durchflussmenge Anarehla"
        {.flags = 0,
         .select = m_select_sensor_profile,
         .name = STRING_SENSOR_DEFAULT_ANAREHBUEHEL,
         .value = &default_idx[SENSOR_DEFAULT_ANAREHLA],
        },
        // set profile for "Durchfulssmenge Auli"
        {.flags = 0,
         .select = m_select_sensor_profile,
         .name = STRING_SENSOR_DEFAULT_AULI,
         .value = &default_idx[SENSOR_DEFAULT_AULI],
        },
    },
    .num_entries = 2,
    .previous = NULL,
};


BYTE sensor_nr[4] = {1,2,3,4};
menu_t main_menu =
{
   .top_entry = 0,
   .current_entry = 0,
   .entry =
   {
      // config sensor1
      {.flags = 0,
       .select = m_select_set_sensors,
       .name = STRING_MENU_SENSOR_1,
       .value = &sensor_nr[0],
      },
      // config sensor2
      {.flags = 0,
       .select = m_select_set_sensors,
       .name = STRING_MENU_SENSOR_2,
       .value = &sensor_nr[1],
      },
      // config sensor3
      {.flags = 0,
       .select = m_select_set_sensors,
       .name = STRING_MENU_SENSOR_3,
       .value = &sensor_nr[2],
      },
      // config sensor4
      {.flags = 0,
       .select = m_select_set_sensors,
       .name = STRING_MENU_SENSOR_4,
       .value = &sensor_nr[3],
      },
      // display sensor profiles
      {.flags = MENU_FLAG_SUBMENU,
       .select = NULL,
       .name = STRING_MENU_SENSOR_PROFILES,
       .value = &sub_sensor_profiles,
      },
      // display error-sub-menu
      {.flags = MENU_FLAG_SUBMENU,
       .select = NULL,
       .name = STRING_MENU_SYSTEM_SETTINGS,
       .value = &sub_system_settings,
      },
   },
   .num_entries = 6,
   .previous = NULL,
};


menu_context_t menu_context =
{
   .x_loc = 0,
   .y_loc = 0,
   .height = 2,
   .width = 16,
   .menu = NULL,
};

/********************************************************************/





/////////////////////////////////////////////////////////////////////////
// function : configure sensor-parameters                              //
/////////////////////////////////////////////////////////////////////////
void m_select_set_sensors(void *arg, char *name)
{
   BYTE result;
   
   ClearEvent(EVENT_KEY_CHANGED);
   SetSensorSettings((BYTE)(*(BYTE*)(arg)));    // configure sensor-settings

   result = USB_LogSensorSettings();            // log settings to usb-stick
   ClearScreen();
   PrintLCD(1,1, STRING_SAFE_PARAMETERS_1ST);
   PrintLCD(1,2, STRING_SAFE_PARAMETERS_2ND);
   if(result)
      PrintLCD(15,2,STRING_DONE);
   else
      PrintLCD(15,2,STRING_ERROR);

   SensorService();
   Sleep(1000);      
}


/////////////////////////////////////////////////////////////////////////
// function : set date and time of external RTC                        //
/////////////////////////////////////////////////////////////////////////
void m_select_set_date_time(void *arg, char *name)
{
   ClearEvent(EVENT_KEY_CHANGED);
   SetDateTime();
   ClearEvent(EVENT_KEY_CHANGED);
}


/////////////////////////////////////////////////////////////////////////
// function : display actual versionstring of datalogger               //
/////////////////////////////////////////////////////////////////////////
void m_select_display_version(void *arg, char *name)
{
   ClearEvent(EVENT_KEY_CHANGED);
   DisplayRevisionString();
   while(!(System.EventID & (EVENT_KEY_CHANGED | EVENT_BOX_CLOSED)));
   ClearEvent(EVENT_KEY_CHANGED);
}


/////////////////////////////////////////////////////////////////////////
// function : display version of uALFAT                                //
/////////////////////////////////////////////////////////////////////////
void m_select_display_uALFAT_version(void *arg, char *name)
{
  BYTE result;
  
  result = InitUSB_Device();     // open connection to uALFAT and get version
  StopUSB_Device();     // shut down connection and power-off

  ClearScreen();
  PrintLCD(1,1,STRING_MENU_UALFAT_VERSION);
  PrintLCD(1,2,"-->");
  
  if(result == TRUE)
    PrintLCD(5,2,&USB.uALFAT_version[0]);
  else
    PrintLCD(5,2,STRING_MENU_USB_INIT_FAILED);

  ClearEvent(EVENT_KEY_CHANGED);
  while(!(System.EventID & (EVENT_KEY_CHANGED | EVENT_BOX_CLOSED)));
  ClearEvent(EVENT_KEY_CHANGED);                     // clear key event
}


/////////////////////////////////////////////////////////////////////////
// function : update firmware of uALFAT                                //
/////////////////////////////////////////////////////////////////////////
void m_select_uALFAT_update_firmware(void *arg, char *name)
{
  ClearScreen();
  PrintLCD(1,1,STRING_MENU_UALFAT_FW_UPDATE);
  PrintLCD(1,2,"-->");

  if(USB_UpdateUALFAT_firmware())
    PrintLCD(5,2,STRING_DONE);           // succesfull
  else
    PrintLCD(5,2,STRING_ERRORLOGGING_ERROR);         // error

  ClearEvent(EVENT_KEY_CHANGED);
  while(!(System.EventID & (EVENT_KEY_CHANGED | EVENT_BOX_CLOSED)));
  ClearEvent(EVENT_KEY_CHANGED);                     // clear key event
}


/////////////////////////////////////////////////////////////////////////
// function : reset to bootloader                                      //
/////////////////////////////////////////////////////////////////////////
//void m_select_update_firmware(void *arg, char *name)
//{
//   soft_reset();
//}

/////////////////////////////////////////////////////////////////////////
// function : display errorlogs                                        //
/////////////////////////////////////////////////////////////////////////
void m_select_show_errorlog(void *arg, char *name)
{
   ClearEvent(EVENT_KEY_CHANGED);
   DisplayErrorLogging();
   ClearEvent(EVENT_KEY_CHANGED);
}


/////////////////////////////////////////////////////////////////////////
// function : erase all safed errors                                   //
/////////////////////////////////////////////////////////////////////////
void m_select_erase_errorlog(void *arg, char *name)
{
   ClearEvent(EVENT_KEY_CHANGED);
   ClearScreen();
   PrintLCD(1,1,STRING_ERROR_ERASE_1ST);              // show "erase errors"
   PrintLCD(1,2,STRING_ERROR_ERASE_2ND);

   SystemError.len  = 0x00;                           // fill error-struct with 0
   EEPromWriteByte(EEPROM_START_ERRORLOGGING, SystemError.len);   // write index
   EEPromWriteByte(EEPROM_START_ERRORLOGGING + 1, 0); // clear
   EEPromWriteByte(EEPROM_START_ERRORLOGGING + 2, 0); //  first
   EEPromWriteByte(EEPROM_START_ERRORLOGGING + 3, 0); //   two
   EEPromWriteByte(EEPROM_START_ERRORLOGGING + 4, 0); //    errors

   Sleep(500);                                        // wait 500ms
   PrintLCD(13,2,STRING_DONE);
   while(!(System.EventID & (EVENT_KEY_CHANGED | EVENT_BOX_CLOSED)));
   ClearEvent(EVENT_KEY_CHANGED);                     // clear key event
}


/////////////////////////////////////////////////////////////////////////
// function : erase all safed sensorlogs                               //
/////////////////////////////////////////////////////////////////////////
void m_select_erase_sensorlog(void *arg, char *name)
{
   BYTE x[2] = {0,0};

   ClearEvent(EVENT_KEY_CHANGED);
   ClearScreen();
   PrintLCD(1,1,STRING_ERROR_ERASE_1ST);              // show "erase measures"
   PrintLCD(1,2,STRING_ERASE_SENSORLOG);

   EEPROM_Bulk_Write(EXT_EEPROM_NUM_LOGS_POS, 2, &x[0]); // erase logging-index

   Sleep(500);                                        // wait 500ms
   PrintLCD(13,2,STRING_DONE);
   while(!(System.EventID & (EVENT_KEY_CHANGED | EVENT_BOX_CLOSED)));
   ClearEvent(EVENT_KEY_CHANGED);                     // clear key event
}


/////////////////////////////////////////////////////////////////////////
// function : set sensordefaults for Anarehla                          //
/////////////////////////////////////////////////////////////////////////
void m_select_sensor_profile(void *arg, char *name)
{
   BYTE default_type = (BYTE)(*(BYTE*)(arg));
   
   ClearEvent(EVENT_KEY_CHANGED);
   ClearScreen();
   
   switch(default_type)
   {
       case SENSOR_DEFAULT_ANAREHLA:
            PrintLCD(1,1,STRING_SENSOR_SET_PROFILE_ANAREH);
            setSensordefaultAnarehbuehel();
          break;
       
       case SENSOR_DEFAULT_AULI:
            PrintLCD(1,1,STRING_SENSOR_SET_PROFILE_AULI);
            setSensordefaultAuli();
          break;
   }
   PrintLCD(1,2,STRING_DEFAULTS);

   if(USB_LogSensorSettings())                        // log settings to usb-stick
      PrintLCD(14,2,STRING_DONE);
   else
      PrintLCD(14,2,STRING_ERROR);

   SensorService();
   while(!(System.EventID & (EVENT_KEY_CHANGED | EVENT_BOX_CLOSED)));
   ClearEvent(EVENT_KEY_CHANGED);                     // clear key event
}



/********************************************************************/
/******************* M E N U   F U N C T I O N S ********************/
/********************************************************************/

/*
 * Execute function for currently selected menu entry (or if it's a submenu,
 *  enter the submenu)
 *
 */
void menu_select(menu_context_t *context)
{
	menu_entry_t *entry;


	entry = &context->menu->entry[context->menu->current_entry];

	if (entry->flags & MENU_FLAG_SUBMENU) {
		// Submenu -- enter it

		menu_enter(context, (menu_t *)entry->value);
	} else {
		// Regular entry -- execute function

		entry->select(entry->value, entry->name);
	}

	// Re-display menu on return

	menu_display(context);
}


/*
 * Print an entry in a menu on display; selected specifies whether it's the
 *  currently highlighted entry (in which case it should be made inverse, or
 *  have an asterisk prepended to let user know it's "highlighted")
 */
static void menu_print_entry(menu_entry_t *entry, BYTE max_width, BYTE selected)
{
	BYTE i;


#ifdef CONFIG_TINYMENU_HAS_INVERSE
	if (selected)
		menu_set_inverse();
#else
	// No inverse; do workaround

	max_width--;
	if (selected) {
		menu_putchar('>');
	} else {
		menu_putchar(' ');
	}
#endif

	// Print the characters in the name; fill out to width with
	//  spaces (mainly for inverse)

	for (i = 0; i < max_width; i++) {
		if (!entry->name[i])
			break;

		menu_putchar(entry->name[i]);
	}

	for (; i < max_width; i++)
		menu_putchar(' ');

#ifdef CONFIG_TINYMENU_HAS_INVERSE
	// Restore non-inverse printing

	menu_set_normal();
#endif
}



/*
 * Display the current menu in the context
 */
void menu_display(menu_context_t *context)
{
	BYTE i;
	menu_t *menu = context->menu;
	menu_entry_t *disp_entry;
	BYTE dindex = 0;

#ifndef CONFIG_TINYMENU_USE_CLEAR
	BYTE j;
#else

	menu_clear();
#endif

	// Display only those entries that will fit on the display

	for (i = 0; i < context->height; i++) {

#ifndef CONFIG_TINYMENU_COMPACT
		// Don't display hidden menu entries

		do {
			disp_entry = &menu->entry[menu->top_entry + dindex];
			if (dindex++ >= menu->num_entries - menu->top_entry)
				goto entries_done;
		} while (disp_entry->flags & MENU_FLAG_HIDDEN);

#else
		disp_entry = &menu->entry[menu->top_entry + dindex];
		if (dindex++ >= menu->num_entries - menu->top_entry)
			return;
#endif

		// Go to correct x,y locations and print the entry

		menu_set_pos(context->x_loc, context->y_loc + i);
		menu_print_entry(disp_entry, context->width,
		  (menu->current_entry + 1 == dindex + menu->top_entry));
	}

#ifndef CONFIG_TINYMENU_COMPACT
entries_done:
#endif

#ifndef CONFIG_TINYMENU_USE_CLEAR
	// Fill rest of menu screen space with spaces

	for (; i < context->height; i++) {
		menu_set_pos(context->x_loc, context->y_loc + i);
		for (j = 0; j < context->width; j++) {
			menu_putchar(' ');
		}
	}
#endif
}



/*
 * Move down currently highlighted to next entry, without going out of bounds.
 *  Also adjust current top entry in display if needed to fit new entry
 *  on display
 */
void menu_next_entry(menu_context_t *context)
{
	menu_t *menu = context->menu;
	BYTE new_entry = menu->current_entry;


#ifndef CONFIG_TINYMENU_COMPACT
	while(1) {
		if (++new_entry >= menu->num_entries) // watch bounds
			return;
		if (!(menu->entry[new_entry].flags & MENU_FLAG_HIDDEN))
			break;
	}
#else
	if (++new_entry >= menu->num_entries)
		return;
#endif

	menu->current_entry = new_entry;

	if (menu->current_entry >= menu->top_entry + context->height)
		menu->top_entry = menu->current_entry - context->height + 1;

	menu_display(context);
}


/*
 * Move up currently highlighted to previous entry, without going out of
 *  bounds.  Also adjust current top entry in display if needed to fit new
 *  entry on display.
 */
void menu_prev_entry(menu_context_t *context)
{
	menu_t *menu = context->menu;
	BYTE new_entry = menu->current_entry;


#ifndef CONFIG_TINYMENU_COMPACT
	while(1) {
		if (new_entry-- == 0) // Watch bounds
			return;

		if (!(menu->entry[new_entry].flags & MENU_FLAG_HIDDEN))
			break;
	}
#else
	if (new_entry-- == 0)
		return;
#endif

	menu->current_entry = new_entry;

	if (menu->current_entry < menu->top_entry)
		menu->top_entry = menu->current_entry;

	menu_display(context);
}



/*
 * Exit a menu (go to the previous menu) -- if there is no previous
 *  menu, don't do anything.
 */
void menu_exit(menu_context_t *context)
{
	if (context->menu->previous) {
		context->menu = context->menu->previous;
		menu_display(context);
	}
}


/*
 * Enter a menu -- save current menu in the menu's previous pointer
 *  so when we exit we can go back, and update the menu context to
 *  reflect new menu... then display it.
 */
void menu_enter(menu_context_t *context, menu_t *menu)
{
	menu->previous = context->menu;
	context->menu = menu;
	menu_display(context);
}

void MenuUpdateDisplay(void)
{
   menu_display(&menu_context);
}

void MenuPrev(void)
{
   menu_prev_entry(&menu_context);
}

void MenuNext(void)
{
   menu_next_entry(&menu_context);
}

void MenuExit(void)
{
   menu_exit(&menu_context);
}

void MenuSelect(void)
{
   menu_select(&menu_context);
}

void StartMenu(void)
{
   main_menu.top_entry = 0;               // set index to top
   main_menu.current_entry = 0;           // set curent index to top
   menu_enter(&menu_context, &main_menu);
}


void DoMenu(BYTE key)
{
   switch(key)
   {
      case KEY_DOWN :   MenuNext();
         break;

      case KEY_UP :     MenuPrev();
         break;

      case KEY_LEFT :   MenuExit();
         break;

      case KEY_RIGHT :
      case KEY_ENTER :  MenuSelect();
         break;

      case KEY_ESCAPE : MenuExit();
         break;
   }
}



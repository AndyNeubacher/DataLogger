#ifndef __MENU_H__
#define __MENU_H__


/*
 * What you need to do:
 *  - change #includes to use your LCD/display include files
 *  - change the macros to match your LCD/display functions
 *
 * #definable options:
 *
 *   CONFIG_TINYMENU_USE_CLEAR -- clear display each time a menu is displayed
 *    (alternative fills up any menu area without entries with spaces)
 *
 *   CONFIG_TINYMENU_HAS_INVERSE -- if your display routines include inverse
 *    display functions, set this and set menu_set_inverse and menu_set_normal
 *    to call your LCD's inverse/normal functions.  Otherwise entries are
 *    all displayed in normal characters, with an asterisk before the
 *    currently "highlighted" entry.
 */



//Set if your LCD has a routine to set inverse printing
//#define CONFIG_TINYMENU_HAS_INVERSE
#ifdef CONFIG_TINYMENU_HAS_INVERSE
# define menu_set_inverse()    lcd_set_mode(LCD_MODE_COPY_INVERSE)
# define menu_set_normal()     lcd_set_mode(0)
#endif


//#define CONFIG_TINYMENU_USE_CLEAR
#ifdef CONFIG_TINYMENU_USE_CLEAR
# define menu_clear()            ClearScreen()
#endif


// Display routine to go to an (x,y) position
//#define menu_set_pos(x,y)      lcd_set_pos((x) * 6, (y) * 6)
#define menu_set_pos(x,y)        MoveXY((x+1),(y+1))

// Display routine to output a character
#define menu_putchar(x)          write_char(x)




#define MENU_FLAG_SUBMENU 1   // This entry calls a submenu

#define CONFIG_TINYMENU_COMPACT
#ifndef CONFIG_TINYMENU_COMPACT
# define MENU_FLAG_HIDDEN 2    // don't display this entry
#endif

#define MENU_ENTRY_NAMELEN 16 // Max size of a menu entry's name




typedef struct menu_entry_s
{
	BYTE flags;                             // see flag definitions above
	void (*select)(void *arg, char *name);  // routine to call when selected
	char name[MENU_ENTRY_NAMELEN];            // name to display for this entry
	void *value;                              // value to pass to select function
} menu_entry_t;


/* Information on a specific menu.  Previous is for the menu
 *  that this is a submenu of, or NULL if this is the main menu
 */

typedef struct menu_s
{
	BYTE top_entry;        //  top displayed entry
	BYTE current_entry;    // currently highlighted entry
	BYTE num_entries;      //  total # of entries in menu
	struct menu_s *previous; // previous menu (for backtracking)
	menu_entry_t entry[];
} menu_t;


/* Used for passing around information about a set of menus */

typedef struct
{
	BYTE x_loc;    // X location of menu
	BYTE y_loc;    // y location of menu
	BYTE height;   // max height of menu
	BYTE width;    // max width of menu
	menu_t *menu;  // menu currently in use
} menu_context_t;



extern menu_t main_menu;



void MenuUpdateDisplay(void);

void MenuPrev(void);

void MenuNext(void);

void MenuExit(void);

void MenuSelect(void);

void menu_select(menu_context_t *context);

void menu_display(menu_context_t *context);

void menu_prev_entry(menu_context_t *context);

void menu_next_entry(menu_context_t *context);

void menu_exit(menu_context_t *context);

void menu_enter(menu_context_t *context, menu_t *menu);

void StartMenu(void);

void DoMenu(BYTE key);





void m_select_set_sensors(void *arg, char *name);
void m_select_set_date_time(void *arg, char *name);
void m_select_display_version(void *arg, char *name);
void m_select_display_uALFAT_version(void *arg, char *name);
void m_select_uALFAT_update_firmware(void *arg, char *name);
//void m_select_update_firmware(void *arg, char *name);
void m_select_show_errorlog(void *arg, char *name);
void m_select_erase_errorlog(void *arg, char *name);
void m_select_erase_sensorlog(void *arg, char *name);
void m_select_sensor_profile(void *arg, char *name);



#endif

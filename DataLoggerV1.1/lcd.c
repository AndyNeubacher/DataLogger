#include "main.h"
#include "lcd.h"
#include "tools.h"



const BYTE scrn_loc[2][16] PROGMEM = {
  {0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F},
  {0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF}};




void InitLCD(void)
{
  DisplayDataEnable();   // set all display-portpins first

  LCD_RS_CONTROL();      // write to control register
  LCD_RW_LOW();          // set to write function
  LCD_E_LOW();           // data-invalid
  _delay_ms(20);         // wait 20ms after restart

  lcd_write(0x30);
  _delay_ms(6);          // wait 5.2ms
  lcd_write(0x30);
  _delay_ms(6);          // wait 5.2ms
  lcd_write(0x30);
  _delay_us(100);        // 80탎

  lcd_write(0x38);       // display has 2lines
  _delay_us(100);        // 80탎

  lcd_write(0x0C);       // display on, no cursor
  _delay_us(100);        // 80탎

  lcd_write(0x01);       // clear display
  _delay_us(100);        // 80탎

  lcd_write(0x06);       // clear display
  _delay_us(100);        // 80탎

  DisplayDataDisable();
}



void lcd_write(BYTE data)
{
  LCD_DATA = data;  // write data
  _delay_us(10);    // 10탎

  LCD_E_HIGH();     // latch data with ENABLE
  _delay_us(10);    // 10탎
  LCD_E_LOW();
}



void Cursor(BYTE shown, BYTE blink)
{
  BYTE store;

  store = 0x0C;                           // Cursor aus, Blinken aus, Display ein
  store = store + (2 * (BYTE)(shown));	 // Cursor ein/aus
  store = store + (BYTE)(blink);  	       // Blinken ein/aus

  DisplayDataEnable();

  LCD_RS_CONTROL();
  lcd_write(store);                       // write cursor settings
  _delay_us(60);

  DisplayDataDisable();
}



void MoveXY(BYTE x_col, BYTE y_row)
{
  if(y_row > 2)
    return;               // not possible -> do nothing
  if(x_col > 16)
    return;               // not possible -> do nothing

  DisplayDataEnable();    // set all display-portpins first

  LCD_RS_CONTROL();
  lcd_write(pgm_read_byte(&scrn_loc[y_row-1][x_col-1]));
  _delay_us(60);

  DisplayDataDisable();
}



void write_char(BYTE character)
{
  DisplayDataEnable();   // set all display-portpins first

  LCD_RS_DATA();          // DD-RAM adressieren
  lcd_write(character);
  _delay_us(60);

  DisplayDataDisable();
}



void ClearScreen(void)
{
  DisplayDataEnable();   // set all display-portpins first

  LCD_RS_CONTROL();
  lcd_write(0x01);
  _delay_ms(2);          // 1.6ms

  DisplayDataDisable();
}



void PrintLCD(BYTE x_pos, BYTE y_pos, CHAR* ptr)
{
	MoveXY(x_pos,y_pos);
	while((BYTE)*ptr != 0)
      write_char((BYTE)*ptr++);
}

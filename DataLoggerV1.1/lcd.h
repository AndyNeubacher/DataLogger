#ifndef __LCD_H__
#define __LCD_H__


#define  CURSOR_ON      1
#define  CURSOR_OFF     0
#define  CURSOR_BLINK   1
#define  CURSOR_STEADY  0



void InitLCD(void);

void lcd_write(BYTE);

void Cursor(BYTE shown, BYTE blink);

void MoveXY(BYTE x_col,BYTE y_row);

void write_char(BYTE character);

void ClearScreen(void);

void PrintLCD(BYTE x_pos, BYTE y_pos, CHAR *ptr);


#endif

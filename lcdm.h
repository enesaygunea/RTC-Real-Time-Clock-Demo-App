#ifndef _LCDM_H
#define _LCDM_H

#define	LCD_OFF	 	0
#define LCD_BLINK   1
#define LCD_CURSOR  2
#define LCD_ON      4

void LCD_Init(void);
void LCD_PutChar(unsigned char c);
void LCD_SetCursor(unsigned char pos);
void LCD_DisplayOn(unsigned char mode);

void LCD_putch(unsigned char c);

#endif

#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "lcdm.h"

// Data: Saf veri (komut ya da karakter kjodu olabilir) RS=0,1
// Komut: RS=0
// Karakter: RS=1

// LCD mod�le 4-bit senkron veri g�nderir
// veri: karakter kodu veya komut kodu
// g�nderilecek veri parametrenin d���k
// anlaml� 4-bitinde
static void LCD_SendDataL(unsigned char c)
{
  // 1) Data setup (verinin haz�rlanmas�)
  IO_Write(IOP_LCD_DB4, (c & 1) != 0);
  IO_Write(IOP_LCD_DB5, (c & 2) != 0);
  IO_Write(IOP_LCD_DB6, (c & 4) != 0);
  IO_Write(IOP_LCD_DB7, (c & 8) != 0);
  
  // 2) Clock generation (saat i�areti g�nderme)
  IO_Write(IOP_LCD_E, 1);
  DelayUs(3);
  IO_Write(IOP_LCD_E, 0);
  DelayUs(100);
}

// LCD mod�le 4-bit senkron komut g�nderir
static void LCD_SendCmdL(unsigned char c)
{
  IO_Write(IOP_LCD_RS, 0);      // RS = 0 (komut)
  
  LCD_SendDataL(c);     // 4-bit veri g�nder
}

// LCD mod�le 8-bit veri g�nderir
// 2 ad�mda, �nce y�ksek 4-bit, sonra d���k 4-bit
static void LCD_SendData(unsigned char c)
{
  LCD_SendDataL(c >> 4);        // y�ksek anlaml� 4-bit
  LCD_SendDataL(c);
}

// LCD mod�le 8-bit komut g�nderir
// 2 ad�mda, �nce y�ksek 4-bit, sonra d���k 4-bit
void LCD_SendCmd(unsigned char c)
{
  IO_Write(IOP_LCD_RS, 0);      // RS = 0 (komut)

  LCD_SendData(c);        // 8-bit veri g�nderiyoruz
}

// LCD mod�le 8-bit karakter g�nderir
// 2 ad�mda, �nce y�ksek 4-bit, sonra d���k 4-bit
void LCD_PutChar(unsigned char c)
{
  IO_Write(IOP_LCD_RS, 1);      // RS = 1 (karakter)

  LCD_SendData(c);        // 8-bit veri g�nderiyoruz
}

// Ekran� temizle
void LCD_Clear(void)
{
  LCD_SendCmd(0x01);    // Clear display
  DelayMs(5);
}

void LCD_DisplayOn(unsigned char mode)
{
  LCD_SendCmd(0x08 | mode);
}
       
void LCD_SetCursor(unsigned char pos)
{
  LCD_SendCmd(0x80 | pos);
}

void LCD_Init(void)
{
  IO_Init(IOP_LCD_RS, IO_MODE_OUTPUT);
  IO_Write(IOP_LCD_E, 0);
  IO_Init(IOP_LCD_E, IO_MODE_OUTPUT);

  IO_Init(IOP_LCD_DB4, IO_MODE_OUTPUT);
  IO_Init(IOP_LCD_DB5, IO_MODE_OUTPUT);
  IO_Init(IOP_LCD_DB6, IO_MODE_OUTPUT);
  IO_Init(IOP_LCD_DB7, IO_MODE_OUTPUT);
  
  DelayMs(100);
      
  // 4-bit aray�z ile ba�latma
  LCD_SendCmdL(0x03);   // 4-bit komut
  DelayMs(5);
  LCD_SendCmdL(0x03);   // 4-bit komut

  LCD_SendCmd(0x32);
  LCD_SendCmd(0x28);    // N=1 (iki sat�r) F=0 (k���k font)
  
  LCD_DisplayOn(0);     // Display off
  LCD_Clear();
  
  LCD_SendCmd(0x06);    //  I/D=1 (artma) S=0
  LCD_DisplayOn(LCD_ON);
}

void LCD_putch(unsigned char c)
{
  switch (c) {
  case '\r':
    LCD_SetCursor(0);
    break;
    
  case '\n':
    LCD_SetCursor(0x40);
    break;
    
  case '\f':
    LCD_Clear();
    break;
    
  default:
    LCD_PutChar(c);
    break;
  }
}


#ifndef _IO_H
#define _IO_H

#include "stm32f10x_gpio.h"

enum {
  IO_PORT_A,
  IO_PORT_B,
  IO_PORT_C,
  IO_PORT_D,
  IO_PORT_E,
  IO_PORT_F,
  IO_PORT_G,
};

enum {
  IO_MODE_INPUT = GPIO_Mode_IN_FLOATING,
  IO_MODE_OUTPUT = GPIO_Mode_Out_PP,
  IO_MODE_ALTERNATE = GPIO_Mode_AF_PP,
  IO_MODE_ANALOG = GPIO_Mode_AIN,
  
  IO_MODE_INPUT_PD = GPIO_Mode_IPD,
  IO_MODE_INPUT_PU = GPIO_Mode_IPU,
  IO_MODE_OUTPUT_OD = GPIO_Mode_Out_OD,
  IO_MODE_ALTERNATE_OD = GPIO_Mode_AF_OD,
};

typedef struct {
  int port;
  int pin;
} IO_PIN;

enum {
  // Blue pill LED
  IOP_LED,
  
  // LCD modül
  IOP_LCD_RS,
  IOP_LCD_E,
  IOP_LCD_DB4,
  IOP_LCD_DB5,
  IOP_LCD_DB6,
  IOP_LCD_DB7,

  // UART PINS
  IOP_U1RX,
  IOP_U1TX,
  IOP_U2RX,
  IOP_U2TX,
  IOP_U3RX,
  IOP_U3TX,
  
  // Push Button PINS
  IOP_BTN_SET,
  IOP_BTN_UP,
  IOP_BTN_DN,
};

// Variable definitions
#ifdef _IOS_
IO_PIN _ios[] = {
  { IO_PORT_C, 13 },    
  
  // LCD modül
  { IO_PORT_B, 9 },    
  { IO_PORT_B, 8 },    
  { IO_PORT_A, 7 },    
  { IO_PORT_A, 6 },    
  { IO_PORT_A, 5 },    
  { IO_PORT_A, 4 },

  // UART I/O PINS
  { IO_PORT_A, 10 },
  { IO_PORT_A, 9 },
  { IO_PORT_A, 3 },
  { IO_PORT_A, 2 },
  { IO_PORT_B, 11 },
  { IO_PORT_B, 10 },
  
  // Push Buttons PINS
  { IO_PORT_A, 0 },
  { IO_PORT_A, 1 },
  { IO_PORT_A, 2 },
};

GPIO_TypeDef    *_GPIO_Ports[] = {
  GPIOA,
  GPIOB,
  GPIOC,
  GPIOD,
  GPIOE,
  GPIOF,
  GPIOG,
};

#endif


void IO_Init(int idx, int mode);
void IO_Write(int idx, int val);
int IO_Read(int idx);

#endif

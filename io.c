#include <stdio.h>
#include <stdint.h>

#include "stm32f10x.h"

#define _IOS_
#include "io.h"

// Initialization using ST Peripheral Library
void IO_Init(int idx, int mode)
{
  GPIO_InitTypeDef ioInit;
  int port;
  
  ioInit.GPIO_Mode = (GPIOMode_TypeDef)mode;
  ioInit.GPIO_Speed = GPIO_Speed_50MHz;
  ioInit.GPIO_Pin = (1 << _ios[idx].pin);
  
  port = _ios[idx].port;
  GPIO_Init(_GPIO_Ports[port], &ioInit);
}

void IO_Write(int idx, int val)
{
  int port;
  
  port = _ios[idx].port;
  
  if (val)
    _GPIO_Ports[port]->BSRR = (1 << _ios[idx].pin);
  else
    _GPIO_Ports[port]->BRR = (1 << _ios[idx].pin);    
}

int IO_Read(int idx)
{
  int port;
  
  port = _ios[idx].port;
  
  return (_GPIO_Ports[port]->IDR & (1 << _ios[idx].pin)) != 0;
}



#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "system.h"
#include "io.h"
#include "uart.h"

#include "stm32f10x_usart.h"

int g_conUART = UART_1;

struct UART_CFG {
	int	ioTx;
	int	ioRx;

	uint32_t	ckUART;
	USART_TypeDef	*pUSART;
};

/*
static USART_TypeDef *_tUART[] = {
		USART1,
		USART2,
		USART3
};
*/

static struct UART_CFG _uCfg[] = {
		{ IOP_U1TX, IOP_U1RX, RCC_APB2Periph_USART1, USART1 },
		{ IOP_U2TX, IOP_U2RX, RCC_APB1Periph_USART2, USART2 },
		{ IOP_U3TX, IOP_U3RX, RCC_APB1Periph_USART3, USART3 },
};


void UART_Init(int idx, int baud)
{
	USART_InitTypeDef uInit;

	// 1) I/O uçlarý yapýlandýrýlýr
	// TX ucu yapýlandýrmasý
	IO_Init(_uCfg[idx].ioTx, IO_MODE_ALTERNATE);

	// Rx ucu yapýlandýrmasý
	IO_Init(_uCfg[idx].ioRx, IO_MODE_INPUT);

	// 2) UART çevresel birim için clock saðlýyoruz
	if (idx == UART_1)
		RCC_APB2PeriphClockCmd(_uCfg[idx].ckUART, ENABLE);
	else
		RCC_APB1PeriphClockCmd(_uCfg[idx].ckUART, ENABLE);

	// 3) Init yapýsý baþlatýlýr
	uInit.USART_BaudRate = baud;
	uInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uInit.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	uInit.USART_Parity = USART_Parity_No;
	uInit.USART_StopBits = USART_StopBits_1;
	uInit.USART_WordLength = USART_WordLength_8b;

	USART_Init(_uCfg[idx].pUSART, &uInit);

	// 4) Çevresel aktive edilir (çalýþtýrýlýr)
	USART_Cmd(_uCfg[idx].pUSART, ENABLE);
}

void UART_Send(int idx, unsigned char val)
{
	// 1) TSR yükleme için uygun mu? (boþ mu)
	// TSR dolu olduðu müddetçe bekle
	while (!USART_GetFlagStatus(_uCfg[idx].pUSART, USART_FLAG_TXE)) ;

	// 2) Yüklemeyi yap
	USART_SendData(_uCfg[idx].pUSART, val);
}

void UART_Send2(int idx, unsigned char val)
{
	// 1) Yüklemeyi yap
	USART_SendData(_uCfg[idx].pUSART, val);

	// 2) Veri gidene dek bekle
	while (!USART_GetFlagStatus(_uCfg[idx].pUSART, USART_FLAG_TC)) ;

}

int UART_DataReady(int idx)
{
	return USART_GetFlagStatus(_uCfg[idx].pUSART, USART_FLAG_RXNE);
}

// Bloke çalýþýr, hazýr veri yoksa bekler
unsigned char UART_Recv(int idx)
{
	while (!UART_DataReady(idx)) ;

	return (unsigned char)USART_ReceiveData(_uCfg[idx].pUSART);
}

void UART_putch(unsigned char c)
{
	if (c == '\n')
		UART_Send2(g_conUART, '\r');

	UART_Send2(g_conUART, c);
}

int UART_puts(const char *str)
{
	int i = 0;

	while (str[i])
		UART_putch(str[i++]);

	return i;
}

int UART_printf(const char *fmt, ...)
{
	va_list args;
	char str[SZ_PRNBUF];

	va_start(args, fmt);
	vsnprintf(str, SZ_PRNBUF, fmt, args);

	return UART_puts(str);
}


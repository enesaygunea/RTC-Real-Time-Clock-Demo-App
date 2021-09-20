#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "uart.h"
#include "button.h"
#include "irtc.h"
#include "lcdm.h"

// RTC setup timeout deðeri
#define RTC_TMOUT       30000

enum { IRTC_WORK, IRTC_SETUP };
int g_RtcMode = IRTC_WORK;

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Konsol baþlangýç
  Sys_ConsoleInit();
  
  // Button Baþlangýç
  BTN_InitButtons();
  
  // RTC Baþlangýç
  IRTC_Init();
  IRTC_IntConfig();
}

void Task_LED(void)
{
  static enum {
    I_LED_OFF,
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
  
  static clock_t t0;    // Duruma ilk geçiþ saati
  clock_t t1;           // Güncel saat deðeri
  
  t1 = clock();
  
  switch (state) {
  case I_LED_OFF:
      t0 = t1;      
      IO_Write(IOP_LED, 1);     // LED off
      state = S_LED_OFF;
      //break;    
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10) 
      state = I_LED_ON;
    break;
  
  case I_LED_ON:
    t0 = t1;
    IO_Write(IOP_LED, 0);     // LED On
    state = S_LED_ON;
    //break;    
  case S_LED_ON:
    if (t1 >= t0 + CLOCKS_PER_SEC / 10) 
      state = I_LED_OFF;
    break;
  }  
}

void Task_Print(void)
{
  static unsigned count;
  
  printf("SAYI:%10u\n", ++count);
  //UART_printf("Count: %10lu\r", count);
}

void Task_Button(void)
{
  static unsigned count = 0;
  
  if (g_Buttons[BTN_SET]) {
    // SET iþle
    UART_printf("SET (%u)\n", ++count);
    
    // g_Buttons[BTN_SET] = 0; // binary semaphore
    --g_Buttons[BTN_SET];      // counting semaphore
  }
  
  if (g_Buttons[BTN_UP]) {
    // UP iþle
    UART_printf("UP (%u)\n", ++count);
    
    // g_Buttons[BTN_UP] = 0;  // binary semaphore
    --g_Buttons[BTN_UP];      // counting semaphore
  }
  
  if (g_Buttons[BTN_DN]) {
    // DOWN iþle
    UART_printf("DN (%u)\n", ++count);
    
    // g_Buttons[BTN_DN] = 0; // binary semaphore
    --g_Buttons[BTN_DN];      // counting semaphore
  }
  
  
  /////////////LONG PRES//////////////////
#ifdef BTN_LONG_PRESS
  if (g_ButtonsL[BTN_SET]) {
    // SET iþle
    UART_printf("SET_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_SET] = 0; 
  }
  
  if (g_ButtonsL[BTN_UP]) {
    // UP iþle
    UART_printf("UP_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_UP] = 0; 
  }
  
  if (g_ButtonsL[BTN_DN]) {
    // DOWN iþle
    UART_printf("DN_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_DN] = 0; 

  }
#endif
}

void SetAlarm(uint32_t nSeconds)
{
  uint32_t tm1;
  
  tm1 = IRTC_GetTime();
  tm1 += nSeconds;
  IRTC_SetAlarm(tm1);
}

void Task_Time(void)
{
  /*UART_printf("RTC time: %10lu\r", 
              IRTC_GetTime());
  */
  
  if((g_RtcMode != IRTC_SETUP) && g_RtcChanged) {
    printf("\nTime:%8lu", ++g_Time);
    g_RtcChanged = 0;
  }
}

void Task_Alarm(void)
{
  static unsigned nAlarms;
  
  if((g_RtcMode != IRTC_SETUP) && g_RtcAlarm) {
    LCD_SetCursor(0x0A);
    printf("(%u)", ++nAlarms);
    
    SetAlarm(60);
    g_RtcAlarm = 0;
  }
}

void DisplayTime(uint32_t tmVal)
{
  int hour, min, sec;
  
  hour = (tmVal / 3600) % 24;
  min = (tmVal / 60) % 60;
  sec = (tmVal % 60);
  
  printf("\r%02d:%02d:%02d",
          hour, min, sec);
}

void Task_RTC(void)
{
  static enum {
    I_WORK,
    S_WORK,
    I_SETHOUR,
    S_SETHOUR,
    I_SETMIN,
    S_SETMIN,
    S_INCHOUR,
    S_DECHOUR,
    S_INCMIN,
    S_DECMIN,
    S_UPDATE,
    S_CANCEL,
  } state = I_WORK;
  
  static clock_t t0, t1;
  static uint32_t tm0, tm1;
  static int hour, min;
  
  t1 = clock(); // güncel saat deðeri (ms)
  
  if (g_RtcOW) {
    
    tm1 = IRTC_GetTime();
    tm1 += 6 * 3600 + 28 * 60 + 16;
    IRTC_SetTime(tm1);
    
    g_RtcOW = 0;
  }
  
  switch (state) {
  case I_WORK:
    g_RtcMode = IRTC_WORK;
    LCD_DisplayOn(LCD_ON);
    state = S_WORK;
    //break;
    
  case S_WORK:
    tm1 = IRTC_GetTime();
    if (tm1 != tm0) {
      DisplayTime(tm1);
      tm0 = tm1;
    }
    
    if (g_ButtonsL[BTN_SET]) {
      hour = (tm1 / 3600) % 24;
      min = (tm1 / 60) % 60;
    
      g_ButtonsL[BTN_SET] = 0;
     
      //clear false signal
      g_Buttons[BTN_SET] = 0;   
      g_Buttons[BTN_UP] = 0;
      g_Buttons[BTN_DN] = 0;
      
      state = I_SETHOUR;
    }
    break;
    
  case I_SETHOUR:
    g_RtcMode = IRTC_SETUP;
    
    LCD_DisplayOn(LCD_ON | LCD_BLINK);
   
    DisplayTime(hour * 3600 + min * 60);
    LCD_SetCursor(1);
    t0 = t1;
    state = S_SETHOUR;
    break;
    
  case S_SETHOUR:
    if (t1 - t0 >= RTC_TMOUT) {
      state = S_CANCEL;
    }
    
    if (g_Buttons[BTN_SET]) {
      g_Buttons[BTN_SET] = 0;
      state = I_SETMIN;
    }
    else if (g_Buttons[BTN_UP]) {
      g_Buttons[BTN_UP] = 0;
      state = S_INCHOUR;
    }
    else if (g_Buttons[BTN_DN]) {
      g_Buttons[BTN_DN] = 0;
      state = S_DECHOUR;
    } 
    break;
    
  case I_SETMIN:
    DisplayTime(hour * 3600 + min * 60);
    LCD_SetCursor(4);
    
    t0 = t1;
    state = S_SETMIN;
    break;
    
  case S_SETMIN:
    if (t1 - t0 >= RTC_TMOUT) {
      state = S_CANCEL;
    }
    
    if (g_Buttons[BTN_SET]) {
      g_Buttons[BTN_SET] = 0;
      state = S_UPDATE;
    }
    else if (g_Buttons[BTN_UP]) {
      g_Buttons[BTN_UP] = 0;
      state = S_INCMIN;
    }
    else if (g_Buttons[BTN_DN]) {
      g_Buttons[BTN_DN] = 0;
      state = S_DECMIN;
    }
    break;
    
  case S_INCHOUR:
    if (++hour >= 24)
      hour = 0;
    
    state = I_SETHOUR;
    break;
    
  case S_DECHOUR:
    if (--hour < 0)
      hour = 23;
    
    state = I_SETHOUR;
    break;
    
  case S_INCMIN:
    if (++min >= 60)
      min = 0;
    
    state = I_SETMIN;
    break;
    
  case S_DECMIN:
    if (--min < 0)
      min = 59;
    
    state = I_SETMIN;
    break;
    
  case S_UPDATE:
    IRTC_SetTime(hour * 3600 + min * 60);
    state = I_WORK;
    break;
    
  case S_CANCEL:
    state = I_WORK;
    break;
  
  }
}


int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();
    
  
  //IRTC_SetTime(22 * 3600 + 39 * 60 + 17);
  //IRTC_SetTime(0xFFFFFFF0);
  SetAlarm(60);
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev) 
  while (1)
  {
    Task_LED();
    //Task_Print();

    //Task_Button();
    Task_Time();
    Task_Alarm();
    
    Task_RTC();
  }
  
  //return 0;
}



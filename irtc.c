#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"

#define IRTC_FLAG       0xE3B8  // Selected random value. NOT: just used as a flag.
#define RTC_PS          32768

volatile int g_Time = 0;
volatile int g_RtcAlarm = 0;
volatile int g_RtcOW = 0;
volatile int g_RtcChanged = 0;

// RTC Interrupt Handler
// Proccessing interrupt handler from 3 different interrupt resource.
void RTC_IRQHandler(void)
{
  // RTC second
  if (RTC_GetITStatus(RTC_IT_SEC)) {  // Get Event Flag
    g_RtcChanged = 1;
    RTC_ClearITPendingBit(RTC_IT_SEC); // Event flag bit = 0
  }
  
  // RTC Alarm
  if (RTC_GetITStatus(RTC_IT_ALR)) {
    g_RtcAlarm = 1;
    RTC_ClearITPendingBit(RTC_IT_ALR);
  }
  
  // RTC Overflow
  if (RTC_GetITStatus(RTC_IT_OW)) {
    g_RtcOW = 1;
    RTC_ClearITPendingBit(RTC_IT_OW);
  }
  
}

uint32_t IRTC_GetTime(void)
{
  return RTC_GetCounter();
}

void IRTC_SetTime(uint32_t tmVal)
{
  // RTC' de süren işlem varsa bekle
  RTC_WaitForLastTask();
  
  RTC_SetCounter(tmVal);
  
  // RTC' de süren işlem varsa bekle
  RTC_WaitForLastTask();
}

void IRTC_Init(void)
{
    // PWR ve BKP birimleri saat işareti açılır.
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); 
  
  // Backup register'larına erişimi açıyoruz.
  PWR_BackupAccessCmd(ENABLE);
  
  if (BKP_ReadBackupRegister(BKP_DR1) != IRTC_FLAG) {
    // Backup domain resetlenmiş ise buraya girer
    BKP_DeInit();
    
    // LSE(Low Speed External Osilatör)' ü çalıştırıyoruz.
    RCC_LSEConfig(RCC_LSE_ON);
    
    // LSE çalışana kadar bekle
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    
    // RTC clock kaynağı = LSE olsun
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
    // RTC clock active
    RCC_RTCCLKCmd(ENABLE);
    
    // RTC register' larına erişim için senkronlama gerekli
    RTC_WaitForSynchro();
    
    // RTC' de süren işlem varsa bekle
    RTC_WaitForLastTask();
    
    // RTC prescale ayarı (LSE periyot register'ı)
    RTC_SetPrescaler(RTC_PS - 1);
    
    // işlem tamamlandı
    BKP_WriteBackupRegister(BKP_DR1, IRTC_FLAG);
  }
  else {
    // RTC register' larına erişim için senkronlama gerekli
    RTC_WaitForSynchro();
  }
}

void IRTC_IntConfig(void)
{
  // *RTC second Kesme Kaynağı
    // RTC Periodic Interrupt - Sayacın her bir artışında kesme alabiliyoruz.
    
    // 2 ayrı bölgede ayar yapacağız.
    // - Peripheral(çevresel)
    // - Core(NVIC)
    
  RTC_ClearITPendingBit(RTC_IT_SEC);  // 0) False interrupt önlemi
  RTC_ITConfig(RTC_IT_SEC, ENABLE); // 1) Çevresel kesme izni
  
  // *RTC Alarm Kesme Kaynağı
  RTC_ClearITPendingBit(RTC_IT_ALR);  // 0) False interrupt önlemi
  RTC_ITConfig(RTC_IT_ALR, ENABLE); // 1) Çevresel kesme izni
  
  // *RTC Overflow Kesme Kaynağı
  RTC_ClearITPendingBit(RTC_IT_OW);  // 0) False interrupt önlemi
  RTC_ITConfig(RTC_IT_OW, ENABLE); // 1) Çevresel kesme izni
  
  NVIC_SetPriority(RTC_IRQn, 3); // 2.1) RTC kesme önceliği = 3 yapmış olduk. Not: 0 en yüksek önceliktir.
  NVIC_EnableIRQ(RTC_IRQn); // 2.2) RTC' nin second kesmesini aktif hale getirdik.
}

void IRTC_SetAlarm(uint32_t alrVal)
{
  RTC_SetAlarm(alrVal - 1);
}


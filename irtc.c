#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"

#define IRTC_FLAG       0xE3B8  // Rastgele bir say� belirleyip sonra onu flag olarak set ediyoruz.
#define RTC_PS          32768

volatile int g_Time = 0;
volatile int g_RtcAlarm = 0;
volatile int g_RtcOW = 0;
volatile int g_RtcChanged = 0;

// RTC Interrupt Handler
// Farkl� 3 interrupt kaynaktan ortaya ��kan kesmeleri i�leyecek.
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

// Saniye sayac� olarak referans zaman�ndan bu yana
// ge�en s�reye geri d�ner
uint32_t IRTC_GetTime(void)
{
  return RTC_GetCounter();
}

void IRTC_SetTime(uint32_t tmVal)
{
  // RTC' de s�ren i�lem varsa bekle
  RTC_WaitForLastTask();
  
  RTC_SetCounter(tmVal);
  
  // RTC' de s�ren i�lem varsa bekle
  RTC_WaitForLastTask();
}

void IRTC_Init(void)
{
    // PWR ve BKP birimleri saat i�areti a��l�r.
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); 
  
  // Backup register'lar�na eri�imi a��yoruz.
  PWR_BackupAccessCmd(ENABLE);
  
  if (BKP_ReadBackupRegister(BKP_DR1) != IRTC_FLAG) {
    // Backup domain resetlenmi� ise buraya girer
    BKP_DeInit();
    
    // LSE(Low Speed External Osilat�r)' � �al��t�r�yoruz.
    RCC_LSEConfig(RCC_LSE_ON);
    
    // LSE �al��ana kadar bekle
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    
    // RTC clock kayna�� = LSE olsun
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
    // RTC clock active
    RCC_RTCCLKCmd(ENABLE);
    
    // RTC register' lar�na eri�im i�in senkronlama gerekli
    RTC_WaitForSynchro();
    
    // RTC' de s�ren i�lem varsa bekle
    RTC_WaitForLastTask();
    
    // RTC prescale ayar� (LSE periyot register'�)
    RTC_SetPrescaler(RTC_PS - 1);
    
    // i�lem tamamland�
    BKP_WriteBackupRegister(BKP_DR1, IRTC_FLAG);
  }
  else {
    // RTC register' lar�na eri�im i�in senkronlama gerekli
    RTC_WaitForSynchro();
  }
}

void IRTC_IntConfig(void)
{
  // *RTC second Kesme Kayna��
    // RTC Periodic Interrupt - Sayac�n her bir art���nda kesme alabiliyoruz.
    
    // 2 ayr� b�lgede ayar yapaca��z.
    // - Peripheral(�evresel)
    // - Core(NVIC)
    
  RTC_ClearITPendingBit(RTC_IT_SEC);  // 0) False interrupt �nlemi
  RTC_ITConfig(RTC_IT_SEC, ENABLE); // 1) �evresel kesme izni
  
  // *RTC Alarm Kesme Kayna��
  RTC_ClearITPendingBit(RTC_IT_ALR);  // 0) False interrupt �nlemi
  RTC_ITConfig(RTC_IT_ALR, ENABLE); // 1) �evresel kesme izni
  
  // *RTC Overflow Kesme Kayna��
  RTC_ClearITPendingBit(RTC_IT_OW);  // 0) False interrupt �nlemi
  RTC_ITConfig(RTC_IT_OW, ENABLE); // 1) �evresel kesme izni
  
  NVIC_SetPriority(RTC_IRQn, 3); // 2.1) RTC kesme �nceli�i = 3 yapm�� olduk. Not: 0 en y�ksek �nceliktir.
  NVIC_EnableIRQ(RTC_IRQn); // 2.2) RTC' nin second kesmesini aktif hale getirdik.
}

void IRTC_SetAlarm(uint32_t alrVal)
{
  RTC_SetAlarm(alrVal - 1);
}


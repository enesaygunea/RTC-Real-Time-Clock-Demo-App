#include <stdio.h>
#include <stdint.h>

#include "io.h"
#include "button.h"

int g_dbMax = 50;

int _bScan = 0;         

static BTN_PIN _bts[] = {
  { IOP_BTN_SET, 1, 0, 0 }, 
  { IOP_BTN_UP, 1, 0, 0 }, 
  { IOP_BTN_DN, 1, 0, 0 }, 
};

#define N_BUTTONS       (sizeof(_bts) / sizeof(BTN_PIN))

unsigned g_Buttons[N_BUTTONS]; // Button Semaphore (binary / counting )
unsigned g_ButtonsL[N_BUTTONS]; // Button Long Press Semaphore (binary)

//Each button invokes from "button timer" tick.
// Each button doing one scaninig.
static void BTN_Scan(int btIdx)
{
  int r; // Pin read value
  
  r = IO_Read(_bts[btIdx].ioIdx);
  
  if (r != _bts[btIdx].cState) {
    if(++_bts[btIdx].dbc >= g_dbMax) {
      _bts[btIdx].cState = r;
      
      _bts[btIdx].dbc = 0;
      
      if (_bts[btIdx].cState == _bts[btIdx].aState ) {
        // Signal !!
        // g_Buttons[btIdx] = 1; // binary semaphore
        ++g_Buttons[btIdx];     // counting semaphore
      }
#ifdef BTN_LONG_PRESS
      else {
        _bts[btIdx].lState = 0;
      }
#endif
    }
  }
  else {
    _bts[btIdx].dbc = 0;
  }

#ifdef BTN_LONG_PRESS
  if(_bts[btIdx].lState == 0) {
    if (_bts[btIdx].cState == _bts[btIdx].aState ){
      if (++_bts[btIdx].acc >= BT_LP_TIME ) {
        _bts[btIdx].acc = 0;
        _bts[btIdx].lState = 1;
        g_ButtonsL[btIdx] = 1; // binary semaphore
      }
    }
  }
#endif
}

void BTN_InitButtons(void)
{
  for (int i =0; i < N_BUTTONS; ++i)
    IO_Init(_bts[i].ioIdx, IO_MODE_INPUT_PU);
  
  _bScan = 1;
}
void BTN_ScanButtons(void)
{
  if (!_bScan)
    return;
  
  for (int i =0; i < N_BUTTONS; ++i)
    BTN_Scan(i);
  
}



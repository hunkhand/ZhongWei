#include "bsp.h"

extern u32 CapRange;
extern u32 MinVarCap;

void CapCalibrate(void)
{
  u8 ptmp[2];
  u16 TempAir;
  OS_ERR  Err;
	
  if((CalibrateLow == RunVar.CalState) || (CalibrateHeight == RunVar.CalState))
  {
    LED_Open();
    OSTmrStart(&TMR_LED_Short_Light, &Err);
  }
  else if(Calibrated == RunVar.CalState)
  {
    RunVar.CalState = Def_State;
    TempAir = (u16)((RunVar.TempInAir + 273.15) * 10);
    ptmp[1] = (u8)TempAir;
    ptmp[0] = (u8)(TempAir >> 8);
    if(ArrToHex(&Cur_Param[CAPMIN]) < ArrToHex(&Cur_Param[CAPMAX]))
    {
      //FilterStartFlag = Bit_RESET;
      I2C1_WNBytesMul3T(EEPROM_ADDRESS, CAPMIN, 8, &Cur_Param[CAPMIN]);
      I2C1_WriteNBytes(EEPROM_ADDRESS, CALIBTEMPER, 2, ptmp);
                        
      ProductPara.CapMin = ArrToHex(&Cur_Param[CAPMIN]);
      ProductPara.CapMax = ArrToHex(&Cur_Param[CAPMAX]);
      ProductPara.CapRange = ProductPara.CapMax - ProductPara.CapMin;
      ProductPara.CalibTemp = TempAir;

      LED_Open();
      ulLedStatue = 2;
      OSTmrStart(&TMR_LED_Double_Light, &Err);
    }
    else
    {
      LED_Open();
      OSTmrStart(&TMR_LED_Short_Light, &Err);
    }
  }
}


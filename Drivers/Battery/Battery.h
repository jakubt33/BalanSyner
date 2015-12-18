#ifndef BATTERY_H_
#define BATTERY_H_

//-----------------------Includes-------------------------------------//
#include "../Drivers/PinDefines.h"

//-----------------------Public typedefs------------------------------//


typedef struct
{
   float Voltage;
   float ChargedPercent;

   void ( *AdjustPwm ) ( float *Pwm );
   float ( *GetVoltage ) ( void );
   void ( *Perform ) ( void );
}Battery_C;

//-----------------------Public defines-------------------------------//

//-----------------------Public macros--------------------------------//

//-----------------------Public variables-----------------------------//
Battery_C oBattery;

//-----------------------Public prototypes----------------------------//
void InitializeBattery();

#endif

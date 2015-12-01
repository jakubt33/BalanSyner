/**
*****************************************************************************
**
**  File        : main.c
**
**  Abstract    : main function.
**
**  Functions   : main
**
**  Environment : Atollic TrueSTUDIO(R)
**
**  Distribution: The file is distributed �as is,� without any warranty
**                of any kind.
**
**  (c)Copyright Atollic AB.
**  You may use this file as-is or modify it according to the needs of your
**  project. This file may only be built (assembled or compiled and linked)
**  using the Atollic TrueSTUDIO(R) product. The use of this file together
**  with other tools than Atollic TrueSTUDIO(R) is not permitted.
**
*****************************************************************************
*/

//-----------------------Includes-------------------------------------//
#include "stm32f30x.h"
#include "main.h"

#include "../StmPeriphInit/SysTick.h"

#include "../Drivers/Clock/clock.h"
#include "../Drivers/LEDs/LED.h"
#include "../Drivers/BT/BT.h"
#include "../Drivers/Wifi/Wifi.h"
#include "../Drivers/MPU/MPU.h"
#include "../Drivers/Motors/Motors.h"

#include "Logic.h"

//-----------------------Private typedefs------------------------------//

//-----------------------Private defines-------------------------------//

//-----------------------Private macros--------------------------------//

//-----------------------Private variables-----------------------------//

//-----------------------Private prototypes----------------------------//
int main(void);

//-----------------------Private functions-----------------------------//
int main(void)
{
   InitializeClock();
   InitializeSysTick();

#ifdef _USE_MOTORS
   InitializeMotors();
#endif

#ifdef _USE_ENCODERS
   InitializeEncoders();
#endif

#ifdef _USE_SERVOS
   InitializeServos();
#endif

#ifdef _USE_LED_NUCLEO
   InitializeLedNucleo();
#endif

#ifdef _USE_LED_14
   InitializeLed14();
#endif

#ifdef _USE_LED_EYE
   InitializeLedEye();
#endif

#ifdef _USE_BT
   InitializeBT();
#endif

#ifdef _USE_WIFI
   InitializeWifi();
#endif

#ifdef _USE_MPU
   InitializeMPU();
#endif

#if 0
   if(//MPU error))
   {
      //sth bad had happened...
   }
#endif

   oMotor.SetSpeed( SelectMotorLeft, 0.0f );
   while (1)
   {

   }
   return 0;
}

//-----------------------Public functions------------------------------//
void priv_SendC( float Value, uint8_t Address )
{
   uint8_t Command[] = { 0xFF, 0xFF, Address, 0, 0, 0, 0, 1 };

   uint32_t transport_bits = *( ( uint32_t* ) &Value );
   *(uint32_t *) &Command[3] = transport_bits;

   uint8_t i = 0;
   while ( 8 > i )
   {
      oBluetooth.PushFifo( &oBluetooth.oBtTxFifo, *( Command + i++ ) );
   }
   oBluetooth.SendFifo();
}

void MainTask8ms()
{
   /*! Measure angle of the robot */
   oMpuKalman.ApplyFilter();

   /*! Apply PID filter to motors to get required angle (output of omega regulator) */
   oPID_Angle.ApplyPid( &oPID_Angle.Parameters, oMpuKalman.AngleFiltered );

   oMotor.SetSpeed( SelectMotorLeft, oPID_Angle.Parameters.OutSignal );
}

void MainTask16ms()
{
   LED1_Toggle;

#if 0
   static float f=0.0f;
   static uint8_t up = 1;
   if(up)
   {
      f += 1;
      if(f>=180) up=0;
   }
   else{
      f -= 1;
      if(f<=-180) up=1;
   }
   oServosArm.SetAngle(SelectServoArmLeft, f);

#endif
}

void MainTask128ms()
{
   LED2_Toggle;
   /*! Check if any command from USART or buttons came and save buffer to struct. ADCx2. */
   Logic_CheckInputs();

   /*! Calculate mean omega of the robot */
   float OmegaMean = ( oEncoderLeft.Perform( &oEncoderLeft.Parameters )
                      +oEncoderRight.Perform( &oEncoderRight.Parameters )
                     ) / 2;

   priv_SendC(oEncoderLeft.Parameters.Omega, 5);

#if 1
   /*! Apply PID filter to motors to get required omega */
   oPID_Omega.ApplyPid( &oPID_Omega.Parameters, oEncoderLeft.Parameters.Omega );
   oPID_Angle.SetDstValue( &oPID_Angle.Parameters, oPID_Omega.Parameters.OutSignal );
#endif
}

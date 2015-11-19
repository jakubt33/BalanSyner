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

#include "../Framework/inc/KalmanFilter.h"

#include "../StmPeriphInit/SysTick.h"

#include "../Drivers/Clock/clock.h"
#include "../Drivers/LEDs/LED.h"
#include "../Drivers/BT/BT.h"
#include "../Drivers/Wifi/Wifi.h"
#include "../Drivers/MPU/MPU.h"
#include "../Drivers/Motors/Motors.h"

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
	if(//no MPU error))
	{
		//sth bad had happened...
	}
#endif

	/*todo: only for labview sending purposes
	*AnglePrsc1000 = (int32_t)(*Angle*1000);*/

	KalmanInitialize();
	while (1)
	{
		;
	}
	return 0;
}

inline void MainTask32ms()
{
	/* whole process needs about 2ms */
	//LED_NUCLEO_IsOn ? LED_Nucleo_SetOn : LED_Nucleo_SetOff;

	//MPU_Perform(); //save angle to struct in MPU.h/c
	oMpuKalman.GetFiltedAngle();
	oEncoders.SetOmega( TIM_ENC1, 10 );
	oEncoders.EncoderOmegaLeft = oEncoders.GetOmega( TIM_ENC1 );

	//CheckInputs(); //check if any command from USART or buttons came and save buffer to struct. ADCx2.
	//LogicPerform(); // analyze angle and commands, PID and set PWMs,
	//SendOutputs(); //Some kind of variant manager maybe? if wifi or bt or pi. Send data to USART receiver, leds, lcd

	oMotor.SetSpeed(SelectMotorLeft, 100, 1);
	oMotor.SetSpeed(SelectMotorRight, 800, 1);
	oServosArm.SetAngle(SelectServoArmLeft, 0);
	oServosArm.SetAngle(SelectServoArmRight, 90);
	oServosArm.SetAngle(SelectServoCamHor, -90);
	oServosArm.SetAngle(SelectServoCamVer, 45);

	BT_SendMeasuredData( );

	//LED_NUCLEO_IsOn ? LED_Nucleo_SetOn : LED_Nucleo_SetOff;
}
//-----------------------Public functions------------------------------//


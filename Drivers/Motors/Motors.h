/*
 * Motors.h
 *
 *  Created on: Oct 25, 2015
 *      Author: Kuba
 */

#ifndef MOTORS_H_
#define MOTORS_H_

//-----------------------Includes-------------------------------------//
#include "../PinDefines.h"

//-----------------------Public typedefs------------------------------//
typedef enum
{
   SelectMotorLeft,
   SelectMotorRight,
}MotorSelector_T;

typedef enum
{
   SelectEncoderLeft,
   SelectEncoderRight,
}EncoderSelector_T;

typedef enum
{
   SelectServoArmLeft,
   SelectServoArmRight,
   SelectServoCamHor,
   SelectServoCamVer,
}ServoSelector_T;

typedef struct
{
   float Dt;
   float Omega;
   uint16_t Counter;
   TIM_TypeDef * TIMx;     /*!< which timer */
}EncoderParameters_T;

typedef struct
{
   EncoderParameters_T Parameters;

   float ( *GetOmega )( EncoderParameters_T *pkThis );
   void ( *SetOmega )( TIM_TypeDef * TIMx, uint32_t Value );
}Encoder_C;

typedef struct
{
   //TODO void get speed
   void ( *SetSpeed )( MotorSelector_T Motor, uint16_t Value, uint8_t Direction );
}Motors_C;

typedef struct
{
   //TODO void get angle
   void ( *SetAngle )( ServoSelector_T ServoSelector, float Angle );
}Servos_C;

//-----------------------Public defines-------------------------------//

//-----------------------Public macros--------------------------------//

//-----------------------Public variables-----------------------------//
Encoder_C oEncoderLeft;
Motors_C oMotor;
Servos_C oServosArm;

//-----------------------Public prototypes----------------------------//
void InitializeEncoders();
void InitializeMotors();
void InitializeServos();

#endif /* MOTORS_H_ */

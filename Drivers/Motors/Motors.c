/*
 * Motors.c
 *
 *  Created on: Oct 25, 2015
 *      Author: Kuba
 */

//-----------------------Includes-------------------------------------//
#include "Motors.h"
#include "../PinDefines.h"

#include "../StmPeriphInit/RCC.h"
#include "../StmPeriphInit/Timers.h"
#include "../StmPeriphInit/GPIO.h"
#include "../StmPeriphInit/NVIC.h"

//-----------------------Private typedefs------------------------------//

//-----------------------Private defines-------------------------------//
/*!
 * For max speed 500RPM and DT=0.128s and encoder resolution 1200 ticks per shaft revolution,
 * maximum possible ticks per iteration is 1280 so any number above that value is safe to use
 * (counter won't over/underflow). Resolution is 1tick*AnglePerTick/DT = 0.0384 deg/sec.
 */
#define CounterDef         1500u /*! Must be less than (tim->period)/2 so counter wont overflow */
#define TicksPerRevolution 1200u
#define AnglePerTick       0.3f
#define RpmPerTIck         0.05f

//-----------------------Private macros--------------------------------//
#define ANGLE_TO_PWM_VALUE(ANGLE)   ( 2u*(ANGLE)+540u )

//-----------------------Private variables-----------------------------//
extern float DT_slow;

//-----------------------Private prototypes----------------------------//
static void priv_MotorSetSpeed(MotorSelector_T Motor, float Value );
static float priv_EncoderPerform( EncoderParameters_T *pkThis );
static float priv_GetOmega( EncoderParameters_T *pkThis );

static void priv_ServoSetAngle(ServoSelector_T Servo, float Angle);

static void priv_SetAngleArmLeft(uint16_t Angle);
static void priv_SetAngleArmRight(uint16_t Angle);
static void priv_SetAngleCamHor(uint16_t Angle);
static void priv_SetAngleCamVer(uint16_t Angle);

//-----------------------Private functions-----------------------------//
/*!
 * Value may vary between -1000 and 1000.
 */
static void priv_MotorSetSpeed( MotorSelector_T MotorSelector, float Value )
{
   if( Value <= 1000 && Value >= -1000 )
   {
      uint8_t Sign = ( Value > 0 );
      switch (MotorSelector)
      {
      case SelectMotorLeft:
         if( Sign )
         {
            GPIO_SetBits( MOT1_DIRA_GPIO,MOT1_DIRA_PIN );
            GPIO_ResetBits( MOT1_DIRB_GPIO,MOT1_DIRB_PIN );
         }
         else
         {
            GPIO_ResetBits( MOT1_DIRA_GPIO,MOT1_DIRA_PIN );
            GPIO_SetBits( MOT1_DIRB_GPIO,MOT1_DIRB_PIN );
         }
         TIM_MOTORS->MOT1_PWM_CHANNEL = Sign ? (uint16_t) Value : (uint16_t) -Value;
         break;
      case SelectMotorRight:
         if( Sign )
         {
            GPIO_ResetBits( MOT2_DIRA_GPIO,MOT2_DIRA_PIN );
            GPIO_SetBits( MOT2_DIRB_GPIO,MOT2_DIRB_PIN );
         }
         else
         {
            GPIO_SetBits( MOT2_DIRA_GPIO,MOT2_DIRA_PIN );
            GPIO_ResetBits( MOT2_DIRB_GPIO,MOT2_DIRB_PIN );
         }
         TIM_MOTORS->MOT2_PWM_CHANNEL = Sign ? (uint16_t) Value : (uint16_t) -Value;;
         break;
      default:
         break;
      }
   }
}

/*!
 * Value may vary between -180 and +180.0. Resolution 0.5.
 */
static void priv_ServoSetAngle(ServoSelector_T ServoSelector, float Angle)
{
   if( -180 > Angle ) Angle = -180;
   if(  180 < Angle ) Angle =  180;

   Angle = (uint16_t) 2*(Angle)+540;

   switch (ServoSelector)
   {
   case SelectServoArmLeft:
      priv_SetAngleArmLeft(Angle);
      break;
   case SelectServoArmRight:
      priv_SetAngleArmRight(Angle);
      break;
   case SelectServoCamHor:
      priv_SetAngleCamHor(Angle);
      break;
   case SelectServoCamVer:
      priv_SetAngleCamVer(Angle);
      break;
   default:
      break;
   }
}

static void priv_SetAngleArmLeft(uint16_t Angle)
{
   TIM_SERVOS->SERVO_ARM_L_PWM_CHANNEL = Angle;
}

static void priv_SetAngleArmRight(uint16_t Angle)
{
   TIM_SERVOS->SERVO_ARM_P_PWM_CHANNEL = Angle;
}

static void priv_SetAngleCamHor(uint16_t Angle)
{
   TIM_SERVOS->SERVO_HOR_PWM_CHANNEL = Angle;
}

static void priv_SetAngleCamVer(uint16_t Angle)
{
   TIM_SERVOS->SERVO_VER_PWM_CHANNEL = Angle;
}

static float priv_EncoderPerform( EncoderParameters_T *pkThis )
{
   /*! How many ticks since last iteration */
   int16_t DeltaTicks = GetCounter( pkThis->TIMx ) - CounterDef;

   /*! Calculate angular speed of shaft */
   pkThis->Omega = ( DeltaTicks * RpmPerTIck ) / pkThis->Dt; /*!< Shaft Omega [RPM] */

   /*! Reset counter so it cannot get out of range */
   SetCounter( pkThis->TIMx, CounterDef );

   return ( pkThis->Omega );
}

static float priv_GetOmega( EncoderParameters_T *pkThis )
{
   return pkThis->Omega;
}

//-----------------------Public functions------------------------------//
void InitializeEncoders()
{
   /*! Physical initialization */
   InitializeRCC( DriverSelectEncoders );
   InitializeTimers( TIM_ENC1 );
   InitializeTimers( TIM_ENC2 );
   InitializeGPIO( DriverSelectEncoders );

   /*! Software */
   oEncoderLeft.Parameters.Dt = DT_slow;
   oEncoderLeft.Parameters.TIMx = TIM_ENC1;
   oEncoderLeft.Parameters.Counter = CounterDef;
   oEncoderLeft.Parameters.Omega = 0u;
   oEncoderLeft.Perform = priv_EncoderPerform;
   oEncoderLeft.GetOmega = priv_GetOmega;
   oEncoderLeft.SetCounter = SetCounter; /*! Timer function */

   oEncoderRight.Parameters.Dt = DT_slow;
   oEncoderRight.Parameters.TIMx = TIM_ENC2;
   oEncoderRight.Parameters.Counter = CounterDef;
   oEncoderRight.Parameters.Omega = 0u;
   oEncoderRight.Perform = priv_EncoderPerform;
   oEncoderRight.GetOmega = priv_GetOmega;
   oEncoderRight.SetCounter = SetCounter; /*! Timer function */
}

void InitializeMotors()
{
   /*! Physical initialization */
   InitializeRCC(DriverSelectMotors);
   InitializeTimers(TIM_MOTORS);
   InitializeGPIO(DriverSelectMotors);

   /*! Software */
   InitializePIDs();
   oMotor.SetSpeed = priv_MotorSetSpeed;
}

void InitializeServos()
{
   /*! Physical initialization */
   InitializeRCC(DriverSelectServosArm);
   InitializeTimers(TIM_SERVOS);
   InitializeGPIO(DriverSelectServosArm);

   /*! Physical initialization */
   InitializeRCC(DriverSelectServosCam);
   InitializeTimers(TIM_SERVOS);
   InitializeGPIO(DriverSelectServosCam);

   /*! Software */
   oServosArm.SetAngle = priv_ServoSetAngle;
}

void InitializePIDs()
{
   PID_Initialize( &oPID_Angle );
   oPID_Angle.SetKp( &oPID_Angle.Parameters, 40.0f );
   oPID_Angle.SetKi( &oPID_Angle.Parameters, 0.0f );
   oPID_Angle.SetKd( &oPID_Angle.Parameters, 17.0f );
   oPID_Angle.Parameters.MaxOutSignal = 1000.0f; /*!< Max output PWM = 1000. */
   oPID_Angle.Parameters.iWindUp = 0;

   PID_Initialize( &oPID_Omega );
   oPID_Omega.SetKp( &oPID_Omega.Parameters, 0.005f );
   oPID_Omega.SetKi( &oPID_Omega.Parameters, 0.095f );
   oPID_Omega.SetKd( &oPID_Omega.Parameters, 0.09f );
   oPID_Omega.Parameters.MaxOutSignal = 35.0f; /*!< Max output angle = 30deg. */
   oPID_Omega.Parameters.iWindUp = 150;
}


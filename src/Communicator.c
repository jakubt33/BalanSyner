/*
 * Communicator.c
 *
 *  Created on: Nov 22, 2015
 *      Author: Kuba
 */

//-----------------------Includes-------------------------------------//
#include "stm32f30x.h"
#include "Communicator.h"
#include "RobotStates.h"

#include "../Drivers/LEDs/LED.h"
#include "../Drivers/BT/BT.h"
#include "../Drivers/MPU/MPU.h"
#include "../Drivers/Motors/Motors.h"

#include "../Framework/PID/PID.h"

//-----------------------Private defines-------------------------------//
#define START_BYTE_DEF           0xFF
#define COMMAND_LENGTH           8
#define PARITY_BIT_TEMP          1

//-----------------------Private macros--------------------------------//

//-----------------------Private typedefs------------------------------//

//-----------------------Private variables-----------------------------//
extern RobotProperties_T kRobotProperties;

//-----------------------Private prototypes----------------------------//
void priv_SendDummy();
uint8_t priv_CheckParityBits();
static float priv_CommandToFloat( uint8_t *Command );

static void priv_ReadKalmanQAngle();
static void priv_ReadKalmanRMeasure();
static void priv_ReadFilteredAngle();
static void priv_ReadRawAngle();
static void priv_ReadOmegaLeft();
static void priv_ReadOmegaRight();
static void priv_ReadPidAngleKp();
static void priv_ReadPidAngleKi();
static void priv_ReadPidAngleKd();
static void priv_ReadPidOmegaKp();
static void priv_ReadPidOmegaKi();
static void priv_ReadPidOmegaKd();
static void privReadPidDstOmega();

static void priv_WriteKalmanQAngle( uint8_t *Command );
static void priv_WriteKalmanQAngleDef();
static void priv_WriteKalmanRMeasure( uint8_t *Command );
static void priv_WriteKalmanRMeasureDef();
static void priv_WritePidAngleKp( uint8_t *Command );
static void priv_WritePidAngleKi( uint8_t *Command );
static void priv_WritePidAngleKd( uint8_t *Command );
static void priv_WritePidOmegaKp( uint8_t *Command );
static void priv_WritePidOmegaKi( uint8_t *Command );
static void priv_WritePidOmegaKd( uint8_t *Command );
static void priv_WritePidRotationKp( uint8_t *Command );
static void priv_WritePidRotationKi( uint8_t *Command );
static void priv_WritePidRotationKd( uint8_t *Command );
static void priv_WritePidDstOmega( uint8_t *Command );
static void priv_WritePidDstRotation( uint8_t *Command );
static void priv_WriteArmAngle( uint8_t *Command );
static void priv_WriteSerHor( uint8_t *Command );
static void priv_WriteSerVer( uint8_t *Command );

static void priv_AutoBalance();
static void priv_AutoStandsUp();
static void priv_AutoLiesDown();

//-----------------------Private functions-----------------------------//
void priv_SendDummy()
{
   pub_SendCommandBT( 0.0f, 255u );
}

uint8_t priv_CheckParityBits()
{
   return 1;
}

static float priv_CommandToFloat( uint8_t *Command )
{
   uint32_t transport_bits = *( ( uint32_t* ) Command );
   return ( *( ( float* ) & transport_bits ) );
}

/*!
 *-------------------------------------------------------------------------------------
 ********************************    READ FUNCTIONS    ********************************
 *-------------------------------------------------------------------------------------
 *       The functions read state of robot and send back this values via USART
 */
static void priv_ReadKalmanQAngle()
{
   pub_SendCommandBT( oMpuKalman.GetKalmanQAngle(), ReadKalmanQAngle );
}

static void priv_ReadKalmanRMeasure()
{
   pub_SendCommandBT( oMpuKalman.GetKalmanRMeasure(), ReadKalmanRMeasure );
}

static void priv_ReadFilteredAngle()
{
   pub_SendCommandBT( oMpuKalman.AngleFiltered, ReadFilteredAngle );
}

static void priv_ReadRawAngle()
{
   pub_SendCommandBT( oMpuKalman.AngleRaw, ReadRawAngle );
}

static void priv_ReadOmegaLeft()
{
   pub_SendCommandBT( oEncoders.GetOmegaLeft(), ReadOmegaLeft );
}

static void priv_ReadOmegaRight()
{
   pub_SendCommandBT( oEncoders.GetOmegaRight(), ReadOmegaRight );
}

static void priv_ReadPidAngleKp()
{
   pub_SendCommandBT( oPID_Angle.GetKp( &oPID_Angle.Parameters ), ReadPidAngleKp );
}

static void priv_ReadPidAngleKi()
{
   pub_SendCommandBT( oPID_Angle.GetKi( &oPID_Angle.Parameters ), ReadPidAngleKi );
}

static void priv_ReadPidAngleKd()
{
   pub_SendCommandBT( oPID_Angle.GetKd( &oPID_Angle.Parameters ), ReadPidAngleKd );
}

static void priv_ReadPidOmegaKp()
{
   pub_SendCommandBT( oPID_Omega.GetKp( &oPID_Omega.Parameters ), ReadPidOmegaKp );
}

static void priv_ReadPidOmegaKi()
{
   pub_SendCommandBT( oPID_Omega.GetKi( &oPID_Omega.Parameters ), ReadPidOmegaKi );
}

static void priv_ReadPidOmegaKd()
{
   pub_SendCommandBT( oPID_Omega.GetKd( &oPID_Omega.Parameters ), ReadPidOmegaKd );
}

static void privReadPidDstOmega()
{
   pub_SendCommandBT( oPID_Omega.GetDstValue( &oPID_Omega.Parameters ), ReadPidDstOmega );
}
/*!
 *-------------------------------------------------------------------------------------
 ********************************    WRITE FUNCTIONS    *******************************
 *-------------------------------------------------------------------------------------
 *          The functions read new values from USART and applies them to robot
 */

static void priv_WriteKalmanQAngle( uint8_t *Command )
{
   oMpuKalman.SetKalmanQAngle( priv_CommandToFloat( Command ) );
}

static void priv_WriteKalmanQAngleDef()
{
   oMpuKalman.SetKalmanQAngleDef();
}

static void priv_WriteKalmanRMeasure( uint8_t *Command )
{
   oMpuKalman.SetKalmanRMeasure( priv_CommandToFloat( Command ) );
}

static void priv_WriteKalmanRMeasureDef()
{
   oMpuKalman.SetKalmanRMeasureDef();
}

static void priv_WritePidAngleKp( uint8_t *Command )
{
   oPID_Angle.SetKp( &oPID_Angle.Parameters, priv_CommandToFloat( Command ) );
}

static void priv_WritePidAngleKi( uint8_t *Command )
{
   oPID_Angle.SetKi( &oPID_Angle.Parameters, priv_CommandToFloat( Command ) );
}

static void priv_WritePidAngleKd( uint8_t *Command )
{
   oPID_Angle.SetKd( &oPID_Angle.Parameters, priv_CommandToFloat( Command ) );
}

static void priv_WritePidOmegaKp( uint8_t *Command )
{
   oPID_Omega.SetKp( &oPID_Omega.Parameters, priv_CommandToFloat( Command ) );
}

static void priv_WritePidOmegaKi( uint8_t *Command )
{
   oPID_Omega.SetKi( &oPID_Omega.Parameters, priv_CommandToFloat( Command ) );
}

static void priv_WritePidOmegaKd( uint8_t *Command )
{
   oPID_Omega.SetKd( &oPID_Omega.Parameters, priv_CommandToFloat( Command ) );
}

static void priv_WritePidRotationKp( uint8_t *Command )
{
   oPID_Rotation.SetKp( &oPID_Rotation.Parameters, priv_CommandToFloat( Command ) );
}

static void priv_WritePidRotationKi( uint8_t *Command )
{
   oPID_Rotation.SetKi( &oPID_Rotation.Parameters, priv_CommandToFloat( Command ) );
}

static void priv_WritePidRotationKd( uint8_t *Command )
{
   oPID_Rotation.SetKd( &oPID_Rotation.Parameters, priv_CommandToFloat( Command ) );
}

static void priv_WritePidDstOmega( uint8_t *Command )
{
   oPID_Omega.SetDstValue( &oPID_Omega.Parameters,
                           oMotors.GetNewSpeedDst( priv_CommandToFloat( Command ) )
                         );
}

static void priv_WritePidDstRotation( uint8_t *Command )
{
   oPID_Rotation.SetDstValue( &oPID_Rotation.Parameters,
                              oMotors.GetNewRotationDst( priv_CommandToFloat( Command ) )
                            );
}

static void priv_WriteArmAngle( uint8_t *Command )
{
   oServos.SetAngleArmLeft( priv_CommandToFloat( Command ) );
   oServos.SetAngleArmRight( priv_CommandToFloat( Command ) );
}

static void priv_WriteSerHor( uint8_t *Command )
{
   oServos.SetAngleCamHor( priv_CommandToFloat( Command ) );
}

static void priv_WriteSerVer( uint8_t *Command )
{
   oServos.SetAngleCamVer( priv_CommandToFloat( Command ) );
}

static void priv_AutoBalance()
{
   kRobotProperties.StateUserRequested = RobotStateUser_Balancing;
}

static void priv_AutoStandsUp()
{
   kRobotProperties.StateUserRequested = RobotStateUser_StandsUp;
}

static void priv_AutoLiesDown()
{
   kRobotProperties.StateUserRequested = RobotStateUser_LiesDown;
}

//-----------------------Public functions------------------------------//
void pub_SendCommandBT( float Value, Addresses_T Address )
{
   uint8_t Command[] = { 0xFF, 0xFF, Address, 0, 0, 0, 0, PARITY_BIT_TEMP };

   uint32_t transport_bits = *( ( uint32_t* ) &Value );
   *(uint32_t *) &Command[3] = transport_bits;

   uint8_t i = 0;
   while ( COMMAND_LENGTH > i )
   {
      oBluetooth.PushFifo( &oBluetooth.oBtTxFifo, *( Command + i++ ) );
   }
   oBluetooth.SendFifo();
}

/*!
 * fn:            Communicator_CheckInputs
 * Description:   This function checks whether external command came. The protocol is as follows:
 *    --------------------------------------------------
 *    | SB | SB | Adr | Val1 | Val2 | Val3 | Val4 | PB |
 *    --------------------------------------------------
 *
 *    SB - start byte = 255
 *
 *    Adr: TODO:Update list. Temporarily look at enum Addresses.
 *       000 - Read&Send Kalman Q_Angle (gyro variance)
 *       001 - Read&Send Kalman R_measure (accelerometer variance)
 *
 *       100 - Write new Kalman Q_Angle (gyro variance)
 *       101 - Write new Kalman R_measure (accelerometer variance)
 *
 *    Valx: uint32_t value or float value
 *       Val1:MSB - - - Val4:LSB
 *
 *       float source_float = 1234.5678f ;
 *       uint32_t transport_bits = *((uint32_t*)&source_float);
 *       float destination_float = *((float*)&transport_bits);
 *
 *    PB - parity check TODO: implement some
 */
uint8_t Communicator_CheckInputs()
{
   USART2->CR1 &= ~USART_CR1_RXNEIE;
   //priv_SendDummy();
   /*! if uSafetyCounter equals 0, it means that connection is not established */
   uint8_t uSafetyCounter = 0u;
   while( 0u != oBluetooth.IsFifoEmpty( &oBluetooth.oBtRxFifo ) )
   {
      /*!
       * Check for first start byte
       */
      uint8_t BufferSize = 0u;
      uint8_t StartByte = 0u;
      do
      {
         BufferSize = oBluetooth.PopFifo( &oBluetooth.oBtRxFifo, &StartByte );
      } while ( ( START_BYTE_DEF != StartByte ) && ( 6u < BufferSize ) );

      /*!
       *  Check for second start byte.
       */
      if( START_BYTE_DEF == StartByte )
      {
         StartByte = 0u;
         BufferSize = oBluetooth.PopFifo( &oBluetooth.oBtRxFifo, &StartByte );

         if( ( START_BYTE_DEF == StartByte ) && ( 5u < BufferSize ) )
         {
            uint8_t Command[6];
            uint8_t Counter;
            /*! Copy buffer to Command array */
            for( Counter = 0u; Counter < 6u; Counter++ )
            {
               oBluetooth.PopFifo( &oBluetooth.oBtRxFifo, &Command[Counter] );
            }

            if( priv_CheckParityBits() == Command[5] )
            {
               uSafetyCounter = 1u;
               LED4_Toggle;
               switch( Command[0] )
               {
                  case ReadKalmanQAngle:
                     priv_ReadKalmanQAngle();
                     break;
                  case ReadKalmanRMeasure:
                     priv_ReadKalmanRMeasure();
                     break;
                  case ReadFilteredAngle:
                     priv_ReadFilteredAngle();
                     break;
                  case ReadRawAngle:
                     priv_ReadRawAngle();
                     break;
                  case ReadOmegaLeft:
                     priv_ReadOmegaLeft();
                     break;
                  case ReadOmegaRight:
                     priv_ReadOmegaRight();
                     break;
                  case ReadPidAngleKp:
                     priv_ReadPidAngleKp();
                     break;
                  case ReadPidAngleKi:
                     priv_ReadPidAngleKi();
                     break;
                  case ReadPidAngleKd:
                     priv_ReadPidAngleKd();
                     break;
                  case ReadPidOmegaKp:
                     priv_ReadPidOmegaKp();
                     break;
                  case ReadPidOmegaKi:
                     priv_ReadPidOmegaKi();
                     break;
                  case ReadPidOmegaKd:
                     priv_ReadPidOmegaKd();
                     break;
                  case ReadPidDstOmega:
                     privReadPidDstOmega();
                     break;

                  case WriteKalmanQAngleDef:
                     priv_WriteKalmanQAngleDef();
                     break;
                  case WriteKalmanQAngle:
                     priv_WriteKalmanQAngle( &Command[1] ); //send address of first value byte
                     break;
                  case WriteKalmanRMeasureDef:
                     priv_WriteKalmanRMeasureDef();
                     break;
                  case WriteKalmanRMeasure:
                     priv_WriteKalmanRMeasure( &Command[1] );
                     break;
                  case WritePidAngleKp:
                     priv_WritePidAngleKp( &Command[1] );
                     break;
                  case WritePidAngleKi:
                     priv_WritePidAngleKi( &Command[1] );
                     break;
                  case WritePidAngleKd:
                     priv_WritePidAngleKd( &Command[1] );
                     break;
                  case WritePidOmegaKp:
                     priv_WritePidOmegaKp( &Command[1] );
                     break;
                  case WritePidOmegaKi:
                     priv_WritePidOmegaKi( &Command[1] );
                     break;
                  case WritePidOmegaKd:
                     priv_WritePidOmegaKd( &Command[1] );
                     break;
                  case WritePidRotationKp:
                     priv_WritePidRotationKp( &Command[1] );
                     break;
                  case WritePidRotationKi:
                     priv_WritePidRotationKi( &Command[1] );
                     break;
                  case WritePidRotationKd:
                     priv_WritePidRotationKd( &Command[1] );
                     break;
                  case WritePidDstOmega:
                     priv_WritePidDstOmega( &Command[1] );
                     break;
                  case WritePidDstRotation:
                     priv_WritePidDstRotation( &Command[1] );
                     break;
                  case WriteArmAngle:
                     priv_WriteArmAngle( &Command[1] );
                     break;
                  case WriteSerHor:
                     priv_WriteSerHor( &Command[1] );
                     break;
                  case WriteSerVer:
                     priv_WriteSerVer( &Command[1] );
                     break;

                  case AutoBalance:
                     priv_AutoBalance();
                     break;
                  case AutoStandsUp:
                     priv_AutoStandsUp();
                     break;
                  case AutoLiesDown:
                     priv_AutoLiesDown();
                     break;

                  default:
                     break;
               }
            }
         }
      }
   }
   USART2->CR1 |= USART_CR1_RXNEIE;
   return uSafetyCounter;
}


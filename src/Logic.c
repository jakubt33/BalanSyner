/*
 * Logic.c
 *
 *  Created on: Nov 22, 2015
 *      Author: Kuba
 */

//-----------------------Includes-------------------------------------//
#include "stm32f30x.h"
#include "Logic.h"

#include "../Drivers/BT/BT.h"
#include "../Drivers/MPU/MPU.h"
#include "../Drivers/Motors/Motors.h"

//-----------------------Private defines-------------------------------//
#define START_BYTE_DEF           0xFF
#define COMMAND_LENGTH           8
#define PARITY_BIT_TEMP          1
//-----------------------Private macros--------------------------------//

//-----------------------Private typedefs------------------------------//
typedef enum
{
   ReadKalmanQAngle     = 0u,
   ReadKalmanRMeasure   = 1u,
   ReadOmegaLeft        = 2u,
   ReadOmegaRight       = 3u,

   WriteKalmanQAngleDef    = 100u,
   WriteKalmanQAngle       = 101u,
   WriteKalmanRMeasureDef  = 102u,
   WriteKalmanRMeasure     = 103u,

}Addresses_T;
//-----------------------Private variables-----------------------------//

//-----------------------Private prototypes----------------------------//
void priv_SendDummy();
uint8_t priv_CheckParityBits();
static void priv_SendCommandBT( uint8_t *Command );

static void priv_ReadKalmanQAngle();
static void priv_ReadKalmanRMeasure();
static void priv_ReadOmegaLeft();
static void priv_ReadOmegaRight();

static void priv_WriteKalmanQAngle( uint8_t *Command );
static void priv_WriteKalmanQAngleDef();
static void priv_WriteKalmanRMeasure( uint8_t *Command );
static void priv_WriteKalmanRMeasureDef();

//-----------------------Private functions-----------------------------//
void priv_SendDummy()
{
   uint8_t Command[] = { 0xFF, 0xFF, 255, 0, 0, 0, 0, PARITY_BIT_TEMP };
   priv_SendCommandBT( Command );
}

uint8_t priv_CheckParityBits()
{
   return 1;
}

static void priv_SendCommandBT( uint8_t *Command )
{
   uint8_t i = 0;
   while ( COMMAND_LENGTH > i )
   {
      oBluetooth.PushFifo( &oBluetooth.oBtTxFifo, *( Command + i++ ) );
   }
   oBluetooth.SendFifo();
}

static void priv_ReadKalmanQAngle()
{
   uint8_t Command[] = { 0xFF, 0xFF, ReadKalmanQAngle, 0, 0, 0, 0, PARITY_BIT_TEMP };

   float Value = oMpuKalman.GetKalmanQAngle();

   uint32_t transport_bits = *( ( uint32_t* ) &Value );
   *(uint32_t *) &Command[3] = transport_bits;

   priv_SendCommandBT( Command );
}

static void priv_ReadKalmanRMeasure()
{
   uint8_t Command[] = { 0xFF, 0xFF, ReadKalmanRMeasure, 0, 0, 0, 0, PARITY_BIT_TEMP };

   float Value = oMpuKalman.GetKalmanRMeasure();

   uint32_t transport_bits = *( ( uint32_t* ) &Value );
   *(uint32_t *) &Command[3] = transport_bits;

   priv_SendCommandBT( Command );
}

static void priv_ReadOmegaLeft()
{
   oEncoderLeft.GetOmega( &oEncoderLeft.Parameters );
}

static void priv_ReadOmegaRight()
{
   oEncoderRight.GetOmega( &oEncoderRight.Parameters );
}

static void priv_WriteKalmanQAngle( uint8_t *Command )
{
   uint32_t transport_bits = *( ( uint32_t* ) Command );
   float destination_float = *( ( float* ) & transport_bits );

   oMpuKalman.SetKalmanQAngle( destination_float );
}

static void priv_WriteKalmanQAngleDef()
{
   oMpuKalman.SetKalmanQAngleDef();
}

static void priv_WriteKalmanRMeasure( uint8_t *Command )
{
   //TODO:check it
   uint32_t transport_bits = *( ( uint32_t* ) Command );
   float destination_float = *( ( float* ) & transport_bits );

   oMpuKalman.SetKalmanRMeasure( destination_float );
}

static void priv_WriteKalmanRMeasureDef()
{
   oMpuKalman.SetKalmanRMeasureDef();
}

//-----------------------Public functions------------------------------//
/*!
 * fn:            Logic_CheckInputs
 * Description:   This function checks whether external command came. The protocol is as follows:
 *    --------------------------------------------------
 *    | SB | SB | Adr | Val1 | Val2 | Val3 | Val4 | PB |
 *    --------------------------------------------------
 *
 *    SB - start byte = 255
 *
 *    Adr: TODO:Update list. Temporarly look at enum Addresses.
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
#include "../Drivers/LEDs/LED.h"
void Logic_CheckInputs()
{
   priv_SendDummy();
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
            for( Counter = 0u ;  Counter < 6u; Counter++ )
            {
               oBluetooth.PopFifo( &oBluetooth.oBtRxFifo, &Command[ Counter ] );
            }

            if( priv_CheckParityBits() == Command[5] )
            {
               /*! Everything is ok. */
               LED_NUCLEO_IsOn ? LED_Nucleo_SetOn : LED_Nucleo_SetOff;

               switch( Command[0] )
               {
                  case ReadKalmanQAngle:
                     priv_ReadKalmanQAngle();
                     break;
                  case ReadKalmanRMeasure:
                     priv_ReadKalmanRMeasure();
                     break;
                  case ReadOmegaLeft:
                     priv_ReadOmegaLeft();
                     break;
                  case ReadOmegaRight:
                     priv_ReadOmegaRight();
                     break;
                  case WriteKalmanQAngleDef:
                     priv_WriteKalmanQAngleDef();
                     break;
                  case WriteKalmanQAngle:
                     priv_WriteKalmanQAngle( &Command[1] ); //send addres of first value byte
                     break;
                  case WriteKalmanRMeasureDef:
                     priv_WriteKalmanRMeasureDef();
                     break;
                  case WriteKalmanRMeasure:
                     priv_WriteKalmanRMeasure( &Command[1] );
                     break;
                  default:
                     break;
               }
            }
         }
      }
   }
}

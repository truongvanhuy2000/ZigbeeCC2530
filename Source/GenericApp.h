/**************************************************************************************************
  Filename:       GenericApp.h
  Revised:        $Date: 2012-02-12 15:58:41 -0800 (Sun, 12 Feb 2012) $
  Revision:       $Revision: 29216 $

  Description:    This file contains the Generic Application definitions.


  Copyright 2004-2012 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef GENERICAPP_H
#define GENERICAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
//#include "ZDObject.h"
#include "ZDProfile.h"
#include "ZComDef.h"

#include "tiny-json.h"
#include "onboard.h"
#include "DebugTrace.h"

/* HAL */
#include "hal_uart.h"
#include "NLMEDE.h"
#include "OSAL_Nv.h"
#include "hal_adc.h"
#include "hal_led.h"

/*********************************************************************
 * CONSTANTS
 */

// These constants are only for example and should be changed to the
// device's needs
#define GENERICAPP_ENDPOINT 10

#define GENERICAPP_PROFID 0x0F04
#define GENERICAPP_DEVICEID 0x0001
#define GENERICAPP_DEVICE_VERSION 0
#define GENERICAPP_FLAGS 0

#define GENERICAPP_MAX_CLUSTERS 3
#define GENERICAPP_CLUSTERID 1

// Send Message Timeout
#define GENERICAPP_SEND_MSG_TIMEOUT 5000 // Every 5 seconds

#define GENERICAPP_ORPHAN_TIME 0x0020
#define GENERICAPP_TIMEINTERVAL 0x0045
#define GENERICAPP_UPDATEADDR 5
#define SERIAL_APP_PORT 0
#define SERIAL_APP_BAUD HAL_UART_BR_38400
#define SERIAL_APP_THRESH 64
#define SERIAL_APP_RX_SZ 128
#define SERIAL_APP_TX_SZ 128
#define SERIAL_APP_IDLE 6
#define SERIAL_APP_TX_MAX 80
#define SERIAL_APP_RSP_CNT 4
#define DATA_LENGTH 50

#define LIGHT_OFF 0
#define LIGHT_ON 1
#define DEFAULT_TIME 1
#define DEFAULT_DIMMER 0

#define NWK_BROADCAST_SHORTADDR_DEVALL 0xFFFF
#define NWK_BROADCAST_SHORTADDR_DEVRXON 0xFFFD
#define NWK_BROADCAST_SHORTADDR_DEVZCZR 0xFFFC

#define NWK_COORDINATOR_ADDRESS 0x0000

#if defined(IAR_ARMCM3_LM)
#define GENERICAPP_RTOS_MSG_EVT 0x0002
#endif

#define TYPE_DEVICE_INDENTITY 0
#define TYPE_DEVICE_SETTING 1
#define TYPE_DEVICE_DATA 2
  /*********************************************************************
   * CONSTANTS
   */

  /*********************************************************************
   * TYPEDEFS
   */
  typedef struct
  {
    uint8 id;
    uint8 lightStatus;
    uint8 dimmer;
    uint8 time;
    uint16 adcValue;
  } GenericAppData;

  extern GenericAppData appData;

#ifdef COOR
  typedef struct
  {
    uint8 extAddr[8];
    uint16 shortAddr;
    uint8 id;
    /* data */
  } deviceInfo;

  struct deviceNode
  {
    deviceInfo device;
    struct deviceNode *next;
  };

  extern struct deviceNode *firstDevice;
#endif
  /*********************************************************************
   * GLOBAL VARIABLES
   */
  // This list should be filled with Application specific Cluster IDs.
  extern const cId_t GenericApp_ClusterList[3];

  extern const SimpleDescriptionFormat_t GenericApp_SimpleDesc;

  // This is the Endpoint/Interface description.  It is defined here, but
  // filled-in in GenericApp_Init().  Another way to go would be to fill
  // in the structure here and make it a "const" (in code space).  The
  // way it's defined in this sample app it is define in RAM.
  extern endPointDesc_t GenericApp_epDesc;
  /*********************************************************************
   * EXTERNAL VARIABLES
   */

  /*********************************************************************
   * EXTERNAL FUNCTIONS
   */

  /*********************************************************************
   * LOCAL VARIABLES
   */

  extern byte GenericApp_TaskID; // Task ID for internal task/event processing
                                 // This variable will be received when
                                 // GenericApp_Init() is called.
  extern devStates_t GenericApp_NwkState;
  extern byte GenericApp_TransID; // This is the unique message ID (counter)

  extern afAddrType_t GenericApp_CoorAddr;

  extern halUARTCfg_t uartConfig;

  /*********************************************************************
   * MACROS
   */

  /*********************************************************************
   * FUNCTIONS
   */

  /*
   * Task Initialization for the Generic Application
   */
  extern void GenericApp_Init(byte task_id);

  /*
   * Task Event Processor for the Generic Application
   */
  extern UINT16 GenericApp_ProcessEvent(byte task_id, UINT16 events);
  // utilities function
  extern int processSending(uint8 *data, GenericAppData *tempData);
  extern ZStatus_t sendData(afAddrType_t *arr, GenericAppData *data);
  extern int countValidData(char *data);
  extern void convertHexToArray(const char *hexString, char *arr);
  extern void exceptionHandler(char *exception);

  extern void GenericApp_ProcessZDOMsgs(zdoIncomingMsg_t *inMsg);
  // static void GenericApp_HandleKeys(byte shift, byte keys);
  extern void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pckt);
  extern ZStatus_t GenericApp_SendTheMessage(afAddrType_t *addr, uint16 clusterId, uint8 *data, int len);
  extern void delayMs(int time);
  extern void endDeviceOperation(GenericAppData *data);
  /*********************************************************************
  *********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* GENERICAPP_H */

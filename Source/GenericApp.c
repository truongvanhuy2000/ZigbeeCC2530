/******************************************************************************
  Filename:       GenericApp.c
  Revised:        $Date: 2012-03-07 01:04:58 -0800 (Wed, 07 Mar 2012) $
  Revision:       $Revision: 29656 $

  Description:    Generic Application (no Profile).


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
******************************************************************************/

/*********************************************************************
  This application isn't intended to do anything useful, it is
  intended to be a simple example of an application's structure.

  This application sends "Hello World" to another "Generic"
  application every 5 seconds.  The application will also
  receives "Hello World" packets.

  The "Hello World" messages are sent/received as MSG type message.

  This applications doesn't have a profile, so it handles everything
  directly - itself.

  Key control:
    SW1:
    SW2:  initiates end device binding
    SW3:
    SW4:  initiates a match description request
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "GenericApp.h"
#include "ioCC2530.h"
#include <stdio.h>
#ifdef COOR
#include "Coordinator.h"
#else
#include "EndDevice.h"
#endif
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
GenericAppData appData = {0xFF, LIGHT_OFF, DEFAULT_DIMMER, DEFAULT_TIME, 0};

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
    {
        GENERICAPP_CLUSTERID, NWK_addr_rsp, NWK_addr_req};

const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
    {
        GENERICAPP_ENDPOINT,             //  int Endpoint;
        GENERICAPP_PROFID,               //  uint16 AppProfId[2];
        GENERICAPP_DEVICEID,             //  uint16 AppDeviceId[2];
        GENERICAPP_DEVICE_VERSION,       //  int   AppDevVer:4;
        GENERICAPP_FLAGS,                //  int   AppFlags:4;
        GENERICAPP_MAX_CLUSTERS,         //  byte  AppNumInClusters;
        (cId_t *)GenericApp_ClusterList, //  byte *pAppInClusterList;
        GENERICAPP_MAX_CLUSTERS,         //  byte  AppNumInClusters;
        (cId_t *)GenericApp_ClusterList  //  byte *pAppInClusterList;
};

endPointDesc_t GenericApp_epDesc;
/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

byte GenericApp_TaskID; // Task ID for internal task/event processing
                        // This variable will be received when
                        // GenericApp_Init() is called.
devStates_t GenericApp_NwkState;
byte GenericApp_TransID; // This is the unique message ID (counter)

afAddrType_t GenericApp_CoorAddr;

halUARTCfg_t uartConfig;

struct deviceNode *firstDevice = NULL;

uint8 timeState;

static uint16 checkAddr = 0x0420;
static uint16 dataAddr = 0x0425;
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void uartInit();
void nvStartupOperation();
void nvWriteOperation();
int processMsgToUart(char *msg, char *data);
void init_ADC0();
void createPWM(uint8 dutyCycle);

static void SerialApp_CallBack(uint8 port, uint8 event);

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */
/*********************************************************************
 * PUBLIC FUNCTIONS
 */
/*********************************************************************
 * @fn      GenericApp_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void GenericApp_Init(uint8 task_id)
{
  HalLedSet(HAL_LED_ALL, HAL_LED_MODE_ON);
  GenericApp_TaskID = task_id;
  GenericApp_NwkState = DEV_INIT;
  GenericApp_TransID = 0;

  // Device hardware initialization can be added here or in main() (Zmain.c).
  // If the hardware is application specific - add it here.
  // If the hardware is other parts of the device add it in main().

  GenericApp_CoorAddr.addrMode = (afAddrMode_t)Addr16Bit;
  GenericApp_CoorAddr.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_CoorAddr.addr.shortAddr = NWK_COORDINATOR_ADDRESS;

  // Fill out the endpoint description.
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;
#ifdef COOR
  // delayMs(20000);
#endif
#ifdef ED
  nvStartupOperation();
  changeDimmerValue(&appData);
#endif
  uartInit();
  init_ADC0();
  // HalLedInit();
  //  Register the endpoint description with the AF
  afRegister(&GenericApp_epDesc);

  // HalUARTWrite(SERIAL_APP_PORT, "START FOR DEBUG", 15);
  //  delay 20s to wait for rasberry pie to boot
}

/*********************************************************************
 * @fn      GenericApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 GenericApp_ProcessEvent(uint8 task_id, uint16 events)
{
  afIncomingMSGPacket_t *MSGpkt;
  afDataConfirm_t *afDataConfirm;
  (void)task_id; // Intentionally unreferenced parameter

  if (events & SYS_EVENT_MSG)
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);
    while (MSGpkt)
    {
      switch (MSGpkt->hdr.event)
      {
      case ZDO_CB_MSG:
        GenericApp_ProcessZDOMsgs((zdoIncomingMsg_t *)MSGpkt);
        break;
      case AF_INCOMING_MSG_CMD:
        GenericApp_MessageMSGCB(MSGpkt);
        break;

      case ZDO_STATE_CHANGE:
        GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
        if (GenericApp_NwkState == DEV_ZB_COORD)
          HalUARTWrite(SERIAL_APP_PORT, "Coordinator\n", 14);

#ifdef ED
        if (GenericApp_NwkState == DEV_END_DEVICE)
        {
          HalUARTWrite(SERIAL_APP_PORT, " Dev end device", 15);
          osal_start_timerEx(GenericApp_TaskID,
                             GENERICAPP_TIMEINTERVAL, 1000);
        }
        if (GenericApp_NwkState == DEV_NWK_ORPHAN)
        {
          SystemResetSoft();
        }
        break;
#endif
      default:
        break;
      }

      // Release the memory
      osal_msg_deallocate((uint8 *)MSGpkt);

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);
    }
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
#ifdef ED
  // Send a message out - This event is generated by a timer
  if (events & GENERICAPP_TIMEINTERVAL)
  {
    static int count = 0;
    count++;
    if (count % appData.time == 0)
    {
      sendData(&GenericApp_CoorAddr, &appData);
    }
    if (count % GENERICAPP_UPDATEADDR == 0)
    {
      EndDeviceUpdateAddr();
    }
    readAdcValue(&appData);
    osal_start_timerEx(GenericApp_TaskID,
                       GENERICAPP_TIMEINTERVAL, 1000);

    return (events ^ GENERICAPP_TIMEINTERVAL);
  }
#endif
  // Discard unknown events
  return 0;
}

/*********************************************************************
 * Event Generation Functions
 */

/*********************************************************************
 * @fn      GenericApp_ProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   none
 *
 * @return  none
 */
void GenericApp_ProcessZDOMsgs(zdoIncomingMsg_t *inMsg)
{
  switch (inMsg->clusterID)
  {
  }
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      GenericApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */
void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pkt)
{
  if (pkt->clusterId == GENERICAPP_CLUSTERID)
  {
    switch (GenericApp_NwkState)
    {
#ifdef COOR
    case DEV_ZB_COORD:
      if (CoordProcessMsg(pkt))
      {
        char sendData[DATA_LENGTH];
        int len = processMsgToUart(pkt->cmd.Data, sendData);
        HalUARTWrite(SERIAL_APP_PORT, sendData, len);
      }
      break;
#endif
#ifdef ED
    case DEV_END_DEVICE:
      if (EndDeviceProcessMsg(pkt))
      {
        readAdcValue(&appData);
        changeDimmerValue(&appData);
        sendData(&GenericApp_CoorAddr, &appData);
        nvWriteOperation();
      }
      break;
#endif
    default:
      break;
    }
  }
  if (pkt->clusterId == NWK_addr_rsp)
  {
#ifdef COOR
    coorProcessRecvAddr(pkt);
#endif
  }
  if (pkt->clusterId == NWK_addr_req)
  {
#ifdef ED
    if (endDeviceProcessAddrRequest(pkt))
    {
      EndDeviceUpdateAddr();
    }
#endif
  }
}

/*********************************************************************
 * @fn      GenericApp_SendTheMessage
 *
 * @brief   Send "the" message.
 *
 * @param   none
 *
 * @return  none
 */
ZStatus_t GenericApp_SendTheMessage(afAddrType_t *addr, uint16 clusterId, uint8 *data, int len)
{
  ZStatus_t status;
  status = AF_DataRequest(addr, &GenericApp_epDesc, clusterId, len,
                          data, &GenericApp_TransID, AF_DISCV_ROUTE, AF_DEFAULT_RADIUS);
  if (status != ZSuccess)
  {
    exceptionHandler("GenericApp_SendTheMessage");
    exceptionHandler((char *)status);
  }
  return status;
}

static void uartInit()
{
  uartConfig.baudRate = SERIAL_APP_BAUD;
  uartConfig.callBackFunc = SerialApp_CallBack;
  HalUARTOpen(SERIAL_APP_PORT, &uartConfig);
}

static void SerialApp_CallBack(uint8 port, uint8 event)
{
  switch (event)
  {
  case HAL_UART_RX_TIMEOUT:
#ifdef COOR
    if (GenericApp_NwkState == DEV_ZB_COORD)
    {
      uint8 *recvData = osal_mem_alloc(DATA_LENGTH * sizeof(uint8));
      HalUARTRead(port, recvData, DATA_LENGTH);
      CoordProcessUartData((char *)recvData);
      osal_mem_free(recvData);
    }
#endif
    break;
  default:
    break;
  }
}

// This func is use commonly for processing json data
int processSending(uint8 *arr, GenericAppData *tempData)
{
  json_t mem[DATA_LENGTH];
  json_t const *json = json_create((char *)arr, mem, DATA_LENGTH);

  if (!json)
  {
    exceptionHandler("processSending");
    return 0;
  }

  json_t const *status = json_getProperty(json, "ON");
  json_t const *dim = json_getProperty(json, "DI");
  json_t const *freq = json_getProperty(json, "TI");
  json_t const *sen = json_getProperty(json, "SE");

  if (status)
  {
    tempData->lightStatus = (uint8)json_getInteger(status);
  }
  if (dim)
  {
    tempData->dimmer = json_getInteger(dim);
  }
  if (freq)
  {
    tempData->time = json_getInteger(freq);
  }
  if (sen)
  {
    tempData->adcValue = json_getInteger(sen);
  }
  return 1;
}

ZStatus_t sendData(afAddrType_t *addr, GenericAppData *data)
{
  ZStatus_t status;
  int len;
  uint8 tempData[DATA_LENGTH];
  osal_memset(tempData, 0, DATA_LENGTH);
  sprintf((char *)tempData, "{\"ID\":%d,\"ON\":%d,\"DI\":%d,\"SE\":%d,\"TI\":%d}",
          data->id, data->lightStatus, data->dimmer, data->adcValue, data->time);
  len = countValidData((char *)tempData);
  status = GenericApp_SendTheMessage(addr, GENERICAPP_CLUSTERID, tempData, len);
  return status;
}

int countValidData(char *data)
{
  int count = 0;
  while (count < DATA_LENGTH)
  {
    if (data[count] == 0)
      break;
    count++;
  }
  return count;
}

void convertHexToArray(const char *hexString, char *arr)
{
  char *endptr;
  unsigned char temp_arr[2];
  for (int i = 0; i < 8; i++)
  {
    osal_memcpy(temp_arr, hexString + 2 * i, 2);
    arr[i] = strtol((const char *)temp_arr, &endptr, 16);
  }
}
void exceptionHandler(char *exception)
{
#ifdef DEBUG
  HalUARTWrite(SERIAL_APP_PORT, "\nerror with op: ", 29);
  HalUARTWrite(SERIAL_APP_PORT, (uint8 *)exception, countValidData(exception));
#endif
}
/*********************************************************************
 */
void delayMs(int time)
{
  for (int i = 0; i < time; i++)
  {
    Onboard_wait(1000);
  }
}
#ifdef ED
void nvStartupOperation()
{
  uint8 checkValue;
  void *tempPtr;
  GenericAppData *tempData;
  osal_nv_item_init(checkAddr, 1, NULL);
  osal_nv_item_init(dataAddr, 6, NULL);

  osal_nv_read(checkAddr, 0, 1, &checkValue);
  if (checkValue != 'x')
  {
    return;
  }
  osal_nv_read(dataAddr, 0, 6, tempPtr);
  tempData = (GenericAppData *)tempPtr;
  appData.id = tempData->id;
  appData.lightStatus = tempData->lightStatus;
  appData.time = tempData->time;
  appData.dimmer = tempData->dimmer;
}

void nvWriteOperation()
{
  uint8 checkValue = 'x';
  osal_nv_write(checkAddr, 0, 1, &checkValue);
  osal_nv_write(dataAddr, 0, 6, &appData);
}
#endif

int processMsgToUart(char *msg, char *data)
{
  int count;
  int j = 0;
  for (int i = 0; i < DATA_LENGTH; i++)
  {
    if (j == 0 && msg[i] == '{')
    {
      data[0] = '{';
      j++;
    }
    else if (j != 0 && msg[i] != '}')
    {
      data[j] = msg[i];
      j++;
    }
    if (msg[i] == '}')
    {
      data[j] = '}';
      break;
    }
  }
  data[j + 1] = '\n';
  count = j + 2;
  return count;
}
void init_ADC0()
{
  P0SEL |= 0x01;
  P0DIR &= ~0x01;
  APCFG |= 0x01;
}

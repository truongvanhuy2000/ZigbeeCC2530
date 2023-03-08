#ifdef ED
#ifndef END_DEVICE
#define END_DEVICE
#include "GenericApp.h"

#define HAL_LED_FLASH_TIME 10
#define HAL_LED_MAX_FLASH_COUNT 255
void EndDeviceUpdateAddr();
int EndDeviceProcessMsg(afIncomingMSGPacket_t *pkt);
int endDeviceProcessAddrRequest(afIncomingMSGPacket_t *pkt);
void updateAppdata(GenericAppData *data);
void createPWM(uint8 dutyCycle);
void changeDimmerValue(GenericAppData *data);
void readAdcValue(GenericAppData *data);
#endif
#endif
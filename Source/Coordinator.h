#ifdef COOR
#ifndef COOR_H
#define COOR_H
#include "GenericApp.h"

int CoordProcessMsg(afIncomingMSGPacket_t *pkt);
void CoordProcessUartData(char *data);
void requestDeviceNwkId(uint8 *ieeeAddr, uint8 id);
struct deviceNode *seachDevice(uint8 *id, uint8 *ieeeAddr);
void coordProcessType0(json_t const *json);
void coordProcessType1(json_t const *json, char *data, int len);
void coorProcessRecvAddr(afIncomingMSGPacket_t *pkt);
#endif

#endif
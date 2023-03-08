#ifdef COOR
#include "Coordinator.h"

int CoordProcessMsg(afIncomingMSGPacket_t *pkt)
{
    char backupData[DATA_LENGTH];
    osal_memcpy(backupData, pkt->cmd.Data, DATA_LENGTH);
    json_t mem[DATA_LENGTH];
    json_t const *json = json_create(backupData, mem, DATA_LENGTH);
    if (!json)
    {
        exceptionHandler("EndDeviceProcessMsg");
        return 0;
    }
    json_t const *id = json_getProperty(json, "ID");
    if (!id)
    {
        return 0;
    }
    const uint8 idValue = (uint8)json_getInteger(id);

    if (seachDevice((uint8 *)&idValue, NULL) == NULL)
    {
        return 0;
    }
    return 1;
}

void coorProcessRecvAddr(afIncomingMSGPacket_t *pkt)
{
    uint8 rcvExtAddr[8];
    uint16 rcvShortAddr;
    struct deviceNode *ptr;
    osal_memcpy(rcvExtAddr, pkt->cmd.Data, 8);
    rcvShortAddr = (pkt->cmd.Data[9] << 8) | pkt->cmd.Data[8];
    ptr = seachDevice(NULL, rcvExtAddr);
    if (ptr == NULL)
    {
        exceptionHandler("processRecvAddr");
        return;
    }
    // HalUARTWrite(SERIAL_APP_PORT, (uint8 *)&rcvShortAddr, 2);
    ptr->device.shortAddr = rcvShortAddr;
}

void requestDeviceNwkId(uint8 *ieeeAddr, uint8 id)
{
    afStatus_t status;
    afAddrType_t dstAddr;
    uint8 payload[9];
    osal_memcpy(payload, ieeeAddr, 8);
    payload[8] = id;
    dstAddr.addrMode = AddrBroadcast;
    dstAddr.endPoint = GENERICAPP_ENDPOINT;
    dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVALL;

    status = GenericApp_SendTheMessage(&dstAddr, NWK_addr_req, payload, 9);
    if (status != ZSuccess)
    {
        exceptionHandler("requestDeviceNwkId");
    }
    return;
}

struct deviceNode *seachDevice(uint8 *id, uint8 *ieeeAddr)
{
    struct deviceNode *ptr = firstDevice;
    if (firstDevice == NULL)
    {
        return NULL;
    }
    while (ptr != NULL)
    {
        if (id != NULL && ieeeAddr == NULL && ptr->device.id == *id)
        {
            return ptr;
        }
        if (id == NULL && ieeeAddr != NULL && osal_ExtAddrEqual(ieeeAddr, ptr->device.extAddr))
        {
            return ptr;
        }
        if (ptr->next == NULL)
        {
            break;
        }
        ptr = ptr->next;
    }
    return NULL;
}

void coordProcessType0(json_t const *json)
{
    json_t const *extAddr = json_getProperty(json, "EX");
    json_t const *id = json_getProperty(json, "ID");

    if (!id)
    {
        exceptionHandler("coordProcessType0");
        return;
    }

    if (!extAddr)
    {
        exceptionHandler("coordProcessType0");
        return;
    }

    const char *extAddrValue = json_getValue(extAddr);
    const uint8 idValue = (uint8)json_getInteger(id);
    struct deviceNode *ptr = firstDevice;
    while (ptr != NULL)
    {
        if (idValue == ptr->device.id)
        {
            ptr->device.id = idValue;
            convertHexToArray(extAddrValue, (char *)ptr->device.extAddr);
            return;
        }
        if (ptr->next == NULL)
        {
            break;
        }
        ptr = ptr->next;
    }
    struct deviceNode *device = osal_mem_alloc(sizeof(struct deviceNode));
    convertHexToArray(extAddrValue, (char *)device->device.extAddr);
    device->device.id = idValue;
    device->next = NULL;

    requestDeviceNwkId((uint8 *)device->device.extAddr, idValue);
    if (!firstDevice)
    {
        firstDevice = device;
        return;
    }
    ptr->next = device;
    return;
}

void coordProcessType1(json_t const *json, char *data, int len)
{
    json_t const *id = json_getProperty(json, "ID");
    if (!id)
    {
        exceptionHandler("coordProcessType1");
        return;
    }

    const uint8 idValue = (uint8)json_getInteger(id);
    struct deviceNode *ptr = seachDevice((uint8 *)&idValue, NULL);
    if (!ptr)
    {
        return;
    }
    afAddrType_t deviceAddr;
    deviceAddr.addrMode = Addr16Bit;
    deviceAddr.endPoint = GENERICAPP_ENDPOINT;
    deviceAddr.addr.shortAddr = ptr->device.shortAddr;
    // HalUARTWrite(SERIAL_APP_PORT, (uint8 *)&deviceAddr.addr.shortAddr, 2);

    GenericApp_SendTheMessage(&deviceAddr, GENERICAPP_CLUSTERID, data, len);
}

void CoordProcessUartData(char *data)
{
    if (GenericApp_NwkState == DEV_ZB_COORD)
    {
        char *backupData = osal_mem_alloc(DATA_LENGTH * sizeof(uint8));
        osal_memcpy(backupData, data, DATA_LENGTH);
        json_t mem[DATA_LENGTH];
        json_t const *json = json_create((char *)data, mem, DATA_LENGTH);
        if (!json)
        {
            return;
        }
        json_t const *type = json_getProperty(json, "TY");
        uint8 typeValue;
        int len;
        if (type)
        {
            typeValue = (uint8)json_getInteger(type);
        }
        switch (typeValue)
        {
        case TYPE_DEVICE_INDENTITY:
            coordProcessType0(json);
            break;
        case TYPE_DEVICE_SETTING:
            len = countValidData(backupData);
            coordProcessType1(json, backupData, len);
            osal_mem_free(backupData);
            break;
        default:
            break;
        }
    }
}

#endif
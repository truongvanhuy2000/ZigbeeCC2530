#ifdef ED

#define HAL_ADC_REF_125V 0x00 /* Internal 1.25V Reference */
#define HAL_ADC_DEC_064 0x00  /* Decimate by 64 : 8-bit resolution */
#define HAL_ADC_DEC_128 0x10  /* Decimate by 128 : 10-bit resolution */
#define HAL_ADC_DEC_512 0x30  /* Decimate by 512 : 14-bit resolution */
#define HAL_ADC_CHN_VDD3 0x0f /* Input channel: VDD/3 */
#define HAL_ADC_CHN_TEMP 0x0e /* Temperature sensor */
#define HAL_ADC_EOC 0x80      /* End of Conversion bit */

#include "EndDevice.h"
#include "ioCC2530.h"

int endDeviceProcessAddrRequest(afIncomingMSGPacket_t *pkt)
{
    uint8 rcvExtAddr[8];
    uint8 id;
    osal_revmemcpy(rcvExtAddr, pkt->cmd.Data, 8);
    id = pkt->cmd.Data[8];
    if (!osal_ExtAddrEqual(rcvExtAddr, NLME_GetExtAddr()))
    {
        exceptionHandler("endDeviceProcessAddrRequest");
        return 0;
    }
    appData.id = id;
    return 1;
}
void EndDeviceUpdateAddr()
{
    byte *extAddr;
    uint8 payload[10];
    uint16 nwkAddr;
    extAddr = NLME_GetExtAddr();
    nwkAddr = NLME_GetShortAddr();
    for (int i = 0; i < 8; i++)
    {
        payload[7 - i] = *(extAddr + i);
    }
    payload[8] = nwkAddr & 0xFF;
    payload[9] = nwkAddr >> 8;
    GenericApp_SendTheMessage(&GenericApp_CoorAddr, NWK_addr_rsp, payload, 10);
}

int EndDeviceProcessMsg(afIncomingMSGPacket_t *pkt)
{
    HalUARTWrite(SERIAL_APP_PORT, pkt->cmd.Data, pkt->cmd.DataLength);
    json_t mem[DATA_LENGTH];
    json_t const *json = json_create((char *)pkt->cmd.Data, mem, DATA_LENGTH);
    if (!json)
    {
        exceptionHandler("EndDeviceProcessMsg");
        return 0;
    }
    json_t const *id = json_getProperty(json, "ID");
    json_t const *status = json_getProperty(json, "ON");
    json_t const *dim = json_getProperty(json, "DI");
    json_t const *time = json_getProperty(json, "TI");
    if (status)
        appData.lightStatus = (uint8)json_getInteger(status);
    if (id)
        appData.id = (uint8)json_getInteger(id);
    if (dim)
        appData.dimmer = json_getInteger(dim);
    if (time)
        appData.time = json_getInteger(time);
    return 1;
}
void endDeviceOperation(GenericAppData *data)
{
    readAdcValue(data);
}
void changeDimmerValue(GenericAppData *data)
{
    if (data->dimmer != 0 && data->lightStatus == 1)
    {
        createPWM(data->dimmer);
    }
    else
        createPWM(0);
}
void readAdcValue(GenericAppData *data)
{
    data->adcValue = HalAdcRead(HAL_ADC_CHN_AIN0, HAL_ADC_RESOLUTION_12);
}
void createPWM(uint8 dutyCycle)
{
    uint32 dutyValue = 4096;
    if(dutyCycle >= 100)
    {
      dutyCycle = 99;
    }
    if(dutyCycle == 0)
    {
      dutyCycle = 100;
    }
    float percent = (float)dutyCycle / 100;
    dutyValue = dutyValue * percent;
    uint8 dutyHighBit = (dutyValue >> 8) & 0xFF;
    uint8 dutyLowBit = (dutyValue)&0xFF;

    PERCFG |= BV(6);                // Select Timer 1 Alternative 2 location
    P2DIR = (P2DIR & ~0xC0) | 0x80; // Give priority to Timer 1
    P1SEL |= BV(1);                 // Set P1_1 to peripheral

    T1CC0L = 0x00; // PWM signal period
    T1CC0H = 0x10;

    T1CC1L = dutyLowBit; // PWM duty cycle
    T1CC1H = dutyHighBit;

    T1CCTL1 = 0x1c;

    T1CTL |= (BV(2) | 0x03);
}
#endif
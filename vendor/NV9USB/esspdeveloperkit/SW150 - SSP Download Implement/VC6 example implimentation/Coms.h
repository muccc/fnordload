
#ifndef _WIN_COM_H
#define _WIN_COM_H


#include "defTypes.h"
#include "CsspUpdate.h"


typedef enum{
	COMMS_MODE_SSP,
	COMMS_MODE_SINGLE
}COMMS_MODE;


enum SERIAL_PORT_TYPE
{
	PORT_DA2,PORT_BV,PORT_UNKNOWN
};

SERIAL_PORT_TYPE GetPortType(const unsigned long portnum);


int16 OpenComPort(SSP_DOWNLOAD* sspD);

void CloseComPort(SSP_DOWNLOAD* sspD);
int16 TransmitDataPacket(SSP_PACKET* sspP);
int16 SetDownloadSpeed(SSP_DOWNLOAD* sspD,uint32 iBaud);
int16 SendComData(uint8* data, uint32 length);
int16 GetTargetChecksum(uint8 chk);
void WaitDelay(uint32 delay);

#endif


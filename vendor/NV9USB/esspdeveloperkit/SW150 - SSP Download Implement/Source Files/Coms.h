#ifndef _WIN_COM_H
#define _WIN_COM_H


#include "defTypes.h"


int16 OpenComPort(SSP_DOWNLOAD* sspD);

void CloseComPort(SSP_DOWNLOAD* sspD);
int16 TransmitDataPacket(SSP_PACKET* sspP);
int16 SetDownloadSpeed(SSP_DOWNLOAD* sspD,uint32 iBaud);
int16 SendComData(uint8* data, uint32 length);
int16 GetTargetChecksum(uint8 chk);
void WaitDelay(uint32 delay);

#endif


#ifndef _CSSP_UPDATE_H
#define _CSSP_UPDATE_H

#include "defTypes.h"


typedef enum{
	STATUS_WAIT,
	STATUS_OK,
	STATUS_ERR_TIMEOUT,
}REPLY_STATUS;

typedef enum{
	COM_PORT_CLOSED,
	COM_PORT_OPEN,
	COM_PORT_ERROR,
}COM_STATUS;


typedef enum{
	SSP_REPLY_OK,
	SSP_NO_REPLY,
	SSP_PORT_ERROR
}SSP_COMMS_STATUS;



typedef struct{
	uint8 command[255];
	uint8 commandLength;
	uint8 txPacketData[255];
	uint8 txPacketLength;
	uint8 retry;
	uint8 replyStatus;
	uint8 syncbit;
	uint8 checkStuff;
	uint8 rxPtr;
	uint8 rxBufferLength;
	uint8 rxData[255];
	uint16 timeout;
}SSP_PACKET;





typedef struct{
	int8* szFileName;
	int16 comPort;
	int16 PortStatus;
	int8 szFirmwareVersion[20];
	int8 szDatasetVersion[20];
	uint8* fileData;
}SSP_DOWNLOAD;


#define DOWNLOAD_COMPLETE				0x100000
#define OPEN_FILE_ERROR					0x100001
#define READ_FILE_ERROR					0x100002
#define NOT_ITL_FILE					0x100003
#define PORT_OPEN_FAIL					0x100004
#define SYNC_CONNECTION_FAIL			0x100005
#define	SECURITY_PROTECTED_FILE			0x100006

#define DATA_TRANSFER_FAIL				0x100010
#define PROG_COMMAND_FAIL				0x100011
#define HEADER_FAIL						0x100012
#define PROG_STATUS_FAIL				0x100013
#define PROG_RESET_FAIL					0x100014
#define DOWNLOAD_NOT_ALLOWED			0x100015
#define HI_TRANSFER_SPEED_FAIL			0x100016



void SSPCommsRx(uint8 data);


#endif



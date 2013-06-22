#define CCONV _stdcall
#define NOMANGLE


#define SSP_STX		0x7F

#define  ssp_CMD_SYNC				0x11
#define  ssp_CMD_POLL					0x7
#define  ssp_CMD_GET_SERIAL_NUMBER 0xC
#define  ssp_CMD_DISABLE 0x9
#define  ssp_CMD_ENABLE 0xA
#define  ssp_CMD_RESET 0x1
#define  ssp_MANU_EXT  0x30

#define ssp_CMD_PROGRAM_CMD  0x0B
#define ssp_CMD_PROG_RAM	 0x03


#define ssp_RSP_OK			0xF0


typedef struct{
	unsigned __int64 FixedKey;
	unsigned __int64 EncryptKey;
}SSP_FULL_KEY;


typedef struct{
	unsigned short packetTime;
	unsigned char PacketLength;
	unsigned char PacketData[255];
}SSP_PACKET;


typedef struct{
	unsigned char* CommandName;
	unsigned char* LogFileName;
	unsigned char Encrypted;
	SSP_PACKET Transmit;
	SSP_PACKET Receive;
}SSP_COMMAND_INFO;



typedef struct{
	SSP_FULL_KEY Key;
	unsigned long BaudRate;
	unsigned long Timeout;
	unsigned char PortNumber;
	unsigned char SSPAddress;
	unsigned char RetryLevel;
	unsigned char EncryptionStatus;
	unsigned char CommandDataLength;
	unsigned char CommandData[255];
	unsigned char ResponseStatus;
	unsigned char ResponseDataLength;
	unsigned char ResponseData[255];
}SSP_COMMAND;


typedef struct{
	unsigned char txData[255];
	unsigned char txPtr;
	unsigned char rxData[255];
	unsigned char rxPtr;
	unsigned char txBufferLength;
	unsigned char rxBufferLength;
	unsigned char SSPAddress;
	unsigned char NewResponse;
	unsigned char CheckStuff;
}SSP_TX_RX_PACKET;


typedef struct{
	unsigned long NumberOfBlocks;
	unsigned long NumberOfRamBytes;
	unsigned char* fData;
}ITL_FILE_DOWNLOAD;


typedef struct{
	unsigned int totalRamBlocks;
	unsigned int currentRamBlocks;
	unsigned int ramState;
}RAM_UPDATE_STATUS;


#define rmd_RAM_DOWNLOAD_IDLE		0x100000
#define rmd_DOWNLOAD_RAM_FILE		0x100001
#define rmd_ESTABLISH_COMS			0x100002
#define rmd_INITIATE_UPDATE_CMD		0x100003
#define rmd_INITIATE_UPDATE_HDR		0x100004
#define rmd_SEND_UPDATE_DATA		0x100005

#define rmd_FILE_VERIFY 			0x100006



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


NOMANGLE int CCONV SSPSendCommand(SSP_COMMAND* cmd, SSP_COMMAND_INFO* sspInfo);
NOMANGLE int CCONV OpenSSPComPort(SSP_COMMAND* cmd);
NOMANGLE int CCONV CloseSSPComPort(void);
NOMANGLE int CCONV DownloadFileToTarget(char* szFilename, int cPort, unsigned char sspAddress);
NOMANGLE long CCONV GetRamDownloadStatus(RAM_UPDATE_STATUS* rmData);

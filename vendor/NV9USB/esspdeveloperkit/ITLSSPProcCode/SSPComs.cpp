#define CCONV _stdcall
#define NOMANGLE

/*----------------------------------------------------------------------------------------------
---------------------------- Header includes ---------------------------------------------------
------------------------------------------------------------------------------------------------*/				
#include "stdafx.h"
#include <stdlib.h>
#include "SSPComs.h"
#include "ITLSSPProc.h"
#include "Encryption.h"
#include <windows.h>
#include <winbase.h>         // standard windows defines
#include <process.h>         // for multithreading functions
#include <winver.h>			 // for version information
#include <time.h>               /* for clock() and clock_t */
#include <fstream.h>
#include <iostream.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

/* command status enumeration */
typedef enum{
	PORT_CLOSED,
	PORT_OPEN,
	PORT_ERROR,
	SSP_REPLY_OK,
	SSP_PACKET_ERROR,
	SSP_CMD_TIMEOUT,
}PORT_STATUS;


extern unsigned int encPktCount;


/*-------------------------------------------------------------------
------------------- module function prototypes-----------------------
---------------------------------------------------------------------*/
void EventNotify(PVOID);
void SetOS(void);
int CompileSSPCommand(SSP_COMMAND* cmd,SSP_TX_RX_PACKET* ss,SSP_COMMAND_INFO* sspInfo);
BOOL WritePort(SSP_TX_RX_PACKET* ss);
unsigned char ReadPort(void);
void SSPDataIn(unsigned char RxChar);
void DownloadITLTarget(void* data);

/*--------------------------------------------------------------------
------------------- module variables ---------------------------------
----------------------------------------------------------------------*/
int PortStatus = PORT_CLOSED;
typedef enum {W_32s,W_95,W_NT} OS_VER; // for os versions
OS_VER OS;
HANDLE hDevice;					// handle to com device
bool bActive = FALSE;
unsigned char ulCharsGone = 0;
unsigned char sspSeq = 0x80;
OVERLAPPED oNotify;            // overlapped structure for non-blocking threads
SSP_TX_RX_PACKET ssp;	
ITL_FILE_DOWNLOAD itlFile;
RAM_UPDATE_STATUS ramStatus;
SSP_COMMAND sspC;
SSP_COMMAND_INFO sspCmdInfo;
/*--------------------------------------------------------------------------
------------------- module functions ----------------------------------------
-----------------------------------------------------------------------------*/

NOMANGLE int CCONV SSPSendCommand(SSP_COMMAND* cmd, SSP_COMMAND_INFO* sspInfo)
{
	
	clock_t txTime,currentTime,rxTime;
	int i;
	unsigned char encryptLength;
	unsigned short crcR;
	unsigned char tData[255];
	unsigned char retry;
	unsigned int slaveCount;

	/* complie the SSP packet and check for errors  */	
	if(!CompileSSPCommand(cmd,&ssp, sspInfo )){
		cmd->ResponseStatus = SSP_PACKET_ERROR;
		return 0;
	}

	retry = cmd->RetryLevel; 
	/* transmit the packet    */
	do{
		ssp.NewResponse = 0;  /* set flag to wait for a new reply from slave   */
		if(WritePort(&ssp) != TRUE){
			CloseSSPComPort();
			cmd->ResponseStatus = PORT_ERROR;
			return 0;
		}
		
		/* wait for out reply   */
		cmd->ResponseStatus = SSP_REPLY_OK;
		txTime = clock();
		while(!ssp.NewResponse){
			/* check for reply timeout   */
			currentTime = clock();
			if(currentTime - txTime > cmd->Timeout){
				PortStatus = SSP_CMD_TIMEOUT;
				cmd->ResponseStatus = SSP_CMD_TIMEOUT;
				break;
			}
		}
		
		if(cmd->ResponseStatus == SSP_REPLY_OK)
			break;

		retry--;
	}while(retry > 0);


	rxTime = clock();
	sspInfo->Receive.packetTime = (unsigned short)(rxTime - txTime);

	if(cmd->ResponseStatus == SSP_CMD_TIMEOUT){
			sspInfo->Receive.PacketLength = 0;
			CloseSSPComPort();
			return 0;
	}

	

	/* load the command structure with ssp packet data   */
	if(ssp.rxData[3] == SSP_STEX){   /* check for encrpted packet    */
		encryptLength = ssp.rxData[2] - 1;
		DecryptSSPPacket(&ssp.rxData[4],&ssp.rxData[4],&encryptLength,&encryptLength,(unsigned __int64*)&cmd->Key);	
		/* check the checsum    */
		crcR = cal_crc_loop_CCITT_A(encryptLength - 2,&ssp.rxData[4] ,CRC_SSP_SEED,CRC_SSP_POLY);
		if((unsigned char)(crcR & 0xFF) != ssp.rxData[ssp.rxData[2] + 1] || (unsigned char)((crcR >> 8) & 0xFF) != ssp.rxData[ssp.rxData[2] + 2]){
			cmd->ResponseStatus = SSP_PACKET_ERROR;
			sspInfo->Receive.PacketLength = 0;
			CloseSSPComPort();
			return 0;				
		}
		/* check the slave count against the host count  */
		slaveCount = 0;
		for(i = 0; i < 4; i++)
			slaveCount += (unsigned int)(ssp.rxData[5 + i]) << (i*8); 
		/* no match then we discard this packet and do not act on it's info  */
		if(slaveCount != encPktCount){
			cmd->ResponseStatus = SSP_PACKET_ERROR;
			sspInfo->Receive.PacketLength = 0;
			CloseSSPComPort();
			return 0;
		}

		/* restore data for correct decode  */
		ssp.rxBufferLength = ssp.rxData[4] + 5;
		tData[0] = ssp.rxData[0];
		tData[1] = ssp.rxData[1];
		tData[2] = ssp.rxData[4];
		for(i = 0; i < ssp.rxData[4]; i++)
			tData[3 + i] = ssp.rxData[9 + i];
		crcR = cal_crc_loop_CCITT_A(ssp.rxBufferLength - 3,&tData[1] ,CRC_SSP_SEED,CRC_SSP_POLY);
		tData[3 + ssp.rxData[4]] = (unsigned char)(crcR & 0xFF);
		tData[4 + ssp.rxData[4]] = (unsigned char)((crcR >> 8) & 0xFF);
		for(i = 0; i < ssp.rxBufferLength; i++)
			ssp.rxData[i] = tData[i]; 
		
		/* for decrypted resonse with encrypted command, increment the counter here  */
		if(!cmd->EncryptionStatus)
		  encPktCount++;


	}


	cmd->ResponseDataLength = ssp.rxData[2];
	for(i = 0; i < cmd->ResponseDataLength; i++)
		cmd->ResponseData[i] = ssp.rxData[i + 3];  


	sspInfo->Receive.PacketLength = ssp.rxBufferLength;
	for(i = 0; i < ssp.rxBufferLength; i++)
		sspInfo->Receive.PacketData[i] = ssp.rxData[i];   

	/* alternate the seq bit   */
	if(sspSeq == 0x80)
		sspSeq = 0;
	else
		sspSeq = 0x80;


	/* terminate the thread function   */
	cmd->ResponseStatus = SSP_REPLY_OK;

	return 1;

}


int CompileSSPCommand(SSP_COMMAND* cmd,SSP_TX_RX_PACKET* ss,SSP_COMMAND_INFO* sspInfo)
{
	
	int i,j;
	unsigned short crc;
	unsigned char tBuffer[255];

	ss->rxPtr = 0;
	for(i = 0; i < 255; i++)
		ss->rxData[i] = 0x00; 

	/* for sync commands reset the deq bit   */
	if(cmd->CommandData[0] == ssp_CMD_SYNC) 
		sspSeq = 0x80;

	/* update the log packet data before any encryption   */	
	sspInfo->Encrypted = cmd->EncryptionStatus; 
	sspInfo->Transmit.PacketLength = cmd->CommandDataLength + 5;
	sspInfo->Transmit.PacketData[0] = SSP_STX;					/* ssp packet start   */
	sspInfo->Transmit.PacketData[1] = cmd->SSPAddress | sspSeq;  /* the address/seq bit */ 
	sspInfo->Transmit.PacketData[2] = cmd->CommandDataLength;    /* the data length only (always > 0)  */  
	for(i = 0; i < cmd->CommandDataLength; i++)  /* add the command data  */
		sspInfo->Transmit.PacketData[3 + i] = cmd->CommandData[i]; 
	/* calc the packet CRC  (all bytes except STX)   */
	crc = cal_crc_loop_CCITT_A(cmd->CommandDataLength + 2,&sspInfo->Transmit.PacketData[1] ,CRC_SSP_SEED,CRC_SSP_POLY);
	sspInfo->Transmit.PacketData[3 + cmd->CommandDataLength] = (unsigned char)(crc & 0xFF);
	sspInfo->Transmit.PacketData[4 + cmd->CommandDataLength] = (unsigned char)((crc >> 8) & 0xFF); 

	/* is this a encrypted packet  */
	if(cmd->EncryptionStatus){
		if(!EncryptSSPPacket(cmd->CommandData,cmd->CommandData,&cmd->CommandDataLength,&cmd->CommandDataLength,(unsigned __int64*)&cmd->Key))
			return 0;
		
	}

	/* create the packet from this data   */
	ss->CheckStuff = 0;
	ss->SSPAddress = cmd->SSPAddress;
	ss->rxPtr = 0;
	ss->txPtr = 0;
	ss->txBufferLength = cmd->CommandDataLength + 5;  /* the full ssp packet length   */
	ss->txData[0] = SSP_STX;					/* ssp packet start   */
	ss->txData[1] = cmd->SSPAddress | sspSeq;  /* the address/seq bit */ 
	ss->txData[2] = cmd->CommandDataLength;    /* the data length only (always > 0)  */  
	for(i = 0; i < cmd->CommandDataLength; i++)  /* add the command data  */
		ss->txData[3 + i] = cmd->CommandData[i]; 
	/* calc the packet CRC  (all bytes except STX)   */
	crc = cal_crc_loop_CCITT_A(ss->txBufferLength - 3,&ss->txData[1] ,CRC_SSP_SEED,CRC_SSP_POLY);
	ss->txData[3 + cmd->CommandDataLength] = (unsigned char)(crc & 0xFF);
	ss->txData[4 + cmd->CommandDataLength] = (unsigned char)((crc >> 8) & 0xFF);

	/* we now need to 'byte stuff' this buffered data   */
	j = 0;
	tBuffer[j++] = ss->txData[0];
	for(i = 1; i < ss->txBufferLength; i++){
		tBuffer[j] = ss->txData[i];
		if (ss->txData[i] ==	SSP_STX){
			tBuffer[++j] = SSP_STX;   /* SSP_STX found in data so add another to 'stuff it'  */
		}
		j++;
	}
	for(i = 0; i < j; i++)
		ss->txData[i] = tBuffer[i];
	ss->txBufferLength  = j;

	return 1;
}


void SSPDataIn(unsigned char RxChar)
{
	unsigned short crc;


	if (RxChar == SSP_STX && ssp.rxPtr == 0){
		// packet start
		ssp.rxData[ssp.rxPtr++] = RxChar;
	}else{
		// if last byte was start byte, and next is not then
		// restart the packet
		if (ssp.CheckStuff == 1){
			if (RxChar != SSP_STX){
				ssp.rxData[0] = SSP_STX;
				ssp.rxData[1] = RxChar;  
				ssp.rxPtr = 2;
			}else
				ssp.rxData[ssp.rxPtr++] = RxChar;
			// reset stuff check flag	
			ssp.CheckStuff = 0; 		
		}else{
			// set flag for stuffed byte check
			if (RxChar == SSP_STX)
				ssp.CheckStuff = 1;
			else{		
				// add data to packet
				ssp.rxData[ssp.rxPtr++] = RxChar;
				// get the packet length
				if (ssp.rxPtr == 	3)
					ssp.rxBufferLength = ssp.rxData[2] + 5; 						
			}
		}
		// are we at the end of the packet
		if (ssp.rxPtr  == ssp.rxBufferLength ){
			// is this packet for us ??
			if ((ssp.rxData[1] & SSP_STX) == ssp.SSPAddress){
				// is the checksum correct
				crc = cal_crc_loop_CCITT_A(ssp.rxBufferLength - 3,&ssp.rxData[1] ,CRC_SSP_SEED,CRC_SSP_POLY);							
				if ((unsigned char)(crc & 0xFF) == ssp.rxData[ssp.rxBufferLength - 2] && (unsigned char)((crc >> 8) & 0xFF) == ssp.rxData[ssp.rxBufferLength - 1])
					ssp.NewResponse = 1;  /* we have a new response so set flag  */
			}
			// reset packet 
			ssp.rxPtr  = 0;	
			ssp.CheckStuff = 0;		
		}				
	}	


}


NOMANGLE int CCONV DownloadFileToTarget(char* szFilename, int cPort, unsigned char sspAddress)
{

	int fh1,i;
	unsigned long numCurBytes;
	unsigned short dBlockSize;

	
		// get file handle and open file
	fh1 = _open(szFilename,_O_BINARY);
	if (fh1 == -1) return OPEN_FILE_ERROR;
	// some space for the data
	itlFile.fData = new unsigned char[_filelength(fh1)];  
	// read the file and store the data in the buffer
	if(_read(fh1,itlFile.fData,_filelength(fh1)) == 0) return READ_FILE_ERROR;
	// close the file
	_close(fh1);

	ramStatus.currentRamBlocks = 0;
	ramStatus.ramState = rmd_RAM_DOWNLOAD_IDLE;
	ramStatus.totalRamBlocks = 0;


	// check for ITL BV/SH type file
    if (itlFile.fData[0] == 'I' && itlFile.fData[1] == 'T' && itlFile.fData[2] == 'L') {

		 numCurBytes = 0;
		 for(i = 0; i <4; i++){
			numCurBytes += (unsigned long)itlFile.fData[17 + i] << (8*(3-i));
		 }
		  //get the block size from header
		 dBlockSize = (256*(unsigned short)itlFile.fData[0x3e]) + (unsigned short)itlFile.fData[0x3f];
		// correct for NV9 type
		 if(dBlockSize == 0) dBlockSize = 4096;
				
		  itlFile.NumberOfBlocks = numCurBytes / dBlockSize;
		  if(itlFile.NumberOfBlocks % dBlockSize != 0) itlFile.NumberOfBlocks += 1;
	}
	else{
		delete itlFile.fData;	
		return NOT_ITL_FILE;
	}

	/* check target connection   */
	sspC.BaudRate = 9600;
	sspC.PortNumber = cPort;
	sspC.RetryLevel = 2;
	sspC.SSPAddress = sspAddress;
	if( OpenSSPComPort(&sspC) == 0)
		return PORT_OPEN_FAIL;

	sspC.EncryptionStatus = 0;
	sspC.CommandDataLength = 1;
	sspC.CommandData[0]  = ssp_CMD_SYNC;
	
	if (SSPSendCommand(&sspC,&sspCmdInfo) == 0){
		CloseSSPComPort();
		delete itlFile.fData;
		return SYNC_CONNECTION_FAIL;
	}
	if(sspC.ResponseData[0] != ssp_RSP_OK){
		CloseSSPComPort();
		delete itlFile.fData;
		return SYNC_CONNECTION_FAIL;		
	}
	
	_beginthread(DownloadITLTarget,0,NULL); 

	return itlFile.NumberOfBlocks;


}


void DownloadITLTarget(void* data)
{
	int i;
	int numRamBlocks;


	itlFile.NumberOfRamBytes = 0;
	for(i = 0; i < 4; i++)
		itlFile.NumberOfRamBytes += (unsigned long)itlFile.fData[7 + i] << (8 * (3-i));

	
	ramStatus.ramState = rmd_ESTABLISH_COMS;

	sspC.CommandDataLength = 2;
	sspC.CommandData[0] = ssp_CMD_PROGRAM_CMD;
	sspC.CommandData[1] = ssp_CMD_PROG_RAM;
	
	if (SSPSendCommand(&sspC,&sspCmdInfo) == 0){
		CloseSSPComPort();
		delete itlFile.fData;
		_endthread();
		return;		
	}
	if(sspC.ResponseData[0] != ssp_RSP_OK){
		CloseSSPComPort();
		delete itlFile.fData;
		_endthread();
		return;		
	}

	sspC.CommandDataLength = 128;
	for(i = 0 ; i < 128; i++)
		sspC.CommandData[i] = itlFile.fData[i];  
	
	if (SSPSendCommand(&sspC,&sspCmdInfo) == 0){
		CloseSSPComPort();
		delete itlFile.fData;
		_endthread();
		return;		
	}
	if(sspC.ResponseData[0] != ssp_RSP_OK){
		CloseSSPComPort();
		delete itlFile.fData;
		_endthread();
		return;		
	}

	/* checnge speed to download level   */
	CloseSSPComPort();
	sspC.BaudRate = 38400;
	if( OpenSSPComPort(&sspC) == 0){
		_endthread();
		return;	
	}





	CloseSSPComPort();


	_endthread();
}





NOMANGLE long CCONV GetRamDownloadStatus(RAM_UPDATE_STATUS* rmData)
{

	rmData->currentRamBlocks = ramStatus.currentRamBlocks;
	rmData->ramState = ramStatus.ramState ;
	rmData->totalRamBlocks = ramStatus.totalRamBlocks ;


	return 1;
}



NOMANGLE int CCONV OpenSSPComPort(SSP_COMMAND* cmd)
{
COMMTIMEOUTS CommTimeouts;
char szCom[20];  // for COM
char openTry;
DCB Dcb;

	/* if port already open then come out   */
	if(PortStatus == PORT_OPEN)
		return 1;

	SetOS();

	if (cmd->PortNumber >9)
		wsprintf(szCom,"\\\\.\\COM%d",cmd->PortNumber ); // format device name as COMx
	else 
		wsprintf(szCom,"COM%d",cmd->PortNumber ); // format device name as COMx

	// retry loop as to make sure dual core processors open OK
	openTry = 0;
	do{
		if (OS==W_NT) 
			hDevice=CreateFile(szCom, // name of device
				GENERIC_READ | GENERIC_WRITE, // access mode
				0,             // exclusive access
				NULL,          // address of security desriptor
				OPEN_EXISTING, // how to create
				FILE_FLAG_OVERLAPPED, // file attributes 
				NULL);         // handle of file with attributes to copy

		else if ((OS==W_95) || (OS==W_32s))
			hDevice=CreateFile(szCom, // name of device
				GENERIC_READ | GENERIC_WRITE, // access mode
				0,             // exclusive access
				NULL,          // address of security desriptor
				OPEN_EXISTING, // how to create
				0,			   // file attributes
				NULL);         // handle of file with attributes to copy
	
				Sleep(100);

	}while(hDevice==INVALID_HANDLE_VALUE && openTry++ < 5);

	if (hDevice==INVALID_HANDLE_VALUE)
	{
		   
		 MessageBox(0,"Unable To open this port",
	   szCom,MB_OK | MB_ICONEXCLAMATION);
		PortStatus = PORT_ERROR;
		return 0;
	}


// First setup the queue sizes
	if (!SetupComm(hDevice,1024,1024)) // if not successful
	{
		PortStatus = PORT_ERROR;
		return 0;
	}

// initialise some timeouts, otherwise unpredictable results
	memset(&CommTimeouts,0,sizeof(CommTimeouts));
	CommTimeouts.ReadIntervalTimeout=MAXDWORD;		// to allow ReadFile to return immediately 
	CommTimeouts.ReadTotalTimeoutMultiplier=0;		// !used 
	CommTimeouts.ReadTotalTimeoutConstant=0;		// !used 
	CommTimeouts.WriteTotalTimeoutMultiplier=0;		// !used 
	CommTimeouts.WriteTotalTimeoutConstant=0;		// !used 
	SetCommTimeouts(hDevice,&CommTimeouts);
	// set the event mask for notification of certain comms events
	SetCommMask(hDevice,EV_RXCHAR | EV_TXEMPTY);

	if (!GetCommState(hDevice,&Dcb))   // get default state
	{
		CloseSSPComPort();
		PortStatus = PORT_ERROR;
		return 0;
	}

    Dcb.BaudRate=cmd->BaudRate; 
	Dcb.ByteSize=8;   
	Dcb.Parity=NOPARITY;
    Dcb.StopBits=TWOSTOPBITS;

	// apply new settings
	if (!SetCommState(hDevice,&Dcb))   // set new state

	{
		CloseSSPComPort();
		MessageBox(GetFocus(),"Error setting new comms params","SetupComm",MB_OK | MB_ICONEXCLAMATION);
		PortStatus = PORT_ERROR;
		return 0;
	}



	bActive =TRUE; // to allow a comms event notification thread to run
	_beginthread(EventNotify, // address of our thread routine
				 0,  // default stacksize
				 NULL); // args to pass to routine


	PortStatus = PORT_OPEN;


	return 1;
}


void EventNotify(PVOID)
{
DWORD dwEvent,dwError;
COMSTAT cs;


	while(bActive){
			WaitCommEvent(hDevice,&dwEvent,NULL);
			ClearCommError (hDevice,&dwError,&cs);
			if (dwEvent & EV_RXCHAR){


				while (cs.cbInQue){
					SSPDataIn(ReadPort());				
					ClearCommError (hDevice,&dwError,&cs);
				}
			}

			if (dwEvent & EV_TXEMPTY)
				ulCharsGone = 1;

	}

   _endthread();
	
  
} 


NOMANGLE int CCONV CloseSSPComPort(void)
{

	bActive = FALSE;


	if(PortStatus != PORT_CLOSED){
		PortStatus = PORT_CLOSED;
		SetCommMask(hDevice,0);
		CloseHandle(hDevice);
	}

	return 1;

}	


BOOL WritePort(SSP_TX_RX_PACKET* ss){
ULONG ulBytesWritten;			// buffer for bytes written count
DWORD dwError;


// overlapped structure not supported in Win95
	if (OS==W_NT)
	{ 
		oNotify.Offset=0;		// start of transfer
		oNotify.OffsetHigh=0;   // start of transfer
		if (!WriteFile(hDevice,	// handle of device
			ss->txData,			// string to write
			ss->txBufferLength,				// number of bytes to write
			&ulBytesWritten,	// number of bytes written
			&oNotify))		// address of overlapped structure
		{
			dwError=GetLastError();
			// ok for overlapped error
			if(dwError == ERROR_IO_PENDING)
				return TRUE;
			else
				return FALSE;	
		}
	}
	else if ((OS==W_95) || (OS==W_32s))
	{
		if (!WriteFile(hDevice,	// handle of device
			ss->txData,			// string to write
			ss->txBufferLength ,				// number of bytes to write
			&ulBytesWritten,	// number of bytes written
			NULL))			// address of overlapped structure
		{
			dwError = GetLastError();
				return FALSE;
		}
	}

	return TRUE;
}


unsigned char ReadPort(void){
ULONG ulBytesRead;      // no. of bytes read var
unsigned char readbyte;

	readbyte = 0;

	if (OS==W_NT)
	{
		ReadFile(hDevice,	    // handle to device
				&readbyte,	// input buffer
				1,			// no. of bytes to read
				&ulBytesRead,// no. of bytes read
				&oNotify);		// overlapped structure
	}
	else if ((OS==W_95) || (OS==W_32s))
	{
		ReadFile(hDevice,	    // handle to device
				&readbyte,	// input buffer
				1,			// no. of bytes to read
				&ulBytesRead,// no. of bytes read
				NULL);		// overlapped structure
	}
	
	return readbyte;
}



void SetOS(void){
	OSVERSIONINFO VerInfo;  // OS version information structure
		// get the OS info
	VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&VerInfo))
	{
		MessageBox(GetFocus(),"Unable To Determine OS","Set OS",MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	switch(VerInfo.dwPlatformId)
	{
		case VER_PLATFORM_WIN32_WINDOWS:
			OS=W_95;
			break;
		case VER_PLATFORM_WIN32_NT:
			OS=W_NT;
			break;
		case VER_PLATFORM_WIN32s:
			OS=W_32s;
			break;
	}
}

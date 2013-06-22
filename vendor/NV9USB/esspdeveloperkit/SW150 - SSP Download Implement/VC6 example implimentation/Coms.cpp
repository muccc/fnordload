
/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|							CWinPortComs
|					A module to provide Windows serial communication port functionality
|								
|					
|---------------------------------------------------------------------------------------------------*/


/*		Module Includes			*/
#include "stdafx.h"
#include <stdlib.h>
#include <windows.h>
#include <winbase.h>         // standard windows defines
#include <process.h>         // for multithreading functions
#include <winver.h>			 // for version information
#include "Coms.h"
#include <stdio.h>
#include <time.h>



#pragma comment(lib,"advapi32.lib")


/* module function declarations			*/
void SetOS(void);
bool WritePort(HANDLE deviceHandle, uint8* txData,uint8 txdatalength);
uint8 ReadPort(HANDLE deviceHandle);
void EventNotify(PVOID);

/* Module variable declarations			*/
typedef enum {W_32s,W_95,W_NT} OS_VER; // for os versions
OS_VER OS;
HANDLE portHandle;
BOOL threadActive = false;
OVERLAPPED oNotify;            // overlapped structure for non-blocking threads
uint8 ulCharsGone;
uint8 rxMode;
uint8 chkRtn;


/*		Module Functions				*/


/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	OpenComPort
|					Description: Opens a serial com port for comunication
|								
|					Parameter: Pointer to SSP_DOWNLOAD structure
|	`				Returns: 0 for fail, 1 for OK
|---------------------------------------------------------------------------------------------------*/
int16 OpenComPort(SSP_DOWNLOAD* sspD)
{
	COMMTIMEOUTS CommTimeouts;
	int8 openTry;
	DCB Dcb;
	int8 szCom[20];


	/* check if port is already open  */
	if(sspD->PortStatus == COM_PORT_OPEN)
		return 1;
	
	/* determine the OS of HOST  */
	SetOS();

	
	if (sspD->comPort >9)
		sprintf(szCom,"\\\\.\\COM%d",sspD->comPort); // format device name as COMx
	else 
		sprintf(szCom,"COM%d",sspD->comPort); // format device name as COMx

	
	// retry loop as to make sure dual core processors open OK
	openTry = 0;
	do{
		if (OS==W_NT) 
			portHandle = CreateFile(szCom, // name of device
				GENERIC_READ | GENERIC_WRITE, // access mode
				0,             // exclusive access
				NULL,          // address of security desriptor
				OPEN_EXISTING, // how to create
				FILE_FLAG_OVERLAPPED, // file attributes 
				NULL);         // handle of file with attributes to copy

		else if ((OS==W_95) || (OS==W_32s))
			portHandle = CreateFile(szCom, // name of device
				GENERIC_READ | GENERIC_WRITE, // access mode
				0,             // exclusive access
				NULL,          // address of security desriptor
				OPEN_EXISTING, // how to create
				0,			   // file attributes
				NULL);         // handle of file with attributes to copy
	
				Sleep(100);

	}while(portHandle == INVALID_HANDLE_VALUE && openTry++ < 5);

	if (portHandle==INVALID_HANDLE_VALUE)
	{
		sspD->PortStatus = COM_PORT_ERROR;
		return 0;
	}	

	/* initialise some timeouts, otherwise unpredictable results	*/
	memset(&CommTimeouts,0,sizeof(CommTimeouts));
	CommTimeouts.ReadIntervalTimeout=MAXDWORD;		// to allow ReadFile to return immediately 
	CommTimeouts.ReadTotalTimeoutMultiplier=0;		// !used 
	CommTimeouts.ReadTotalTimeoutConstant=0;		// !used 
	CommTimeouts.WriteTotalTimeoutMultiplier=0;		// !used 
	CommTimeouts.WriteTotalTimeoutConstant=0;		// !used 
	SetCommTimeouts(portHandle,&CommTimeouts);
	// set the event mask for notification of certain comms events
	SetCommMask(portHandle,EV_RXCHAR | EV_TXEMPTY);



	if (!GetCommState(portHandle,&Dcb))   // get default state
	{
		return 0;
	}
	Dcb.BaudRate = 9600;
	Dcb.ByteSize = 8;
	Dcb.Parity = NOPARITY;
	Dcb.StopBits = TWOSTOPBITS;
	if (!SetCommState(portHandle, &Dcb))
		return 0;

	rxMode = COMMS_MODE_SSP;

	/* start the thread  */
	sspD->PortStatus = COM_PORT_OPEN;
	threadActive = true;
	_beginthread(EventNotify,0,NULL); // args to pass to routine	


	return 1;

}

/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	WaitDelay
|					Description: Provides a delay
|								
|					Parameter: unsigned long delay in ms
|	`				Returns: None
|---------------------------------------------------------------------------------------------------*/
void WaitDelay(uint32 delay)
{
	Sleep(delay);

}


/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	EventNotify
|					Description: Port Notify Thread
|								
|					Parameter: None
|	`				Returns: None
|---------------------------------------------------------------------------------------------------*/
void EventNotify(PVOID)
{
DWORD dwEvent,dwError;
COMSTAT cs;


	while(threadActive){
			WaitCommEvent(portHandle,&dwEvent,NULL);
			ClearCommError (portHandle,&dwError,&cs);
			if (dwEvent & EV_RXCHAR)
			{
				while (cs.cbInQue){
					if(rxMode == COMMS_MODE_SSP)
						SSPCommsRx(ReadPort(portHandle));	
					else
						chkRtn = 1;

					ClearCommError (portHandle,&dwError,&cs);
				}				
			}
			if (dwEvent & EV_TXEMPTY)
				ulCharsGone = 1;


	}

   _endthread();
	
  
} 



/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	SendComData
|					Description: Writes data to serial com port - this is used for bulk data transmission
|								
|					Parameter: Pointer to char data array, length of data to write
|	`				Returns: 0 for fail, 1 for OK
|---------------------------------------------------------------------------------------------------*/
int16 SendComData(uint8* data, uint32 length)
{
ULONG ulBytesWritten;			// buffer for bytes written count
DWORD dwError;	

	
	rxMode = COMMS_MODE_SINGLE;
	chkRtn = 0;
	ulCharsGone = 0;
	
	if (OS==W_NT)
	{ 
		oNotify.Offset=0;		// start of transfer
		oNotify.OffsetHigh=0;   // start of transfer
		if (!WriteFile(portHandle,	// handle of device
			data,			// string to write
			length,				// number of bytes to write
			&ulBytesWritten,	// number of bytes written
			&oNotify))		// address of overlapped structure
		{
			dwError=GetLastError();
			// ok for overlapped error
			if(dwError == ERROR_IO_PENDING)
				return 1;
			else
				return 0;	
		}
	}
	else if ((OS==W_95) || (OS==W_32s))
	{
		if (!WriteFile(portHandle,	// handle of device
			data,			// string to write
			length,				// number of bytes to write
			&ulBytesWritten,	// number of bytes written
			NULL))			// address of overlapped structure
		{
			dwError = GetLastError();
				return 0;
		}
	}


	while(!ulCharsGone)
		Sleep(5);

	return 1;
}



/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	GetTargetChecksum
|					Description: Waits for a byte retrun from the target and compares with parameter byte
|								
|					Parameter: char byte to test against
|	`				Returns: 0 for fail, 1 for OK
|---------------------------------------------------------------------------------------------------*/
int16 GetTargetChecksum(uint8 chk)
{

	clock_t thisTime,lastTime;
	
	lastTime = clock();
	while(!chkRtn){
		/* check for timeout  */
		thisTime = clock();
		if(thisTime - lastTime > 5000)
			return 0;
	}

	if(ReadPort(portHandle) != chk)
		return 0;


	return 1;
}


/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	CloseComPort
|					Description: Closes handle to open com port and terminated notify thread
|								
|					Parameter: pointer to SSP_DOWNLOAD structure
|	`				Returns: None
|---------------------------------------------------------------------------------------------------*/
void CloseComPort(SSP_DOWNLOAD* sspD)
{

	if(sspD->PortStatus == COM_PORT_OPEN){
		threadActive = false;
		SetCommMask(portHandle,0);
		CloseHandle(portHandle);
		sspD->PortStatus = COM_PORT_CLOSED;
	}
	

}

/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	SetDownloadSpeed
|					Description: Changes com prt speed to required rate. Note a test is doen for port type 
|								 here - USB virual ports cause SetComState function to crash!
|					Parameter: pointer to SSP_DOWNLOAD structure, the required baud rate to change to 
|	`				Returns: 0 for fail, 1 for OK
|---------------------------------------------------------------------------------------------------*/
int16 SetDownloadSpeed(SSP_DOWNLOAD* sspD,uint32 iBaud)
{

	DCB Dcb;

	/* if the port is USB - do not use Get and SetCommState */
	if(GetPortType(sspD->comPort) == PORT_BV)
		return 1;

	if (!GetCommState(portHandle,&Dcb))   // get default state
	{
		return 0;
	}
	Dcb.BaudRate = iBaud;
	Dcb.ByteSize = 8;
	Dcb.Parity = NOPARITY;
	Dcb.StopBits = TWOSTOPBITS;
	if (!SetCommState(portHandle, &Dcb))
		return 0;



	return 1;

}

/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	SetOS
|					Description: Determines the operating system of windows and sets a global variable
|								
|					Parameter: None
|	`				Returns: None
|---------------------------------------------------------------------------------------------------*/
void SetOS(void)
{
	OSVERSIONINFO VerInfo;  // OS version information structure
		// get the OS info
	VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&VerInfo))
	{
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


/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	TransmitDataPacket
|					Description: Function to transmit an SSP packet and wait for a response, retying if necessary
|								
|					Parameter: Pointer to the SSP packet structure
|	`				Returns: 0 for fail, 1 for OK
|---------------------------------------------------------------------------------------------------*/
int16 TransmitDataPacket(SSP_PACKET* sspP)
{

	clock_t thisTime,lastTime;
	uint8 retries;

	retries = 0;

	while(retries < sspP->retry){ 
		sspP->replyStatus = STATUS_WAIT;
		sspP->rxPtr = 0;
		sspP->checkStuff = 0;
		if(!WritePort(portHandle,sspP->txPacketData,sspP->txPacketLength))
			return 0;

			
			/* wait for reply  */
		lastTime = clock();
		while(sspP->replyStatus == STATUS_WAIT){
			thisTime = clock();	
			if(thisTime - lastTime >= sspP->timeout)
				sspP->replyStatus = STATUS_ERR_TIMEOUT;
		}
		if(sspP->replyStatus == STATUS_OK){
			/* toggle the sync bit for a successful reply  */
			if(sspP->syncbit == 0x80)
				sspP->syncbit = 0;
			else
				sspP->syncbit = 0x80;
			return 1;
		}

		retries++;
	}
	

	return 1;

}



/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	WritePort
|					Description: Writes data to com port - used for SSP data packet transmission
|								
|					Parameter: Handle to open com device, pointer to char data array, length of data to transmit
|	`				Returns: bool - false for fail, true for ok
|---------------------------------------------------------------------------------------------------*/
bool WritePort(HANDLE deviceHandle, uint8* txData,uint8 txdatalength)
{
ULONG ulBytesWritten;			// buffer for bytes written count
DWORD dwError;


// overlapped structure not supported in Win95
	if (OS==W_NT)
	{ 
		oNotify.Offset=0;		// start of transfer
		oNotify.OffsetHigh=0;   // start of transfer
		if (!WriteFile(deviceHandle,	// handle of device
			txData,			// string to write
			txdatalength,				// number of bytes to write
			&ulBytesWritten,	// number of bytes written
			&oNotify))		// address of overlapped structure
		{
			dwError=GetLastError();
			// ok for overlapped error
			if(dwError == ERROR_IO_PENDING)
				return true;
			else
				return false;	
		}
	}
	else if ((OS==W_95) || (OS==W_32s))
	{
		if (!WriteFile(deviceHandle,	// handle of device
			txData,			// string to write
			txdatalength,				// number of bytes to write
			&ulBytesWritten,	// number of bytes written
			NULL))			// address of overlapped structure
		{
			dwError = GetLastError();
				return false;
		}
	}

	return true;
}


/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	ReadPort
|					Description: Reads a byte from an open com port
|								
|					Parameter: HANDLE to open com port
|	`				Returns: bool - char - the byte read
|---------------------------------------------------------------------------------------------------*/
uint8 ReadPort(HANDLE deviceHandle)
{
	ULONG ulBytesRead;      // no. of bytes read var
	uint8 readbyte;

	readbyte = 0;

	if (OS==W_NT)
	{
		ReadFile(deviceHandle,	    // handle to device
				&readbyte,	// input buffer
				1,			// no. of bytes to read
				&ulBytesRead,// no. of bytes read
				&oNotify);		// overlapped structure
	}
	else if ((OS==W_95) || (OS==W_32s))
	{
		ReadFile(deviceHandle,	    // handle to device
				&readbyte,	// input buffer
				1,			// no. of bytes to read
				&ulBytesRead,// no. of bytes read
				NULL);		// overlapped structure
	}
	
	return readbyte;
}



/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	GetPortType
|					Description:	Reads windows registry and returns the type of serial com
|									port in use. 
|					Parameters: The number of the com port to investigate
|---------------------------------------------------------------------------------------------------*/
SERIAL_PORT_TYPE GetPortType(const unsigned long portnum)
{
	HKEY hkey = 0;
	SERIAL_PORT_TYPE port_type = PORT_UNKNOWN;
	//open the registry key needed
	LONG error = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_QUERY_VALUE, &hkey);
	if (ERROR_SUCCESS == error)
	{
		DWORD counter = 0;

		//declare some buffers
		DWORD regKeyLen = 255;
		LPSTR regKey = new CHAR[regKeyLen];
		
		DWORD regValueLen = 255;
		LPBYTE regValue = new BYTE[regValueLen];

		char * comname = new char[255];
		sprintf(comname,"COM%d",portnum);
		//get the first subkey
		error = RegEnumValue(hkey,counter,regKey,&regKeyLen,(LPDWORD)0,(LPDWORD)0,regValue,&regValueLen);
		while ((ERROR_SUCCESS == error) && (PORT_UNKNOWN == port_type))
		{
			//check if this port is the correct number
			if (0 == strcmp(comname,(char *)regValue))
			{
				//check what type of port this is
				if (0 != strstr((char *)regKey,"\\Device\\VCP"))
					port_type = PORT_DA2;
				else if (0 != strstr((char *)regKey,"\\Device\\USBSER"))
					port_type = PORT_BV;
			}			
			//these get set to the length of the return value, so need to reset to length of buffer
			regValueLen = 255;
			regKeyLen = 255;
			//get the next subkey
			error = RegEnumValue(hkey,++counter,regKey,&regKeyLen,(LPDWORD)0,(LPDWORD)0,regValue,&regValueLen);
		}
		
		//cleanup 
		RegCloseKey(hkey);
		delete [] regKey;
		delete [] regValue;
		delete [] comname;
	}
	return port_type;
}






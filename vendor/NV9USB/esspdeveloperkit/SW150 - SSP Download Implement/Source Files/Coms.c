
#include "Coms.h"


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

	/* TO DO add your hardware port open implmentation	*/


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

	/* To Do - impliment your hardwrae function to close serail com port  

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
	/*  TO DO - Add your hardware delay implientation  */

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




	/*  TO DO - Add your hardware function to write bytes to serial com port  */


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
	/* TO DO - impliment your function to read bytte from the target and compare against parameter */


	return 1;
} 



/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	SetDownloadSpeed
|					Description: Changes com prt speed to required rate.
|					Parameter: pointer to SSP_DOWNLOAD structure, the required baud rate to change to 
|	`				Returns: 0 for fail, 1 for OK
|---------------------------------------------------------------------------------------------------*/
int16 SetDownloadSpeed(SSP_DOWNLOAD* sspD,uint32 iBaud)
{
		
		/* TO DO - Impliment your hardware UART speed settimg function  */
	
	
	return 1;
	
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


	uint8 retries;

	retries = 0;

	while(retries < sspP->retry){ 
		sspP->replyStatus = STATUS_WAIT;
		sspP->rxPtr = 0;
		sspP->checkStuff = 0
		
		if(!SendComData(sspP->txPacketData,sspP->txPacketLength))
			return 0;

			
			/* wait for reply  */
		while(sspP->replyStatus == STATUS_WAIT){
			/* TO DO - in this loop impliment a timeout function if no reply is recieved */
			/*	sspP->replyStatus = STATUS_ERR_TIMEOUT;  */
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


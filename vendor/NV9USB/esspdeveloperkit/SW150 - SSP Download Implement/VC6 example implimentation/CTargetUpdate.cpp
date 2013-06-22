
#include "stdafx.h"

#include <stdio.h>
#include "CsspUpdate.h"
#include "Coms.h"
#include <io.h>
#include <fcntl.h>
#include <malloc.h>
#include "CTargetUpdate.h"


/*   Module defines							*/
#define CRC_SSP_SEED		0xFFFF
#define CRC_SSP_POLY		0x8005
#define ssp_RSP_OK			0xF0

#define SSP_STX							0x7F
#define ssp_CMD_SYNC					0x11
#define ssp_CMD_EXPANSION				0x30
#define ssp_CMD_VALIDATOR				0x05
#define ssp_CMD_GET_FULL_FIRMWARE		0x44
#define ssp_CMD_GET_FULL_DATASET		0x16
#define ssp_CMD_PROGRAM_TARGET			0x0B
#define ssp_CMD_PROGRAM_RAM				0x03

#define ram_OK_ACK						0x32


#define HDR_BLOCK_SIZE					128
#define DOWNLOAD_BLOCK_SIZE				4096

/*			module variables			*/
SSP_PACKET sspPacket;


/*		Module function declarations	*/
int16 SendSSPCommand(SSP_PACKET* sspP);
uint16 cal_crc_loop_CCITT_A( int16 l, uint8* p, uint16 seed, uint16 cd);



/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	EstablishSSPComms
|					Description: SSP function to test target connection and get target data
|								
|					Parameter: Pointer to SSP_DOWNLOAD structure
|	`				Returns: 0 for fail, 1 for OK
|---------------------------------------------------------------------------------------------------*/
int16 EstablishSSPComms(SSP_DOWNLOAD* sspD)
{
	uint16 i;
	
	/* Open the com port   */
	if(!OpenComPort(sspD)){
		return SSP_PORT_ERROR;
	}

	/* Send sync */
	sspPacket.command[0] = ssp_CMD_SYNC;
	sspPacket.commandLength = 1;
	sspPacket.retry = 3;
	sspPacket.timeout = 500;
	while(!SendSSPCommand(&sspPacket));
	if(sspPacket.replyStatus != STATUS_OK){
		CloseComPort(sspD);
		return SSP_NO_REPLY;
	}

	/* get the target info  */
	sspPacket.command[0] = ssp_CMD_EXPANSION; 
	sspPacket.command[1] = ssp_CMD_VALIDATOR;
	sspPacket.command[2] = ssp_CMD_GET_FULL_FIRMWARE;
	sspPacket.commandLength = 3;
	while(!SendSSPCommand(&sspPacket));
	if(sspPacket.replyStatus != STATUS_OK){
		CloseComPort(sspD);
		return SSP_NO_REPLY;
	}
	for(i = 0; i < sspPacket.rxData[2] - 1; i++)
		sspD->szFirmwareVersion[i] = sspPacket.rxData[4 + i];
	/* null terminate for string display  */
	sspD->szFirmwareVersion[i] = '\0';

	sspPacket.command[2] = ssp_CMD_GET_FULL_DATASET;
	sspPacket.commandLength = 3;
	while(!SendSSPCommand(&sspPacket));
	if(sspPacket.replyStatus != STATUS_OK){
		CloseComPort(sspD);
		return SSP_NO_REPLY;
	}
	for(i = 0; i < sspPacket.rxData[2] - 1; i++)
		sspD->szDatasetVersion[i] = sspPacket.rxData[4 + i];
	/* null terminate for string display  */
	sspD->szDatasetVersion[i]  = '\0';
	


	

	return SSP_REPLY_OK;


}



/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	DownloadSSPFile
|					Description: This function handles the complete SSP download process
|								
|					Parameter: Pointer to SSP_DOWNLOAD structure
|	`				Returns: Status code 
|---------------------------------------------------------------------------------------------------*/
int32 DownloadSSPFile(SSP_DOWNLOAD* sspD)
{

	int32 fh1, dataSize,i,numBlocks,numRamBytes,j;
	uint8 chk;

	/* Open the specified file  */
	fh1 = _open(sspD->szFileName,_O_BINARY);
	if(fh1 == -1)
		return OPEN_FILE_ERROR;

	// some space for the data
	sspD->fileData = (uint8*)malloc(_filelength(fh1));  
	// read the file and store the data in the buffer
	if(_read(fh1,sspD->fileData,_filelength(fh1)) == 0){
		CloseComPort(sspD);
		free(sspD->fileData);
		return READ_FILE_ERROR;
	}
	// close the file
	_close(fh1);

	/* check for ITL file  */
	if(sspD->fileData[0] != 'I' || sspD->fileData[1] != 'T' ||  sspD->fileData[2] != 'L'){
		CloseComPort(sspD);
		free(sspD->fileData);
		return NOT_ITL_FILE;
	}
	
	/* get the sizes  */
	dataSize = 0;
	for(i = 0; i < 4; i++)
		dataSize += (uint32)sspD->fileData[17 + i] << (8*(3-i));
	numBlocks = dataSize/DOWNLOAD_BLOCK_SIZE;


	/* send the programming command */
	sspPacket.command[0] = ssp_CMD_PROGRAM_TARGET;
	sspPacket.command[1] = ssp_CMD_PROGRAM_RAM;
	sspPacket.commandLength = 2;
	while(!SendSSPCommand(&sspPacket));
	if(sspPacket.replyStatus != STATUS_OK){
		CloseComPort(sspD);
		free(sspD->fileData);
		return PROG_COMMAND_FAIL;
	}
	/* now send the 128 byte header   */
	sspPacket.commandLength = HDR_BLOCK_SIZE;
	for(i = 0; i < HDR_BLOCK_SIZE; i++)
		sspPacket.command[i] = sspD->fileData[i];  
	while(!SendSSPCommand(&sspPacket));
	if(sspPacket.replyStatus != STATUS_OK){
		CloseComPort(sspD);
		free(sspD->fileData);
		return PROG_COMMAND_FAIL;
	}
	/* set to high speed */
	if(!SetDownloadSpeed(sspD,38400)){
		CloseComPort(sspD);
		free(sspD->fileData);
		return PROG_COMMAND_FAIL;
	} 
	
	WaitDelay(20);

	/* now send the ram file data  */
	numRamBytes = 0;
	for(i = 0; i < 4; i++)
		numRamBytes += (uint32)sspD->fileData[i + 7] << (8*(3-i));


	if(!SendComData(&sspD->fileData[HDR_BLOCK_SIZE],numRamBytes)){
		CloseComPort(sspD);
		free(sspD->fileData);
		return DATA_TRANSFER_FAIL;
	}
		/* wait for the checksum back from validator */
	if(!GetTargetChecksum(sspD->fileData[0x10])){
		CloseComPort(sspD);
		free(sspD->fileData);
		return DATA_TRANSFER_FAIL;		
	}


	/* delay to allow target to restart in RAM mode */
	CloseComPort(sspD);
	WaitDelay(2000);
	if(!OpenComPort(sspD)){
		return PORT_OPEN_FAIL;
	}
	/* set to high speed */
	if(!SetDownloadSpeed(sspD,38400)){
		CloseComPort(sspD);
		free(sspD->fileData);
		return DATA_TRANSFER_FAIL;
	} 
	
	/* send the update code  */
	if(!SendComData(&sspD->fileData[6],1)){
		CloseComPort(sspD);
		free(sspD->fileData);
		return DATA_TRANSFER_FAIL;
	}	

	/* wait for ack reply  */
	if(!GetTargetChecksum(ram_OK_ACK)){
		CloseComPort(sspD);
		free(sspD->fileData);
		return DATA_TRANSFER_FAIL;		
	}

	/* Resend the header   */
	if(!SendComData(sspD->fileData,HDR_BLOCK_SIZE)){
		CloseComPort(sspD);
		free(sspD->fileData);
		return DATA_TRANSFER_FAIL;
	}


	/* wait for ack reply  */
	if(!GetTargetChecksum(ram_OK_ACK)){
		CloseComPort(sspD);
		free(sspD->fileData);
		return DATA_TRANSFER_FAIL;		
	}
	
	printf("Transmitting update data...\n");
	/* now send the data in blocks, waiting for an ack */
	chk = 0;
	for(i = 0; i < numBlocks; i++){
		printf("."); /* display progress  */
		chk = 0;  /* we need to checksum each block */
		for(j = 0; j < DOWNLOAD_BLOCK_SIZE; j++)
			chk ^= sspD->fileData[HDR_BLOCK_SIZE + j + (i*DOWNLOAD_BLOCK_SIZE) + numRamBytes]; 
		/* now send the block of data */
		if(!SendComData(&sspD->fileData[HDR_BLOCK_SIZE + (i*DOWNLOAD_BLOCK_SIZE) + numRamBytes],DOWNLOAD_BLOCK_SIZE)){
			CloseComPort(sspD);
			free(sspD->fileData);
			return DATA_TRANSFER_FAIL;
		}		
		/* send the checksum  */
		if(!SendComData(&chk,1)){
			CloseComPort(sspD);
			free(sspD->fileData);
			return DATA_TRANSFER_FAIL;
		}	
		/* wait for the checksum echo  */
		if(!GetTargetChecksum(chk)){
			CloseComPort(sspD);
			free(sspD->fileData);
			return DATA_TRANSFER_FAIL;		
		}
		WaitDelay(50);
	}
	

	free(sspD->fileData);
	CloseComPort(sspD);

	/* a delay to allow the target to restart  */
	WaitDelay(2000);

	return DOWNLOAD_COMPLETE;

}



/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	SendSSPCommand
|					Description: Function to complile an SSP packet and initiate transmission
|								
|					Parameter: Pointer to SSP_PACKET structure
|	`				Returns: 0 for fail, 1 for pass
|---------------------------------------------------------------------------------------------------*/
int16 SendSSPCommand(SSP_PACKET* sspP)
{
	int16 i,j;
	uint16 CRC;
	uint8 tBuffer[255];

	/* for sync commands, reset the sync bit  */
	if(sspP->command[0] == ssp_CMD_SYNC)
		sspP->syncbit = 0x80;

	/* build the packet to transmit  */
	sspP->txPacketLength  = 5 + sspP->commandLength; 
	sspP->txPacketData[0] = SSP_STX;
	sspP->txPacketData[1] = sspP->syncbit;
	sspP->txPacketData[2] = sspP->commandLength;   
	for(i = 0; i < sspP->commandLength; i++)
		sspP->txPacketData[i + 3] = sspP->command[i];  

	/* add the CRC bytes   */
	CRC = cal_crc_loop_CCITT_A(sspP->txPacketLength - 3,&sspP->txPacketData[1],CRC_SSP_SEED,CRC_SSP_POLY);	
	sspP->txPacketData[3 + sspP->commandLength] = (uint8)(CRC & 0xFF);	
	sspP->txPacketData[4 + sspP->commandLength] = (uint8)((CRC >> 8) & 0xFF);	

	/* stuff the bytes   */
	j = 0;
	tBuffer[j++] = sspP->txPacketData[0];
	for(i = 1; i < sspP->txPacketLength; i++){
		tBuffer[j] = sspP->txPacketData[i];
		if(sspP->txPacketData[i] == SSP_STX)
			tBuffer[++j] = SSP_STX;
		 j++;
	}
	for(i = 0; i < j; i++)
		sspP->txPacketData[i] = tBuffer[i];
	sspP->txPacketLength = (uint8)j;

	if(!TransmitDataPacket(sspP))
		return 0;


	return 1;

}



/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	SSPCommsRx
|					Description: Recived byte handler for SSP - compiles into SSP packet, checks CRC and response status
|								
|					Parameter: Recieved byte
|	`				Returns: None
|---------------------------------------------------------------------------------------------------*/
void SSPCommsRx(uint8 data)
{	
	uint16 CRC;



	if (data == SSP_STX && sspPacket.rxPtr == 0){
		// packet start
		sspPacket.rxData[sspPacket.rxPtr++] = data;
	}else{	
		// if last byte was start byte, and next is not then
		// restart the packet
		if (sspPacket.checkStuff == 1){
			if (data != SSP_STX){
				sspPacket.rxData[0] = SSP_STX;
				sspPacket.rxData[1] = data;  
				sspPacket.rxPtr = 2;
			}else
				sspPacket.rxData[sspPacket.rxPtr++] = data;
			// reset stuff check flag	
			sspPacket.checkStuff = 0; 		
		}else{
			// set flag for stuffed byte check
			if (data == SSP_STX)
				sspPacket.checkStuff = 1;
			else{		
				// add data to packet
				sspPacket.rxData[sspPacket.rxPtr++] = data;
				// get the packet length
				if (sspPacket.rxPtr == 	3)
					sspPacket.rxBufferLength = sspPacket.rxData[2] + 5; 						
			}
		}

		if(sspPacket.rxPtr  == sspPacket.rxBufferLength){
			/* check the CRC sum */
			CRC = cal_crc_loop_CCITT_A(sspPacket.rxBufferLength - 3,&sspPacket.rxData[1],CRC_SSP_SEED,CRC_SSP_POLY);
			if((uint8)(CRC & 0xFF) == sspPacket.rxData[sspPacket.rxBufferLength - 2] &&  (uint8)((CRC >> 8) & 0xFF) == sspPacket.rxData[sspPacket.rxBufferLength - 1]){ 
				/* check for generic reply  */
				if(sspPacket.rxData[3] == ssp_RSP_OK)
					sspPacket.replyStatus = STATUS_OK;
			}
			
		}
	}

}





/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	cal_crc_loop_CCITT_A
|					Description: Calculates 16bit CRC value on passed parameters
|								
|					Parameter: length, pointer to byte data array, the CRC seed and the CRC polynomial
|	`				Returns: None
|---------------------------------------------------------------------------------------------------*/
uint16 cal_crc_loop_CCITT_A( int16 l, uint8* p, uint16 seed, uint16 cd)
{
	int16 i, j;
	uint16 crc = seed;
	
	for ( i = 0; i < l; ++i )
	{
		crc ^= ( p[ i ] << 8 );
		for ( j = 0; j < 8; ++j )
		{
			if ( crc & 0x8000 )
			    crc = ( crc << 1 ) ^ cd;
			else
				crc <<= 1;
		}
	}
	return crc;
}


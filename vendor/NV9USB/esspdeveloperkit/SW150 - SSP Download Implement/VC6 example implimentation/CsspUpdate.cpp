
/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|							CsspUpdate.c
|					A module to provide ITL serial com download to a commected ITL target
|								
|					
|---------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "defTypes.h"
#include <stdio.h>
#include "CsspUpdate.h"
#include "CTargetUpdate.h"




/*		Module variables			*/
SSP_DOWNLOAD sspDownload; 




/*		Module functions				*/


/*-------------------------------------------------------------------------------------------------
|						Innovative Technology Ltd
|					Function:	main
|					Description: main function - parses command line parameters and initiates target update
|								
|					Parameter: argument pointers
|	`				Returns: 
|---------------------------------------------------------------------------------------------------*/
int main(uint32 argc, int8* argv[])
{
	uint16 i,j,k;
	uint8 cmPort[10];

	
	sspDownload.comPort = -1;
	sspDownload.szFileName = 0;
	sspDownload.PortStatus = COM_PORT_CLOSED;

	/* loop arguments for download parameters  */
	for(i = 1; i < argc; i++){		
			for(j = 0; j < strlen(argv[i]); j++){
				if(argv[i][j] == '-'){
					switch(argv[i][j + 1]){
					/* the download filename  */
					case 'd':
					case 'D':
						sspDownload.szFileName = &argv[i][j + 2];
					break;
					/* the communication port number  */
					case 'p':
					case 'P':
						for(k = j + 2; k < strlen(argv[i]); k++)
							cmPort[k - j - 2] = argv[i][k];
						cmPort[k - j - 2] = '\0'; /* null terminate for string function */
						sspDownload.comPort = atoi((const char*)cmPort);
					break;
	
					}
					break;
				}
		}
	}

	/* check that required parameters are present  */
	if(sspDownload.comPort == -1){
		printf("Error in command line: No communication port parameter detected (-p)\n");
		return 0;
	}
	if(sspDownload.szFileName == 0){
		printf("Error in command line: No download filename detected (-d)\n");
		return 0;
	}
	
	printf("Connecting to target...\n");
	if(EstablishSSPComms(&sspDownload) > SSP_REPLY_OK){
		printf("Error establishing target connection!\n");
		return 0;
	}
	printf("Target firmware: %s\n",sspDownload.szFirmwareVersion );
	printf("Target dataset: %s\n",sspDownload.szDatasetVersion);
	printf("Downloading to target...\n");
	if(DownloadSSPFile(&sspDownload) > DOWNLOAD_COMPLETE){
		printf("Error downloading to target!\n");
		return 0;
	}
	if(EstablishSSPComms(&sspDownload)  > SSP_REPLY_OK){
		printf("Error establishing target connection!\n");
		return 0;
	}
	printf("\nTarget firmware: %s\n",sspDownload.szFirmwareVersion );
	printf("Target dataset: %s\n",sspDownload.szDatasetVersion);

	printf("\nUpdate complete\n");

	return 0;
}



#include "../inc/SSPComs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//#define PORT_TO_USE "/dev/ttyS0" //USB0"
#define PORT_TO_USE "/dev/ttyUSB0"
//#define PORT_TO_USE "/dev/ttyUSB1"


void RunValidator(SSP_PORT port, const unsigned char ssp_address)
{
    SSP_COMMAND_SETUP ssp_setup;
    SSP_POLL_DATA poll;
    SSP_UNIT_DATA unit_data;
    SSP_CHANNEL_DATA scd;
    SSP_SETUP_REQUEST_DATA setup_data;
    int i;
    unsigned long serial;
    //setup the required information
    ssp_setup.port = port;
	ssp_setup.Timeout = 1000;
	ssp_setup.RetryLevel = 3;
	ssp_setup.SSPAddress = ssp_address;
	ssp_setup.EncryptionStatus = NO_ENCRYPTION;

    //check validator is present
	if (ssp_sync(ssp_setup) != SSP_RESPONSE_OK)
	{
	    printf("NO VALIDATOR FOUND\n");
	    return;
	}
	printf ("Validator Found\n");

    //try to setup encryption using the default key
	if (ssp_setup_encryption(&ssp_setup,(unsigned long long)0x123456701234567LL) != SSP_RESPONSE_OK)
        printf("Encryption Failed\n");
	else
        printf("Encryption Setup\n");

	if (ssp_get_serial(ssp_setup,&serial) != SSP_RESPONSE_OK)
	{
		printf("Serial Failed\n");
		return;
	}
	printf("-------------SERIAL----------------\n");
	printf("Serial: %ld\n",serial);

	if (ssp_unit_data(ssp_setup,&unit_data) != SSP_RESPONSE_OK)
	{
		printf("Unit Data Failed\n");
		return;
	}
	printf("-------------UNIT DATA----------------\n");
	printf("UnitType: %d\n",unit_data.UnitType);
	printf("FirmwareVersion: %s\n",unit_data.FirmwareVersion);
	printf("Country: %s\n",unit_data.CountryCode);
	printf("ValueMultiplier: %ld\n",unit_data.ValueMultiplier);
	printf("ProtocolVersion: %d\n",unit_data.ProtocolVersion);


	if (ssp_channel_value_data(ssp_setup,&scd) != SSP_RESPONSE_OK)
	{
		printf("channel data failed\n");
		return;
	}
	printf("-------------Channel Data-------------\n");
	printf("NumChans: %d\n",scd.NumberOfChannels);
	for (i = 0; i < scd.NumberOfChannels; ++i)
		printf("Channel %d: %d\n",i+1,scd.ChannelData[i]);

	if (ssp_channel_security_data(ssp_setup,&scd) != SSP_RESPONSE_OK)
	{
		printf("channel security data failed\n");
		return;
	}
	printf("-------------Security Data-------------\n");
	printf("NumChans: %d\n",scd.NumberOfChannels);
	for (i = 0; i < scd.NumberOfChannels; ++i)
		printf("Channel %d: %d\n",i+1,scd.ChannelData[i]);

	if (ssp_setup_request(ssp_setup,&setup_data) != SSP_RESPONSE_OK)
	{
		printf("Setup Request Failed\n");
		return;
	}
	printf("-------------SETUP REQUEST----------------\n");
	printf("UnitType: %d\n",setup_data.UnitType);
	printf("FirmwareVersion: %s\n",setup_data.FirmwareVersion);
	printf("Country: %s\n",setup_data.CountryCode);
	printf("ValueMultiplier: %ld\n",setup_data.ValueMultiplier);
	printf("RealValueMultiplier: %ld\n",setup_data.RealValueMultiplier);
	printf("ProtocolVersion: %d\n",setup_data.ProtocolVersion);
	printf("NumChans: %d\n",setup_data.ChannelValues.NumberOfChannels);
	for (i = 0; i < setup_data.ChannelValues.NumberOfChannels; ++i)
		printf("Channel %d: %d\n",i+1,setup_data.ChannelValues.ChannelData[i]);
	printf("NumChans: %d\n",setup_data.ChannelSecurity.NumberOfChannels);
	for (i = 0; i < setup_data.ChannelSecurity.NumberOfChannels; ++i)
		printf("Channel %d: %d\n",i+1,setup_data.ChannelSecurity.ChannelData[i]);
}

int main(int argc, char *argv[])
{
    int ssp_address;
	SSP_PORT port;


    //check for command line arguments - first is port, second is ssp address
	if (argc <= 2)
	{
	    printf("Usage: ValidatorInfo <port> <sspaddress>\n");
	    return 1;
    }

    //open the com port
    printf("PORT: %s\n",argv[1]);
    port = OpenSSPPort(argv[1]);
    if (port == -1)
	{
	    printf("Port Error\n");
        return 1;
	}

    //set the ssp address
    ssp_address = atoi(argv[2]);
    printf("SSP ADDRESS: %d\n",ssp_address);


    //run the validator
	RunValidator(port,ssp_address);
	CloseSSPPort(port);
	return 0;


//    printf("%x\n",DownloadFileToTarget("/home/rstacey/CNY01609_NV02004000000TES_IF_01.bv1",PORT_TO_USE,0));

}

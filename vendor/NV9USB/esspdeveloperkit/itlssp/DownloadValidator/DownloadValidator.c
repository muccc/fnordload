#include "../inc/SSPComs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



int main(int argc, char *argv[])
{
    int ssp_address;
    unsigned long last_status = 0xFFFFFFFFL;
    unsigned long status;
    //check for command line arguments - first is port, second is ssp address
    if (argc <= 3)
    {
        printf("Usage: DownloadValidator <port> <sspaddress> <file>\n");
        return 1;
    }

    //open the com port
    printf("PORT: %s\n",argv[1]);
    //set the ssp address
    ssp_address = atoi(argv[2]);
    printf("SSP ADDRESS: %d\n",ssp_address);

    printf("File : %s\n",argv[3]);

    printf("Blocks To Download: %d\n",DownloadFileToTarget(argv[3],argv[1],ssp_address));
    do
    {
        status = GetDownloadStatus();
        if (status != last_status)
        {
            if (status <DOWNLOAD_COMPLETE)
            {
                printf("Block %ld\n",status);
                last_status = status;
            }
        }
    }
    while (status < DOWNLOAD_COMPLETE);
    printf("Finished: %lx\n",status);
	return 0;


//

}

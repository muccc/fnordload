// ITLSSPProc.cpp : Defines the entry point for the DLL application.
//

#define CCONV _stdcall
#define NOMANGLE

#include "stdafx.h"
#include "ITLSSPProc.h"
#include <stdlib.h>
#include "random.h"
#include "encryption.h"


#define VER_MAJ  1  // not > 255
#define VER_MIN	 0	// not > 255
#define VER_REV	 0	// not > 255


unsigned int encPktCount = 0;


/*		Linear Feedback Shift Registers			*/
#define LFSR(n)    {if (n&1) n=((n^0x80000055)>>1)|0x80000000; else n>>=1;}
/*		Rotate32								*/
#define ROT(x, y)  (x=(x<<y)|(x>>(32-y)))



BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:

			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:

			break;
    }
    return TRUE;
}


typedef enum{
	KEY_GENERATOR,
	KEY_MODULUS,
	KEY_HOST_INTER,
	KEY_HOST_RANDOM,
	KEY_SLAVE_INTER,
	KEY_SLAVE_RANDOM,
	KEY_HOST,
	KEY_SLAVE,
}SSP_KEY_INDEX;


NOMANGLE int CCONV GetDLLVersion(unsigned char* ver)
{
	ver[0] = VER_MAJ;
	ver[1] = VER_MIN;
	ver[2] = VER_REV;

	return 1;

}



/*    DLL function call to generate host intermediate numbers to send to slave  */
NOMANGLE int CCONV InitiateSSPHostKeys(__int64*  keyArray)
{
	
	
	__int64 swap = 0;

	/* create the two random prime numbers  */
	keyArray[KEY_GENERATOR] = GeneratePrime();
	keyArray[KEY_MODULUS] = GeneratePrime();
	/* make sure Generator is larger than Modulus   */
	if (keyArray[KEY_GENERATOR] > keyArray[KEY_MODULUS])
	{
		swap = keyArray[KEY_GENERATOR];
		keyArray[KEY_GENERATOR] = keyArray[KEY_MODULUS];
		keyArray[KEY_MODULUS] = swap;
	}


	if(CreateHostInterKey(keyArray)== -1)
		return 0;

	
	/* reset the apcket counter here for a successful key neg  */
	encPktCount = 0;

	return 1;
}

/* creates the host encryption key   */
NOMANGLE int CCONV CreateSSPHostEncryptionKey(__int64* keyArray)
{
	keyArray[KEY_HOST] = XpowYmodN(keyArray[KEY_SLAVE_INTER],keyArray[KEY_HOST_RANDOM],keyArray[KEY_MODULUS]);

	return 1;
}



NOMANGLE int CCONV EncryptSSPPacket(unsigned char* dataIn, unsigned char* dataOut, unsigned char* lengthIn,unsigned char* lengthOut, unsigned __int64* key)
{
	#define FIXED_PACKET_LENGTH   7
	unsigned char pkLength,i,packLength = 0;
	unsigned short crc;
	unsigned char tmpData[255];

	
	pkLength = *lengthIn + FIXED_PACKET_LENGTH;
    
	/* find the length of packing data required */
	if(pkLength % C_MAX_KEY_LENGTH != 0){
		packLength = C_MAX_KEY_LENGTH - (pkLength % C_MAX_KEY_LENGTH);
	}
	pkLength += packLength;

	tmpData[0] = *lengthIn; /* the length of the data without packing */

	/* add in the encrypted packet count   */
	for(i = 0; i < 4; i++)
		tmpData[1 + i] = (unsigned char)((encPktCount >> (8*i) & 0xFF));


	for(i = 0; i < *lengthIn; i++)
		tmpData[i + 5] = dataIn[i];

	/* add random packing data  */
	for(i = 0; i < packLength; i++)
		tmpData[5 + *lengthIn + i] = (unsigned char)(rand() % 255);
	/* add CRC to packet end   */
	
	crc = cal_crc_loop_CCITT_A(pkLength - 2,tmpData,CRC_SSP_SEED,CRC_SSP_POLY);
	
	tmpData[pkLength - 2] = (unsigned char)(crc & 0xFF);
	tmpData[pkLength - 1] = (unsigned char)((crc >> 8) & 0xFF);

	if (aes_encrypt( C_AES_MODE_ECB,(unsigned char*)key,C_MAX_KEY_LENGTH,NULL,0,tmpData,&dataOut[1],pkLength) != E_AES_SUCCESS)
							return 0;
	
	pkLength++; /* increment as the final length will have an STEX command added   */	
	*lengthOut = pkLength;
	dataOut[0] = SSP_STEX;

	encPktCount++;  /* incremnet the counter after a successful encrypted packet   */

	return 1;
}


NOMANGLE int CCONV DecryptSSPPacket(unsigned char* dataIn, unsigned char* dataOut, unsigned char* lengthIn,unsigned char* lengthOut, unsigned __int64* key)
{


	if (aes_decrypt( C_AES_MODE_ECB,(unsigned char*)key,C_MAX_KEY_LENGTH,NULL,0,dataOut,dataIn,*lengthIn) != E_AES_SUCCESS)
							return 0;	

	

	return 1;
}





/* Creates a host intermediate key */
int CreateHostInterKey(__int64* keyArray)
{

	if (keyArray[KEY_GENERATOR] ==0 || keyArray[KEY_MODULUS] ==0 )
		return -1;

	keyArray[KEY_HOST_RANDOM]= (__int64) (GenerateRandomNumber() % MAX_RANDOM_INTEGER);
	keyArray[KEY_HOST_INTER] = XpowYmodN(keyArray[KEY_GENERATOR],keyArray[KEY_HOST_RANDOM],keyArray[KEY_MODULUS] );

	return 0;	
}




/* -----------------------------------------------------------------------------------------------------------------*/
/*    These function are not usually included in the host implimentation - they inlcuded here so the user can		*/
/*	  test the key functions in the host and system	and so the user can impliment them in the slave					*/
/* -----------------------------------------------------------------------------------------------------------------*/

/* DLL function call to test user implimentation - emulates slave functions, uses host data to generate HOST and SLAVE keys 
|  These keys should be the same for a successful key negotation                                            */
NOMANGLE int CCONV TestSSPSlaveKeys(__int64*  keyArray)
{

	
	if(CreateSlaveInterKey(keyArray) == -1)
		return 0;
		
	CreateSSPHostEncryptionKey(keyArray);
	CreateSlaveEncryptionKey(keyArray);

	if(keyArray[KEY_HOST] == keyArray[KEY_SLAVE])
		return 1;
	else
		return 0;
}


/* Creates a slave encryption key - test function only - this is usually only implimented in the slave */
int CreateSlaveEncryptionKey(__int64* keyArray)
{
	keyArray[KEY_SLAVE] = XpowYmodN(keyArray[KEY_HOST_INTER],keyArray[KEY_SLAVE_RANDOM],keyArray[KEY_MODULUS]);

	return 0;
}


/* creates a slave intermediate key - test function only - this is usually implimented only in the slave  */
int CreateSlaveInterKey(__int64* keyArray)
{
	if ( keyArray[KEY_GENERATOR] ==0 || keyArray[KEY_MODULUS] ==0 )
		return -1;

	keyArray[KEY_SLAVE_RANDOM] = (__int64) (GenerateRandomNumber() % MAX_RANDOM_INTEGER);
	keyArray[KEY_SLAVE_INTER] = XpowYmodN(keyArray[KEY_GENERATOR],keyArray[KEY_SLAVE_RANDOM],keyArray[KEY_MODULUS]);
	return 0;
}

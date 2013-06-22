#define CCONV _stdcall
#define NOMANGLE



int CreateHostInterKey(__int64* keyArray);
int CreateSlaveInterKey(__int64* keyArray);
int CreateSlaveEncryptionKey(__int64* keyArray);


NOMANGLE int CCONV InitiateSSPHostKeys(__int64*  keyArray);
NOMANGLE int CCONV TestSSPSlaveKeys(__int64*  keyArray);
NOMANGLE int CCONV CreateSSPHostEncryptionKey(__int64* keyArray);
NOMANGLE int CCONV EncryptSSPPacket(unsigned char* dataIn, unsigned char* dataOut, unsigned char* lengthIn,unsigned char* lengthOut, unsigned __int64* key);
NOMANGLE int CCONV DecryptSSPPacket(unsigned char* dataIn, unsigned char* dataOut, unsigned char* lengthIn,unsigned char* lengthOut, unsigned __int64* key);
NOMANGLE int CCONV GetDLLVersion(unsigned char* ver);






#ifndef _RANDOM_H
#define _RANDOM_H


#define MAX_RANDOM_INTEGER		2147483648 //Should make these numbers massive to be more secure
#define MAX_PRIME_NUMBER		2147483648 //Bigger the number the slower the algorithm
								
/*		Linear Feedback Shift Registers			*/
#define LFSR(n)    {if (n&1) n=((n^0x80000055)>>1)|0x80000000; else n>>=1;}
/*		Rotate32								*/
#define ROT(x, y)  (x=(x<<y)|(x>>(32-y)))


unsigned __int64 GeneratePrime(void);
bool MillerRabin (__int64 n, __int64 trials);
bool IsItPrime (__int64 n, __int64 a);
__int64 XpowYmodN(__int64 x, __int64 y, __int64 N);
unsigned __int64 GenerateRandomNumber(void);
__int64 GetRTSC( void );



#endif



#include "stdafx.h"
#include <stdlib.h>
#include "random.h"



/*	Generates a large prime number by
|	choosing a randomly large integer, and ensuring the value is odd
|	then uses the miller-rabin primality test on it to see if it is prime
|	if not the value gets increased until it is prime			*/

unsigned __int64 GeneratePrime(void)
{
	unsigned __int64 tmp = 0;

	tmp	=  GenerateRandomNumber();
	tmp	%= MAX_PRIME_NUMBER;

	/*  ensure it is an odd number	*/
	if ((tmp & 1)==0)
		tmp += 1;
	
	/* test for prime  */
	if (MillerRabin(tmp,5)==true) return tmp;
	/*  increment until prime  */
	do
	{
		tmp+=2;	
	} while (MillerRabin(tmp,5)==false);
		
	return tmp;
}


/*	Performs the miller-rabin primality test on a guessed prime n.
|	trials is the number of attempts to verify this, because the function
|	is not 100% accurate it may be a composite.  However setting the trial
|	value to around 5 should guarantee success even with very large primes		*/

bool MillerRabin (__int64 n, __int64 trials) 
{ 
	__int64 a = 0; 

	for (__int64 i=0; i<trials; i++)
	{ 
		a = (rand() % (n-3))+2;/* gets random value in [2..n-1] */
		
		if (IsItPrime (n,a)==false) 
		{ 
			return false; 
			/*n composite, return false */
		} 
	} return true; /* n probably prime */
} 


/* Checks the integer n for primality		*/

bool IsItPrime (__int64 n, __int64 a) 
{ 
	__int64 d = XpowYmodN(a, n-1, n); 
	if (d==1) 
		return true; 
	else 
		return false; 
	 
} 


/*		Raises X to the power Y in modulus N
		the values of X, Y, and N can be massive, and this can be 
		acheived by first calculating X to the power of 2 then 
		using power chaining over modulus N				*/

__int64 XpowYmodN(__int64 x, __int64 y, __int64 N)
{
	if (y==1) return (x % N);

	__int64 result = 1;
	
	const __int64 oneShift63 = (( __int64) 1) << 63;
	
	
	for (int i = 0; i < 64; y <<= 1, i++){
		result = result * result % N;
		if (y & oneShift63)
			result = result * x % N;
	}
	
	return result;

}


/*	Generates a random number by first getting the RTSC of the CPU, then 
|	thanks to Ilya O. Levin uses a Linear feedback shift register.
|	The RTSC is then added to fill the 64-bits					*/

unsigned __int64 GenerateRandomNumber(void)
{
	unsigned long rnd = 0x41594c49;
	unsigned long x   = 0x94c49514;
	__int64  n;
	unsigned __int64 ret;


	LFSR(x); 

	n = GetRTSC();

	rnd ^= n^x; 

	ROT(rnd,7);

	ret = (unsigned __int64)GetRTSC() + (unsigned __int64)rnd;

	return ret;
} 







/*	Returns the Read Time Stamp Counter of the CPU
|	The instruction returns in registers EDX:EAX the count of ticks from processor reset.
|	Added in Pentium. Opcode: 0F 31.				*/

__int64 GetRTSC( void )
{
	int tmp1 = 0;
	int tmp2 = 0;
	
	__asm
	{
		RDTSC;			/* Clock cycles since CPU started		*/
		mov tmp1, eax;
		mov tmp2, edx;
	}

	return ((__int64)tmp1 * (__int64)tmp2);
}


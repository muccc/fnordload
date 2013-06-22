#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	unsigned short crc = 0xFFFF;
	unsigned char p = 0x80;

	crc ^= ( p << 8 );

	printf("CRC: %x\n", crc);
}


#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "netint.h"

#define MAX_OCTET_1	0x00000000000000ff
#define MAX_OCTET_2	0x000000000000ffff
#define MAX_OCTET_3	0x0000000000ffffff
#define MAX_OCTET_4	0x00000000ffffffff
#define MAX_OCTET_5	0x000000ffffffffff
#define MAX_OCTET_6	0x0000ffffffffffff
#define MAX_OCTET_7	0x00ffffffffffffff
#define MAX_OCTET_8	0xffffffffffffffff

uint32_t data_ntohl(const void *data, size_t size)
{
	uint32_t value;
	void *p;

	if( size > sizeof(uint32_t) ) {
		size = sizeof(uint32_t);
	}

	value = 0;
	p = (void *)&value;

	memcpy(p + sizeof(uint32_t) - size, data, size);

	return ntohl(value);
}

int data_htonl(void *buf, uint32_t val)
{
	int len;
	uint32_t nval;

	len = int_max_octet(val);
	nval = htonl(val);
	//memcpy(buf, &nval, len);
	memcpy(buf, (void *)&nval+(sizeof(nval)-len), len);

	return len;
}

int int_max_octet(uint64_t value)
{
	if( value <= MAX_OCTET_1  ) {
		return 1;
	}
	else if( value <= MAX_OCTET_2 ) {
		return 2;
	}
	else if( value <= MAX_OCTET_3 ) {
		return 3;
	}
	else if( value <= MAX_OCTET_4 ) {
		return 4;
	}
	/*else if( value <= MAX_OCTET_5 ) {
		return 5;
	}
	else if( value <= MAX_OCTET_6 ) {
		return 6;
	}
	else if( value <= MAX_OCTET_7 ) {
		return 7;
	}
	else if( value <= MAX_OCTET_8 ) {
		return 8;
	}*/

	return -1;
}

int byteorder(void)
{
	union {
		uint16_t s;
		uint8_t c[sizeof(short)];
	} un;

	un.s = 0x0102;

	// big-endian
	if( un.c[0] == 1 && un.c[1] == 2 ) {
		return BOBIG;
	}
	// little-endian
	else if( un.c[0] == 2 && un.c[1] == 1 ) {
		return BOLITTLE;
	}
	// error
	else {
		return BOERROR;
	}

	return BOERROR;
}





#include "bits.h"
#include <stdio.h>

unsigned long bits_get(const void *buf, long pos, int n)
{
    long bytepos, bitpos;
    long mask, result;
    const unsigned char *ptr = (const unsigned char *)buf;
    int shiftwidth;

    bytepos = pos / 8;
    bitpos = pos % 8;

    mask = (1 << n) - 1;

    if( bitpos + n <= 8 ) {
        //printf("bitpos \n");
        shiftwidth = 8 - (bitpos + n);
        return (ptr[bytepos] >> shiftwidth) & mask;
    }

    result = 0;
    while( bitpos + n > 8 ) {
        result = (result << 8)  | ptr[bytepos];
        n -= (8 - bitpos);
        bitpos = 0;
        bytepos++;
    }

    result <<= n;
    result |= ptr[bytepos] >> (8 - n);
    return result & mask;
}

void bits_set(void *buf, long pos, int n, unsigned long value)
{
    long bytepos, bitpos;
    unsigned long mask;
    unsigned char *ptr = (unsigned char *)buf;
    int c, bits, shiftwidth;

    mask = (1 << n) - 1;

    bytepos = pos / 8;
    bitpos = pos % 8;

    if( bitpos + n <= 8) {
        shiftwidth = 8 - (bitpos + n);
        c = ptr[bytepos] & ~(mask << shiftwidth);
        c |= value << shiftwidth;
        ptr[bytepos] = c;
        return;
    }

    while( bitpos + n > 8 ) {
        bits = 8 - bitpos;
        shiftwidth = n - bits;
        mask = (1 << bits) - 1;
        c = (value >> shiftwidth) & mask;
        ptr[bytepos] = (ptr[bytepos] & ~mask) | c;
        n -= (8 - bitpos);
        bitpos = 0;
        bytepos++;
    }

    mask = (1 << n) - 1;
    shiftwidth = 8 - n;
    c = ptr[bytepos] & ~(mask << shiftwidth);
    c |= value << shiftwidth;
    ptr[bytepos] = c;
}

int bits_size(int n)
{
    return n / 8 + (n % 8 > 0 ? 1 : 0);
}

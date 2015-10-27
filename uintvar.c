
#include <string.h>
#include "uintvar.h"

int uintvar_pack(void *buf, long len, unsigned long value)
{
    int i, packlen, pos;
    unsigned char pack[UINTVAR32_LEN];

    pack[UINTVAR32_LEN-1] = value & 0x7f;
    value >>= 7;
    for( i = UINTVAR32_LEN-2 ; value > 0 && i >= 0 ; i-- ) {
        pack[i] = 0x80 | (value & 0x7f);
        value >>= 7;
    }

    pos = i + 1;
    packlen = UINTVAR32_LEN - pos;
    if( len < packlen )
        return -1;

    memcpy(buf, pack+pos, packlen);

    return packlen;
}

int uintvar_unpack(unsigned long *value, const void *buf, long len)
{
    int i;
    unsigned long val = 0;
    unsigned char *ptr = (unsigned char *)buf;

    for( i = 0 ; i < UINTVAR32_LEN && i < len ; i++ ) {
        val = (val << 7) | (ptr[i] & 0x7f);
        if( !(ptr[i] & 0x80) ) {
            *value = val;
            return i + 1;
        }
    }

    return -1;
}

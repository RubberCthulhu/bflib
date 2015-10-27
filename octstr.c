
#define _GNU_SOURCE

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <iconv.h>
#include "octstr.h"
#include "bits.h"
#include "uintvar.h"
#include "netint.h"

struct Octstr {
    // TODO Change type of *s from 'unsigned char' to 'char'.
    unsigned char *s;
    size_t len;
    size_t size;
};

static void octstr_grow(octstr_t *s, size_t len);
static size_t octstr_calc_size(size_t len);

static long min(long a, long b);

static octstr_t * octstr_find_and_replace1(octstr_t *s, octstr_t *what, octstr_t *replace,
    long (*find)(octstr_t *, long, octstr_t *));

octstr_t * octstr_create(const void *buf, size_t len)
{
    octstr_t *s;

    s = (octstr_t *)malloc(sizeof(octstr_t));
    memset(s, 0, sizeof(*s));
    octstr_str_replace(s, buf, len);

    return s;
}

octstr_t * octstr_create_from_str(const char *str)
{
    return octstr_create(str, strlen(str));
}

void octstr_destroy(octstr_t *s)
{
    if( s->s && s->size > 0 ) {
        free(s->s);
    }
    free(s);
}

void octstr_destructor(void *ptr)
{
    octstr_destroy((octstr_t *)ptr);
}

size_t octstr_len(octstr_t *s)
{
    return s->len;
}

void octstr_erase(octstr_t *s)
{
    octstr_str_replace(s, NULL, 0);
}

octstr_t * octstr_copy(octstr_t *s)
{
    return octstr_create(s->s, s->len);
}

void octstr_replace(octstr_t *dst, octstr_t *src)
{
    octstr_str_replace(dst, src->s, src->len);
}

void octstr_str_replace(octstr_t *dst, const void *src, size_t len)
{
    //if( len > dst->size ) {
    //    octstr_resize(dst, len);
    //}
    octstr_grow(dst, len);

    if( len > 0 ) {
        memcpy(dst->s, src, len);
        dst->s[len] = 0;
        dst->len = len;
    }
    else {
        dst->len = 0;
    }
}

void octstr_append(octstr_t *dst, octstr_t *src)
{
    octstr_str_append(dst, src->s, src->len);
}

void octstr_str_append(octstr_t *dst, const void *src, size_t len)
{
    int new_len;

    if( len > 0 ) {
        new_len = dst->len + len;
        //if( dst->size < new_len ) {
        //    octstr_resize(dst, new_len);
        //}
        octstr_grow(dst, new_len);

        memcpy(dst->s + dst->len, src, len);
        dst->s[new_len] = 0;
        dst->len = new_len;
    }
}

void octstr_append_str(octstr_t *dst, const char *str)
{
    return octstr_str_append(dst, str, strlen(str));
}

void octstr_append_oct(octstr_t *s, unsigned int oct)
{
    octstr_grow(s, s->len+1);
    s->s[s->len++] = oct;
    s->s[s->len] = 0;
}

void octstr_fill(octstr_t *s, int c, int pos, int n)
{
    int len;

    len = pos + n;
    if( len > 0 ) {
        //if( s->size < len ) {
        //    octstr_resize(s, len);
        //}
        octstr_grow(s, len);

        memset(s->s + pos, c, n);
        s->s[len] = 0;
        s->len = len > s->len ? len : s->len;
    }
}

void octstr_resize(octstr_t *s, size_t len)
{
    // +1 byte for '\0'.
    len = octstr_calc_size(len+1);
    if( len == 0 ) {
        if( s->s ) {
            free(s->s);
        }
        s->s = NULL;
    }
    else if( s->s ) {
        s->s = (unsigned char *)realloc(s->s, len);
    }
    else {
        s->s = (unsigned char *)malloc(sizeof(unsigned char) * len);
    }

    if( s->s ) {
        s->s[len-1] = 0;
    }
    s->size = len;
    s->len = len < s->len ? len : s->len;
}

static void octstr_grow(octstr_t *s, size_t len)
{
    int size;

    // +1 byte for '\0'.
    size = octstr_calc_size(len+1);
    if( size > s->size ) {
        if( s->s ) {
            s->s = (unsigned char *)realloc(s->s, size);
        }
        else {
            s->s = (unsigned char *)malloc(size);
        }

        s->size = size;
    }
}

static size_t octstr_calc_size(size_t len)
{
    return len + 1024 - (len % 1024);
}

octstr_t * octstr_substr(octstr_t *s, int pos, int n)
{
    octstr_t *substr;
    size_t len;

    len = octstr_len(s);
    if( pos >= len ) {
        substr = octstr_create(NULL, 0);
    }
    else {
        n = pos + n <= len ? n : len - pos;
        substr = octstr_create(s->s+pos, n);
    }

    return substr;
}

unsigned int octstr_get_oct(octstr_t *s, int pos)
{
    if( pos >= octstr_len(s) )
        return -1;

    return s->s[pos];
}

size_t octstr_replace_substr(octstr_t *dst, octstr_t *src, int pos, int n)
{
    size_t len;

    len = octstr_len(src);
    if( pos >= len ) {
        octstr_str_replace(dst, NULL, 0);
        n = 0;
    }
    else {
        n = pos + n <= len ? n : len - pos;
        octstr_str_replace(dst, src->s+pos, n);
    }

    return n;
}

void octstr_delete(octstr_t *s, int pos, int n)
{
    octstr_t *substr1, *substr2;

    substr1 = octstr_substr(s, 0, pos);
    substr2 = octstr_substr(s, pos + n, octstr_len(s) - (pos + n));

    octstr_replace(s, substr1);
    octstr_append(s, substr2);
    octstr_destroy(substr1);
    octstr_destroy(substr2);
}

long octstr_oct_index(octstr_t *s, long pos, unsigned int oct)
{
    unsigned char *p = (unsigned char *)octstr_get_cstr(s);
    long len = octstr_len(s);
    int i;

    for( i = pos ; i < len ; i++ ) {
        if( p[i] == oct )
            return i;
    }

    return -1;
}

long octstr_str_index(octstr_t *s, long pos, const char *str)
{
    char *p, *ptr;

    if( pos >= octstr_len(s) )
        return -1;

    p = octstr_get_cstr(s);
    if( (ptr = strstr(p + pos, str)) != NULL ) {
        return ptr - p;
    }

    return -1;
}

long octstr_caseindex(octstr_t *s, long pos, octstr_t *str)
{
    return octstr_str_caseindex(s, pos, octstr_get_cstr(str));
}

long octstr_str_caseindex(octstr_t *s, long pos, const char *str)
{
    char *p, *ptr;

    if( pos >= octstr_len(s) )
        return -1;

    p = octstr_get_cstr(s);
    if( (ptr = strcasestr(p + pos, str)) != NULL ) {
        return ptr - p;
    }

    return -1;
}

long octstr_index(octstr_t *s, long pos, octstr_t *str)
{
    return octstr_str_index(s, pos, octstr_get_cstr(str));
}

unsigned int octstr_get_bits(octstr_t *s, long pos, int n)
{
    int len;

    len = (pos + n) / 8 + ((pos + n) % 8 > 0 ? 1 : 0);
    if( s->len >= len ) {
        return bits_get(s->s, pos, n);
    }

    return 0;
}

void octstr_set_bits(octstr_t *s, long pos, int n, unsigned long value)
{
    int len;

    len = (pos + n) / 8 + ((pos + n) % 8 > 0 ? 1 : 0);
    octstr_grow(s, len);

    bits_set(s->s, pos, n, value);
    s->len = len > s->len ? len : s->len;
    s->s[s->len] = 0;
}

long octstr_append_uintvar(octstr_t *s, unsigned long value)
{
    long len;
    char buf[UINTVAR32_LEN];

    len = uintvar_pack(buf, UINTVAR32_LEN, value);
    octstr_str_append(s, buf, len);

    return len;
}

long octstr_unpack_uintvar(octstr_t *s, long pos, unsigned long *value)
{
    long len;

    if( pos >= octstr_len(s) )
        return -1;

    if( (len = uintvar_unpack(value, octstr_get_cstr(s) + pos, octstr_len(s) - pos)) != -1 ) {
        return pos + len;
    }

    return -1;
}

long octstr_append_htonl(octstr_t *s, unsigned long value, int min)
{
    long len, n;
    char buf[4];

    n = data_htonl(buf, value);
    len = n;
    if( !min ) {
        while( len++ < 4 ) {
            octstr_append_oct(s, 0);
        }
    }

    octstr_str_append(s, buf, n);

    return len;
}

long octstr_unpack_ntohl(octstr_t *s, long pos, long len, unsigned long *value)
{
    uint32_t v;
    unsigned char *ptr;

    if( pos+len >= octstr_len(s) )
        return -1;

    ptr = (unsigned char *)octstr_get_cstr(s);
    v = data_ntohl(ptr+pos, len);
    if( value )
        *value = v;

    return pos + len;
}

long octstr_tol(octstr_t *s, long *value)
{
    char *ptr, *endptr;
    long v;

    if( octstr_len(s) == 0 )
        return -1;

    ptr = octstr_get_cstr(s);
    v = strtol(ptr, &endptr, 10);
    if( endptr-ptr != octstr_len(s) )
        return -1;

    if( value )
        *value = v;

    return endptr - ptr;
}

char * octstr_get_cstr(octstr_t *s)
{
    return (char *)s->s;
}

octstr_t * octstr_to_hex(octstr_t *s)
{
    octstr_t *hex;
    int len, i;
    char buf[3];

    hex = octstr_create(NULL, 0);
    len = octstr_len(s);
    for( i = 0 ; i < len ; i++ ) {
        sprintf(buf, "%.2x", (unsigned int)s->s[i]);
        octstr_str_append(hex, buf, 2);
    }

    return hex;
}

octstr_t * octstr_load_from_file(const char *path)
{
    octstr_t *s;
    int fd, n;
    char buf[1024000];

    if( (fd = open(path, O_RDONLY)) == -1 ) {
        return NULL;
    }

    s = octstr_create(NULL, 0);
    while( (n = read(fd, buf, 1024000)) > 0 ) {
        octstr_str_append(s, buf, n);
    }

    close(fd);

    if( n < 0 ) {
        octstr_destroy(s);
        return NULL;
    }

    return s;
}

int octstr_save_to_file(octstr_t *s, const char *path)
{
    int fd, n, len;

    if( (fd = open(path, O_CREAT|O_WRONLY, 0664)) == -1 ) {
        return -1;
    }

    len = 0;
    while( len < s->len ) {
        n = write(fd, s->s + len, s->len-len);
        if( n == -1 ) {
            close(fd);
            return -1;
        }

        len += n;
    }

    close(fd);
    return len;
}

octstr_t * octstr_sprintf(const char *fmt, ...)
{
    va_list ap;
    octstr_t *s;
    int n;

    s = octstr_create_from_str("");
    if( fmt != NULL ) {
        octstr_grow(s, strlen(fmt));
        va_start(ap, fmt);
        n = vsnprintf((char *)s->s, s->size, fmt, ap);
        va_end(ap);
        while( n >= s->size ) {
            octstr_grow(s, n+1);
            va_start(ap, fmt);
            n = vsnprintf((char *)s->s, s->size, fmt, ap);
            va_end(ap);
        }

        s->len = n;
    }

    return s;
}

static long min(long a, long b)
{
    return a < b ? a : b;
}

int octstr_cmp(octstr_t *s1, octstr_t *s2)
{
    int cmp;
    long n, len1, len2;

    len1 = octstr_len(s1);
    len2 = octstr_len(s2);
    n = min(len1, len2);
    cmp = memcmp(octstr_get_cstr(s1), octstr_get_cstr(s2), n);
    if( cmp == 0 ) {
        if( len1 < len2 )
            cmp = -1;
        else if( len1 > len2 )
            cmp = 1;
    }

    return cmp;
}

int octstr_casecmp(octstr_t *s1, octstr_t *s2)
{
    return strcasecmp(octstr_get_cstr(s1), octstr_get_cstr(s2));
}

int octstr_str_cmp(octstr_t *s1, const char *s2)
{
    octstr_t *tmp;
    int r;

    tmp = octstr_create_from_str(s2);
    r = octstr_cmp(s1, tmp);
    octstr_destroy(tmp);

    return r;
}

int octstr_str_casecmp(octstr_t *s1, const char *s2)
{
    return strcasecmp(octstr_get_cstr(s1), s2);
}

#define SPACE_CHARS " \n\r\t"
octstr_t * octstr_strip(octstr_t *s)
{
    long pos1, pos2, len;
    char *p;
    octstr_t *tmp;

    p = octstr_get_cstr(s);
    len = octstr_len(s);
    if( len == 0 )
        return s;

    for( pos1 = 0 ; pos1 < len && strchr(SPACE_CHARS, p[pos1]) != NULL ; pos1++ ) {}
    if( pos1 == len ) {
        octstr_erase(s);
        return s;
    }

    for( pos2 = len-1 ; pos2 >= 0 && strchr(SPACE_CHARS, p[pos2]) != NULL; pos2-- ) {}

    tmp = octstr_substr(s, pos1, pos2+1-pos1);
    octstr_replace(s, tmp);
    octstr_destroy(tmp);

    return s;
}

octstr_t * octstr_iconv(octstr_t *s, const char *to, const char *from)
{
    iconv_t cd;
    char buf[1024], *out, *in;
    size_t outleft, inleft;
    octstr_t *s1;

    if( (cd = iconv_open(to, from)) == (iconv_t)-1 )
        return NULL;

    s1 = octstr_create_from_str("");
    in = octstr_get_cstr(s);
    inleft = octstr_len(s);
    while( inleft > 0 ) {
        out = buf;
        outleft = 1024;
        if( iconv(cd, &in, &inleft, &out, &outleft) == (size_t)-1 ) {
            if( errno != E2BIG ) {
                octstr_destroy(s1);
                iconv_close(cd);
                return NULL;
            }
        }

        octstr_str_append(s1, buf, 1024-outleft);
    }

    iconv_close(cd);
    
    return s1;
}

octstr_t * octstr_find_and_replace(octstr_t *s, octstr_t *what, octstr_t *replace)
{
    return octstr_find_and_replace1(s, what, replace, octstr_index);
}

octstr_t * octstr_case_find_and_replace(octstr_t *s, octstr_t *what, octstr_t *replace)
{
    return octstr_find_and_replace1(s, what, replace, octstr_caseindex);
}

static octstr_t * octstr_find_and_replace1(octstr_t *s, octstr_t *what, octstr_t *replace,
    long (*find)(octstr_t *, long, octstr_t *))
{
    long pos, pos1;
    octstr_t *s1, *tmp;

    s1 = octstr_create(NULL, 0);
    pos = 0;
    while( (pos1 = find(s, pos, what)) > 0 ) {
        tmp = octstr_substr(s, pos, pos1-pos);
        octstr_append(s1, tmp);
        octstr_append(s1, replace);
        octstr_destroy(tmp);
        pos = pos1 + octstr_len(what);
    }

    tmp = octstr_substr(s, pos, octstr_len(s)-pos);
    octstr_append(s1, tmp);
    octstr_destroy(tmp);

    return s1;
}


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "die.h"

void die(const char *fmt, ...)
{
    va_list ap;

    if( fmt != NULL ) {
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        printf("\n");
    }

    exit(EXIT_FAILURE);
}

void die_status(int status, const char *fmt, ...)
{
    va_list ap;

    if( fmt != NULL ) {
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        printf("\n");
    }

    exit(status);
}

void die_full(const char *file, long line, const char *func, int status, const char *fmt, ...)
{
    va_list ap;

    if( fmt != NULL ) {
        printf("%s:%ld:%s: ", file, line, func);
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        printf("\n");
    }

    exit(status);
}

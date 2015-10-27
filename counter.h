
#ifndef COUNTER_H
#define COUNTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>

#define COUNTER_MIN 0
#define COUNTER_MAX ULONG_MAX

typedef struct Counter counter_t;

counter_t * counter_create();
void counter_destroy(counter_t *counter);
unsigned long counter_next(counter_t *counter);
void counter_reset(counter_t *counter);

#define COUNTER_CTL_MIN     0x01
#define COUNTER_CTL_MAX     0x02
#define COUNTER_CTL_CURRENT 0x04
void counter_ctl(counter_t *counter, int opt, unsigned long value);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* COUNTER_H */

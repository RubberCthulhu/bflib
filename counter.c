
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "counter.h"

struct Counter {
    unsigned long n;
    unsigned long min;
    unsigned long max;
    pthread_mutex_t lock;
};

counter_t * counter_create()
{
    counter_t *counter;

    counter = malloc(sizeof(counter_t));
    memset(counter, 0, sizeof(*counter));
    counter->min = COUNTER_MIN;
    counter->max = COUNTER_MAX;
    counter->n = counter->min;
    pthread_mutex_init(&counter->lock, NULL);

    return counter;
}

void counter_destroy(counter_t *counter)
{
    pthread_mutex_destroy(&counter->lock);
    free(counter);
}

unsigned long counter_next(counter_t *counter)
{
    unsigned long next;

    pthread_mutex_lock(&counter->lock);
    if( counter->n >= counter->max )
        counter->n = counter->min;
    next = counter->n++;
    pthread_mutex_unlock(&counter->lock);

    return next;
}

void counter_reset(counter_t *counter)
{
    counter_ctl(counter, COUNTER_CTL_MIN, counter->min);
}

void counter_ctl(counter_t *counter, int opt, unsigned long value)
{
    pthread_mutex_lock(&counter->lock);
    switch( opt ) {
    case COUNTER_CTL_MIN:
        counter->min = value;
        break;

    case COUNTER_CTL_MAX:
        counter->max = value;
        break;

    case COUNTER_CTL_CURRENT:
        counter->n = value;
        break;
    }
    pthread_mutex_unlock(&counter->lock);
}

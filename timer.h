
#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/time.h>

typedef struct ETimer etimer_t;
typedef struct ETimerEvent * etimer_ref_t;
typedef void (*etimer_cb_t)(etimer_t *, etimer_ref_t, void *, int);

#define ETIMER_REF_NULL NULL

etimer_t * etimer_create();
void etimer_destroy(etimer_t *timer);
int etimer_empty(etimer_t *timer);
int etimer_run(etimer_t *timer);
void etimer_stop(etimer_t *timer);
etimer_ref_t etimer_after(etimer_t *timer, long sec, etimer_cb_t cb, void *ud);
etimer_ref_t etimer_mafter(etimer_t *timer, long msec, etimer_cb_t cb, void *ud);
etimer_ref_t etimer_deadline(etimer_t *timer, struct timeval *deadline, etimer_cb_t cb, void *ud);
etimer_ref_t etimer_cancel(etimer_t *timer, etimer_ref_t ref);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TIMER_H */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

#include "timer.h"
#include "list.h"
#include "heap.h"

// TODO It's better to implement timeout handling via
//   timerfd_create, timerfd_settime, timerfd_gettime or
//   timer_create, timer_settime, timer_gettime.

struct ETimerEvent {
    struct timeval deadline;
    etimer_cb_t cb;
    void *userdata;
};

typedef struct ETimerEvent etimer_event_t;

static etimer_event_t * etimer_event_create(struct timeval *deadline, etimer_cb_t cb, void *userdata);
//static void * etimer_event_constructor(void *ptr);
static void etimer_event_destructor(void *ptr);
static int etimer_event_cmp(const void *a, const void *b);
static int etimer_event_match_ref(const void *item, const void *pattern);

struct ETimer {
    int epfd;
    int pin;
    int pout;
    pthread_mutex_t lock;
    heap_t *queue;
    int run;
};

static int etimer_calc_timeout(etimer_t *timer);
static int etimer_clean_signals(etimer_t *timer);
static list_t * etimer_extract_ready(etimer_t *timer);
static void etimer_events_do(list_t *events, etimer_t *timer, int canceled);
static etimer_ref_t etimer_add(etimer_t *timer, etimer_event_t *new_event);
static int etimer_signal(etimer_t *timer);
static int etimer_lock(etimer_t *timer);
static int etimer_unlock(etimer_t *timer);

etimer_t * etimer_create()
{
    etimer_t *timer;
    int pipefd[2], flags;
    struct epoll_event ev;

    timer = malloc(sizeof(etimer_t));
    memset(timer, 0, sizeof(*timer));

    timer->epfd = epoll_create(10);
    pthread_mutex_init(&timer->lock, NULL);

    pipe(pipefd);
    flags = fcntl(pipefd[0], F_GETFL, 0);
    fcntl(pipefd[0], F_SETFL, flags|O_NONBLOCK);
    timer->pin = pipefd[0];
    timer->pout = pipefd[1];

    memset(&ev, 0, sizeof(ev));
    ev.data.fd = timer->pin;
    ev.events = EPOLLIN|EPOLLET;
    epoll_ctl(timer->epfd, EPOLL_CTL_ADD, timer->pin, &ev);

    timer->queue = heap_create(etimer_event_cmp);

    return timer;
}

void etimer_destroy(etimer_t *timer)
{
    list_t *queue;
    etimer_event_t *event;

    etimer_lock(timer);

    queue = list_create();
    while( (event = heap_shift(timer->queue)) ) {
        list_append(queue, event);
    }

    heap_destroy(timer->queue, NULL);
    close(timer->pin);
    close(timer->pout);
    close(timer->epfd);

    etimer_unlock(timer);

    pthread_mutex_destroy(&timer->lock);
    free(timer);

    etimer_events_do(queue, NULL, 1);
    list_destroy(queue, etimer_event_destructor);
}

int etimer_empty(etimer_t *timer)
{
    int empty;

    etimer_lock(timer);
    empty = heap_size(timer->queue) == 0;
    etimer_unlock(timer);

    return empty;
}

int etimer_run(etimer_t *timer)
{
    struct epoll_event events[1];
    int n, i, timeout, run;
    list_t *ready;

    etimer_lock(timer);
    timer->run = 1;
    run = timer->run;
    etimer_unlock(timer);
    while( run ) {
        etimer_lock(timer);
        timeout = etimer_calc_timeout(timer);
        if( timeout != 0 ) {
            etimer_unlock(timer);

            n = epoll_wait(timer->epfd, events, 1, timeout);
            etimer_lock(timer);
            for( i = 0 ; i < n ; i++ ) {
                if( events[i].data.fd == timer->pin ) {
                    etimer_clean_signals(timer);
                }
            }
        }

        run = timer->run;
        if( run )
            ready = etimer_extract_ready(timer);
        else
            ready = NULL;
        etimer_unlock(timer);

        if( ready ) {
            etimer_events_do(ready, timer, 0);
            list_destroy(ready, etimer_event_destructor);
        }
    }

    return 0;
}

void etimer_stop(etimer_t *timer)
{
    etimer_lock(timer);
    timer->run = 0;
    etimer_signal(timer);
    etimer_unlock(timer);
}

static int etimer_calc_timeout(etimer_t *timer)
{
    etimer_event_t *event;
    struct timeval now, sub;
    int timeout;

    if( !(event = heap_min(timer->queue)) )
        return -1;

    gettimeofday(&now, NULL);
    if( timercmp(&event->deadline, &now, <) )
        return 0;

    timersub(&event->deadline, &now, &sub);
    timeout = sub.tv_sec*1000 + sub.tv_usec/1000 + (sub.tv_usec % 1000 ? 1 : 0);
    if( timeout < 0 )
        timeout = 0;

    return timeout;
}

static int etimer_clean_signals(etimer_t *timer)
{
    int n, total;
    char buf[100];

    total = 0;
    while( 1 ) {
        n = read(timer->pin, buf, 100);
        if( n == -1 && errno == EAGAIN )
            return total;
        total += n;
    }

    return -1;
}

static list_t * etimer_extract_ready(etimer_t *timer)
{
    list_t *ready;
    struct timeval now;
    etimer_event_t *event;

    ready = list_create();
    gettimeofday(&now, NULL);
    while( (event = heap_min(timer->queue)) ) {
        if( timercmp(&event->deadline, &now, <) ) {
            heap_shift(timer->queue);
            list_append(ready, event);
        }
        else {
            return ready;
        }
    }

    return ready;
}

static void etimer_events_do(list_t *events, etimer_t *timer, int canceled)
{
    list_iterator_t *li;
    etimer_event_t *event;

    for( li = list_first(events) ; li != NULL ; li = list_next(li) ) {
        event = list_iterator_get_value(li);
        event->cb(timer, event, event->userdata, canceled);
    }
}

etimer_ref_t etimer_after(etimer_t *timer, long sec, etimer_cb_t cb, void *ud)
{
    struct timeval deadline, tv1, tv2;
    etimer_event_t *event;
    etimer_ref_t ref;

    gettimeofday(&tv1, NULL);
    tv2.tv_sec = sec;
    tv2.tv_usec = 0;
    timeradd(&tv1, &tv2, &deadline);

    event = etimer_event_create(&deadline, cb, ud);
    etimer_lock(timer);
    ref = etimer_add(timer, event);
    etimer_unlock(timer);

    return ref;
}

etimer_ref_t etimer_mafter(etimer_t *timer, long msec, etimer_cb_t cb, void *ud)
{
    struct timeval deadline, tv1, tv2;
    etimer_event_t *event;
    etimer_ref_t ref;

    gettimeofday(&tv1, NULL);
    tv2.tv_sec = 0;
    tv2.tv_usec = msec * 1000;
    timeradd(&tv1, &tv2, &deadline);

    event = etimer_event_create(&deadline, cb, ud);
    etimer_lock(timer);
    ref = etimer_add(timer, event);
    etimer_unlock(timer);

    return ref;
}

etimer_ref_t etimer_deadline(etimer_t *timer, struct timeval *deadline, etimer_cb_t cb, void *ud)
{
    etimer_event_t *event;
    etimer_ref_t ref;

    event = etimer_event_create(deadline, cb, ud);
    etimer_lock(timer);
    ref = etimer_add(timer, event);
    etimer_unlock(timer);

    return ref;
}

static etimer_ref_t etimer_add(etimer_t *timer, etimer_event_t *new_event)
{
    etimer_event_t *min1, *min2;

    min1 = heap_min(timer->queue);
    heap_insert(timer->queue, new_event);
    min2 = heap_min(timer->queue);
    if( !min1 || timercmp(&min2->deadline, &min1->deadline, <) ) {
        etimer_signal(timer);
    }

    return new_event;
}

etimer_ref_t etimer_cancel(etimer_t *timer, etimer_ref_t ref)
{
    etimer_event_t *event;

    etimer_lock(timer);
    event = heap_del_item(timer->queue, etimer_event_match_ref, &ref);
    etimer_unlock(timer);

    if( event ) {
        event->cb(timer, event, event->userdata, 1);
        etimer_event_destructor(event);

        return ref;
    }

    return ETIMER_REF_NULL;
}

static int etimer_signal(etimer_t *timer)
{
    char ch = 0xff;

    return write(timer->pout, &ch, sizeof(ch));
}

static int etimer_lock(etimer_t *timer)
{
    return pthread_mutex_lock(&timer->lock);
}

static int etimer_unlock(etimer_t *timer)
{
    return pthread_mutex_unlock(&timer->lock);
}

static etimer_event_t * etimer_event_create(struct timeval *deadline, etimer_cb_t cb, void *userdata)
{
    etimer_event_t *event;

    event = malloc(sizeof(etimer_event_t));
    event->deadline = *deadline;
    event->cb = cb;
    event->userdata = userdata;

    return event;
}

/*static void * etimer_event_constructor(void *ptr)
{
    etimer_event_t *event, *new_event;

    event = (etimer_event_t *)ptr;
    new_event = malloc(sizeof(etimer_event_t));
    *new_event = *event;

    return new_event;
}*/

static void etimer_event_destructor(void *ptr)
{
    etimer_event_t *event = (etimer_event_t *)ptr;
    free(event);
}

static int etimer_event_cmp(const void *a, const void *b)
{
    etimer_event_t *e1 = (etimer_event_t *)a;
    etimer_event_t *e2 = (etimer_event_t *)b;

    if( timercmp(&e1->deadline, &e2->deadline, <) )
        return -1;
    else if( timercmp(&e1->deadline, &e2->deadline, >) )
        return 1;

    return 0;
}

static int etimer_event_match_ref(const void *item, const void *pattern)
{
    etimer_event_t *event = (etimer_event_t *)item;
    etimer_ref_t *ref = (etimer_ref_t *)pattern;

    return event == *ref;
}

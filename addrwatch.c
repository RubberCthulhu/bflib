
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include "addrwatch.h"
#include "list.h"

struct AddrItem {
    char *addr;
    enum {
        ST_NULL,
        ST_UP,
        ST_DOWN
    } state;
};

struct AddrWatcher {
    int timeout;
    list_t *addrs;
    addr_watcher_cb_t cb;
    void *ud;
    int loop;
};

static struct AddrItem * addr_item_create(const char *addr);
static void addr_item_destructor(void *p);
static int addr_item_match_addr(void *p1, void *p2);

int addrwatch(const char *addr)
{
    int sock, flags, r;
    struct sockaddr_in sa;

    r = -1;
    if( (sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )
        goto FIN1;

    flags = 1;
    if( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(int)) == -1 )
        goto FIN2;

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    if( inet_pton(AF_INET, addr, &sa.sin_addr) != 1 )
        goto FIN2;

    if( bind(sock, (struct sockaddr *)&sa, sizeof(sa)) == 0 )
        r = 1;
    else if( errno == EADDRNOTAVAIL )
        r = 0;
    else
        r = -1;

FIN2:
    close(sock);

FIN1:
    return r;
}

addr_watcher_t * addr_watcher_create(addr_watcher_cb_t cb, void *userdata, int timeout)
{
    addr_watcher_t *watcher;

    watcher = malloc(sizeof(addr_watcher_t));
    memset(watcher, 0, sizeof(*watcher));
    watcher->timeout = timeout;
    watcher->addrs = list_create();
    watcher->cb = cb;
    watcher->ud = userdata;

    return watcher;
}

void addr_watcher_destroy(addr_watcher_t *watcher)
{
    list_destroy(watcher->addrs, addr_item_destructor);
    free(watcher);
}

int addr_watcher_add_addr(addr_watcher_t *watcher, const char *addr)
{
    struct AddrItem *item;

    if( list_find(watcher->addrs, addr_item_match_addr, (void *)addr) == NULL ) {
        item = addr_item_create(addr);
        list_append(watcher->addrs, item);
    }

    return list_size(watcher->addrs);
}

int addr_watcher_del_addr(addr_watcher_t *watcher, const char *addr)
{
    struct AddrItem *item;

    if( (item = list_remove_first_match(watcher->addrs, addr_item_match_addr, (void *)addr)) ) {
        addr_item_destructor(item);
    }

    return list_size(watcher->addrs);
}

int addr_watcher_watch(addr_watcher_t *watcher)
{
    struct AddrItem *item;
    list_iterator_t *li;
    int n = 0, up, state;

    for( li = list_first(watcher->addrs) ; li != NULL ; li = list_next(li) ) {
        item = (struct AddrItem *)list_iter_value(li);

        if( (up = addrwatch(item->addr)) == -1 ) {
            return -1;
        }
        else {
            state = up ? ST_UP : ST_DOWN;
            if( state != item->state ) {
                item->state = state;
                watcher->cb(watcher, item->addr, up, watcher->ud);
                n++;
            }
        }
    }

    return n;
}

int addr_watcher_watchloop(addr_watcher_t *watcher)
{
    int total = 0, n;
    struct timeval now, next, timeout;

    watcher->loop = 1;
    gettimeofday(&next, NULL);
    while( watcher->loop ) {
        gettimeofday(&now, NULL);
        if( timercmp(&now, &next, >) ) {
            if( (n = addr_watcher_watch(watcher)) == -1 ) {
                watcher->loop = 0;
                total = -1;
            }
            else {
                total += n;

                timeout.tv_sec = watcher->timeout / 1000;
                timeout.tv_usec = (watcher->timeout % 1000) * 1000;
                timeradd(&now, &timeout, &next);
            }
        }

        if( watcher->loop ) {
            gettimeofday(&now, NULL);
            if( timercmp(&next, &now, >) ) {
                timersub(&next, &now, &timeout);
                usleep(timeout.tv_sec*1000000 + timeout.tv_usec);
            }
        }
    }

    return total;
}

void addr_watcher_breakloop(addr_watcher_t *watcher)
{
    watcher->loop = 0;
}

static struct AddrItem * addr_item_create(const char *addr)
{
    struct AddrItem *item;

    item = malloc(sizeof(struct AddrItem));
    memset(item, 0, sizeof(*item));
    item->state = ST_NULL;
    item->addr = malloc(strlen(addr)+1);
    strcpy(item->addr, addr);

    return item;
}

static void addr_item_destructor(void *p)
{
    struct AddrItem *item = (struct AddrItem *)p;
    free(item->addr);
    free(item);
}

static int addr_item_match_addr(void *p1, void *p2)
{
    return strcmp(((struct AddrItem *)p1)->addr, (char *)p2) == 0;
}

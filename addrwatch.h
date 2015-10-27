
#ifndef ADDRWATCH_H
#define ADDRWATCH_H

#ifdef __cplusplus
extern "C" {
#endif

int addrwatch(const char *addr);

typedef struct AddrWatcher addr_watcher_t;
typedef void (*addr_watcher_cb_t)(addr_watcher_t *, char *, int, void *);

addr_watcher_t * addr_watcher_create(addr_watcher_cb_t cb, void *userdata, int timeout);
void addr_watcher_destroy(addr_watcher_t *watcher);
int addr_watcher_add_addr(addr_watcher_t *watcher, const char *addr);
int addr_watcher_del_addr(addr_watcher_t *watcher, const char *addr);
int addr_watcher_watch(addr_watcher_t *watcher);
int addr_watcher_watchloop(addr_watcher_t *watcher);
void addr_watcher_breakloop(addr_watcher_t *watcher);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ADDRWATCH_H */

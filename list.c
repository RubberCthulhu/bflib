
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <pthread.h>
#include "list.h"

struct List {
    TAILQ_HEAD(ListHead, ListItem) head;
    pthread_mutex_t lock;
};

struct ListItem {
    void *item;
    list_t *list;
    TAILQ_ENTRY(ListItem) items;
};

#ifndef TAILQ_EMPTY
#define TAILQ_EMPTY(head)               ((head)->tqh_first == NULL)
#endif /* TAILQ_EMPTY */

#ifndef TAILQ_FIRST
#define TAILQ_FIRST(head)               ((head)->tqh_first)
#endif /* TAILQ_FIRST */

#ifndef TAILQ_NEXT
#define TAILQ_NEXT(elm, field)          ((elm)->field.tqe_next)
#endif /* TAILQ_NEXT */

#ifndef TAILQ_LAST
#define TAILQ_LAST(head, headname) \
        (*(((struct headname *)((head)->tqh_last))->tqh_last))
#endif /* TAILQ_LAST */

#ifndef TAILQ_PREV
#define TAILQ_PREV(elm, headname, field) \
        (*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
#endif /* TAILQ_PREV */

/*struct sort_info {
    int (*cmp)(const void *, const void *);
};
static int list_sort_cmp(const void *a, const void *b, void *ud);
*/

static int list_item_match_ptr(void *item, void *ptr);

list_t * list_create()
{
    list_t *list;

    list = (list_t *)malloc(sizeof(list_t));
    memset(list, 0, sizeof(*list));
    TAILQ_INIT(&list->head);
    pthread_mutex_init(&list->lock, NULL);

    return list;
}

void list_destroy(list_t *list, list_destructor_t destructor)
{
    list_erase(list, destructor);
    pthread_mutex_destroy(&list->lock);
    free(list);
}

void list_erase(list_t *list, list_destructor_t destructor)
{
    list_iterator_t *li;

    while( (li = list_first(list)) ) {
        if( destructor ) {
            destructor(li->item);
        }

        TAILQ_REMOVE(&list->head, li, items);
        free(li);
    }
}

long list_size(list_t *list)
{
    list_iterator_t *li;
    long i = 0;

    for( li = list_first(list) ; li != NULL ; li = list_next(li) ) {
        i++;
    }

    return i;
}

int list_empty(list_t *list)
{
    return TAILQ_EMPTY(&list->head);
}

list_t * list_copy(list_t *list, list_constructor_t constructor)
{
    list_t *copy;
    list_iterator_t *li;
    void *item;

    copy = list_create();
    for( li = list_first(list) ; li != NULL ; li = list_next(li) ) {
        item = list_iterator_get_value(li);
        if( constructor ) {
            list_append(copy, constructor(item));
        }
        else {
            list_append(copy, item);
        }
    }

    return copy;
}

list_iterator_t * list_append(list_t *list, void *userdata)
{
    list_iterator_t *li;

    li = (list_iterator_t *)malloc(sizeof(list_iterator_t));
    memset(li, 0, sizeof(*li));
    li->list = list;
    li->item = userdata;
    TAILQ_INSERT_TAIL(&list->head, li, items);

    return li;
}

void list_append_list(list_t *list1, list_t *list2, list_constructor_t constructor)
{
    list_iterator_t *li;
    void *item;

    for( li = list_first(list2) ; li != NULL ; li = list_next(li) ) {
        item = list_iterator_get_value(li);
        if( constructor )
            item = constructor(item);
        list_append(list1, item);
    }
}

list_iterator_t * list_insert_after(list_iterator_t *li, void *userdata)
{
    list_iterator_t *new;

    new = (list_iterator_t *)malloc(sizeof(list_iterator_t));
    memset(new, 0, sizeof(*new));
    new->list = li->list;
    new->item = userdata;
    TAILQ_INSERT_AFTER(&li->list->head, li, new, items);

    return new;
}

list_iterator_t * list_insert_before(list_iterator_t *li, void *userdata)
{
    list_iterator_t *new, *prev;

    if( (prev = list_prev(li)) )
        new = list_insert_after(li, userdata);
    else
        new = list_unshift(li->list, userdata);

    return new;
}

list_iterator_t * list_first(list_t *list)
{
    return TAILQ_FIRST(&list->head);
}

list_iterator_t * list_next(list_iterator_t *li)
{
    return TAILQ_NEXT(li, items);
}

list_iterator_t * list_last(list_t *list)
{
    return TAILQ_LAST(&list->head, ListHead);
}

list_iterator_t *list_prev(list_iterator_t *li)
{
    return TAILQ_PREV(li, ListHead, items);
}

void * list_iterator_get_value(list_iterator_t *li)
{
    return li->item;
}

void * list_iter_value(list_iterator_t *li)
{
    return li->item;
}

int list_is_first(list_t *list, list_iterator_t *li)
{
    return li == list_first(list);
}

list_iterator_t * list_unshift(list_t *list, void *item)
{
    list_iterator_t *li;

    li = (list_iterator_t *)malloc(sizeof(list_iterator_t));
    memset(li, 0, sizeof(*li));
    li->list = list;
    li->item = item;
    TAILQ_INSERT_HEAD(&list->head, li, items);

    return li;
}

void * list_shift(list_t *list)
{
    list_iterator_t *li;
    void *item = NULL;

    if( (li = list_first(list)) ) {
        item = list_iterator_get_value(li);
        list_remove(list, li);
    }

    return item;
}

void * list_find(list_t *list, list_match_pattern_t match, void *pattern)
{
    list_iterator_t *li;

    if( (li = list_find_iterator(list, match, pattern)) ) {
        return li->item;
    }

    return NULL;
}

list_iterator_t * list_find_iterator(list_t *list, list_match_pattern_t match, void *pattern)
{
    list_iterator_t *li;

    for( li = list_first(list) ; li != NULL ; li = list_next(li) ) {
        if( match(li->item, pattern) ) {
            return li;
        }
    }

    return NULL;
}

void * list_remove(list_t *list, list_iterator_t *li)
{
    void *item;

    item = li->item;
    TAILQ_REMOVE(&list->head, li, items);
    free(li);

    return item;
}

static int list_item_match_ptr(void *item, void *ptr)
{
    return item == ptr;
}

void * list_remove_ptr(list_t *list, void *ptr)
{
    return list_remove_first_match(list, list_item_match_ptr, ptr);
}

void * list_remove_first_match(list_t *list, list_match_pattern_t match, void *pattern)
{
    list_iterator_t *li;

    if( (li = list_find_iterator(list, match, pattern)) ) {
        return list_remove(list, li);
    }

    return NULL;
}

list_t * list_filter(list_t *list, list_match_pattern_t match, void *pattern)
{
    list_t *filtered;
    list_iterator_t *li;
    void *item;

    filtered = list_create();
    for( li = list_first(list) ; li != NULL ; li = list_next(li) ) {
        item = list_iterator_get_value(li);
        if( match(item, pattern) )
            list_append(filtered, item);
    }

    return filtered;
}

void list_sort(list_t *list, int (*cmp)(const void *, const void *))
{
    list_iterator_t *li;
    void **arr;
    long n, i;
    //struct sort_info sort_info;

    n = list_size(list);
    if( n == 0 ) {
        return;
    }

    arr = malloc(sizeof(void *) * n);
    i = 0;
    for( li = list_first(list) ; li != NULL ; li = list_next(li) ) {
        arr[i++] = list_iterator_get_value(li);
    }

    //sort_info.cmp = cmp;
    //qsort_r(arr, n, sizeof(void *), list_sort_cmp, &sort_info);
    qsort(arr, n, sizeof(void *), cmp);
    list_erase(list, 0);
    for( i = 0 ; i < n ; i++ ) {
        list_append(list, arr[i]);
    }

    free(arr);
}

/*static int list_sort_cmp(const void *a, const void *b, void *ud)
{
    const void **pa = (const void **)a;
    const void **pb = (const void **)b;
    struct sort_info *sort_info = (struct sort_info *)ud;

    return sort_info->cmp(*pa, *pb);
}*/

int list_lock(list_t *list)
{
    return pthread_mutex_lock(&list->lock);
}

int list_unlock(list_t *list)
{
    return pthread_mutex_unlock(&list->lock);
}

list_t * list_map(list_t *list, list_map_fun_t fun)
{
    list_t *map;
    list_iterator_t *li;

    map = list_create();
    for( li = list_first(list) ; li != NULL ; li = list_next(li) ) {
        list_append(map, fun(list_iterator_get_value(li)));
    }

    return map;
}

void list_foreach(list_t *list, list_foreach_fun_t fun)
{
    list_iterator_t *li;

    for( li = list_first(list) ; li != NULL ; li = list_next(li) ) {
        fun(list_iterator_get_value(li));
    }
}

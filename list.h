
#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct List list_t;

typedef struct ListItem list_iterator_t;

typedef void list_destructor_t(void *item);
typedef void * list_constructor_t(void *item);

list_t * list_create();
void list_destroy(list_t *list, list_destructor_t destructor);
void list_erase(list_t *list, list_destructor_t destructor);
long list_size(list_t *list);
int list_empty(list_t *list);
list_t * list_copy(list_t *list, list_constructor_t constructor);
list_iterator_t * list_append(list_t *list, void *userdata);
void list_append_list(list_t *list1, list_t *list2, list_constructor_t constructor);
list_iterator_t * list_insert_after(list_iterator_t *li, void *userdata);
list_iterator_t * list_insert_before(list_iterator_t *li, void *userdata);
list_iterator_t * list_first(list_t *list);
list_iterator_t * list_next(list_iterator_t *li);
list_iterator_t * list_last(list_t *list);
list_iterator_t * list_prev(list_iterator_t *li);
void * list_iterator_get_value(list_iterator_t *li);
void * list_iter_value(list_iterator_t *li);
int list_is_first(list_t *list, list_iterator_t *li);

list_iterator_t * list_unshift(list_t *list, void *item);
void * list_shift(list_t *list);

typedef int list_match_pattern_t(void *item, void *pattern);

void * list_find(list_t *list, list_match_pattern_t match, void *pattern);
list_iterator_t * list_find_iterator(list_t *list, list_match_pattern_t match, void *pattern);
void * list_remove(list_t *list, list_iterator_t *li);
void * list_remove_ptr(list_t *list, void *ptr);
void * list_remove_first_match(list_t *list, list_match_pattern_t match, void *pattern);

list_t * list_filter(list_t *list, list_match_pattern_t match, void *pattern);

void list_sort(list_t *list, int (*cmp)(const void *, const void *));

int list_lock(list_t *list);
int list_unlock(list_t *list);

typedef void * (*list_map_fun_t)(void *);
list_t * list_map(list_t *list, list_map_fun_t fun);

typedef void (*list_foreach_fun_t)(void *);
void list_foreach(list_t *list, list_foreach_fun_t fun);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIST_H */

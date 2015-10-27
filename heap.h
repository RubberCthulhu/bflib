
#ifndef HEAP_H
#define HEAP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Heap heap_t;

typedef int (*heap_cmp_t)(const void *, const void *);

heap_t * heap_create(heap_cmp_t cmp);
heap_t * heap_build(heap_cmp_t cmp, void **array, long n);
void heap_destroy(heap_t *heap, void (*destructor)(void *));
long heap_size(heap_t *heap);
long heap_insert(heap_t *heap, void *item);
void * heap_min(heap_t *heap);
void * heap_shift(heap_t *heap);
void * heap_del_item(heap_t *heap, int (*match)(const void *, const void *), const void *pattern);

typedef struct HeapInfo * heap_info_t;
typedef heap_info_t (*heap_info_accessor_t)(const void *);
int heap_set_info_accessor(heap_t *heap, heap_info_accessor_t fun);
void heap_info_init(heap_info_t *info);
void heap_info_destroy(heap_info_t *info);

void * heap_hipify(heap_t *heap, void *item);
void * heap_del(heap_t *heap, void *item);

int heap_check_properties(heap_t *heap);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* HEAP_H */

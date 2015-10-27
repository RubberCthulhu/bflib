
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "heap.h"

struct Heap {
    void **heap;
    long heap_size;
    long size;
    heap_cmp_t cmp;
    heap_info_accessor_t get_info;
};

#define HEAP_BLOCK_SIZE      128
#define HEAP_PARENT_POS(pos) (((pos) - 1) / 2)
#define HEAP_LEFT_POS(pos)   ((pos) * 2 + 1)
#define HEAP_RIGHT_POS(pos)  ((pos) * 2 + 2)

static int heap_up(heap_t *heap, long pos);
static int heap_down(heap_t *heap, long pos);
static long heap_hipify_pos(heap_t *heap, long pos);
static long heap_build_heap(heap_t *heap);
static long heap_resize(heap_t *heap, long size);

struct HeapInfo {
    long pos;
};

static void * heap_move(heap_t *heap, long pos, void *item);
static void * heap_del_pos(heap_t *heap, long pos);

heap_t * heap_create(heap_cmp_t cmp)
{
    heap_t *heap;

    heap = malloc(sizeof(heap_t));
    memset(heap, 0, sizeof(*heap));
    heap->heap = malloc(sizeof(void *) * HEAP_BLOCK_SIZE);
    heap->heap_size = HEAP_BLOCK_SIZE;
    heap->cmp = cmp;

    return heap;
}

heap_t * heap_build(heap_cmp_t cmp, void **array, long n)
{
    heap_t *heap;

    heap = heap_create(cmp);
    heap_resize(heap, n);
    memcpy(heap->heap, array, sizeof(void *)*n);
    heap->size = n;
    heap_build_heap(heap);

    return heap;
}

void heap_destroy(heap_t *heap, void (*destructor)(void *))
{
    long i;

    if( destructor ) {
        for( i = 0 ; i < heap->size ; i++ ) {
            destructor(heap->heap[i]);
        }
    }

    free(heap->heap);
    free(heap);
}

long heap_size(heap_t *heap)
{
    return heap->size;
}

long heap_insert(heap_t *heap, void *item)
{
    if( heap->heap_size < heap->size+1 )
        heap_resize(heap, heap->size+1);

    heap_move(heap, heap->size++, item);
    heap_up(heap, heap->size-1);

    return heap->size;
}

void * heap_shift(heap_t *heap)
{
    void *item = NULL;

    if( heap->size > 0 ) {
        item = heap->heap[0];
        heap_move(heap, 0, heap->heap[--heap->size]);
        heap_down(heap, 0);

        if( heap->heap_size - heap->size > HEAP_BLOCK_SIZE )
            heap_resize(heap, heap->size);
    }

    return item;
}

void * heap_min(heap_t *heap)
{
    if( heap->size > 0 )
        return heap->heap[0];

    return NULL;
}

void * heap_del_item(heap_t *heap, int (*match)(const void *, const void *), const void *pattern)
{
    long i;

    for( i = 0 ; i < heap->size ; i++ ) {
        if( match(heap->heap[i], pattern) ) {
            return heap_del_pos(heap, i);
        }
    }

    return NULL;
}

static int heap_up(heap_t *heap, long pos)
{
    int stop = 0, n = 0;
    long parent;
    void *tmp;

    while( pos > 0 && !stop ) {
        parent = HEAP_PARENT_POS(pos);
        if( heap->cmp(heap->heap[pos], heap->heap[parent]) < 0 ) {
            tmp = heap->heap[parent];
            heap_move(heap, parent, heap->heap[pos]);
            heap_move(heap, pos, tmp);
            pos = parent;
            n++;
        }
        else {
            stop = 1;
        }
    }

    return n;
}

static int heap_down(heap_t *heap, long pos)
{
    long left, right, down;
    int stop = 0, n = 0;
    void *tmp;

    while( !stop ) {
        left = HEAP_LEFT_POS(pos);
        right = HEAP_RIGHT_POS(pos);
        down = right < heap->size
            ? (heap->cmp(heap->heap[left], heap->heap[right]) < 0 ? left : right)
            : left;
        if( down < heap->size ) {
            if( heap->cmp(heap->heap[down], heap->heap[pos]) < 0 ) {
                tmp = heap->heap[down];
                heap_move(heap, down, heap->heap[pos]);
                heap_move(heap, pos, tmp);
                pos = down;
                n++;
            }
            else {
                stop = 1;
            }
        }
        else {
            stop = 1;
        }
    }

    return n;
}

static long heap_hipify_pos(heap_t *heap, long pos)
{
    long n;

    if( (n = heap_up(heap, pos)) == 0 )
        n = heap_down(heap, pos);

    return n;
}

static long heap_build_heap(heap_t *heap)
{
    long i, n = 0;

    if( heap->size > 1 ) {
        for( i = heap->size/2 - 1 ; i >= 0 ; i-- ) {
            n += heap_down(heap, i);
        }
    }

    return n;
}

static long heap_resize(heap_t *heap, long size)
{
    long heap_size;

    heap_size = size + HEAP_BLOCK_SIZE - (size % HEAP_BLOCK_SIZE);
    if( heap->heap_size != heap_size ) {
        heap->heap = realloc(heap->heap, sizeof(void *)*heap_size);
        heap->heap_size = heap_size;
    }

    return heap->heap_size;
}

static void * heap_move(heap_t *heap, long pos, void *item)
{
    heap_info_t info;

    if( heap->get_info ) {
        info = heap->get_info(item);
        info->pos = pos;
    }

    heap->heap[pos] = item;

    return item;
}

int heap_set_info_accessor(heap_t *heap, heap_info_accessor_t fun)
{
    long i;
    heap_info_t info;

    heap->get_info = fun;
    if( heap->get_info ) {
        for( i = 0 ; i < heap->size ; i++ ) {
            info = heap->get_info(heap->heap[i]);
            info->pos = i;
        }
    }

    return 0;
}

void heap_info_init(heap_info_t *info)
{
    *info = malloc(sizeof(struct HeapInfo));
    memset(*info, 0, sizeof(struct HeapInfo));
}

void heap_info_destroy(heap_info_t *info)
{
    free(*info);
}

void * heap_hipify(heap_t *heap, void *item)
{
    heap_info_t info;

    if( heap->get_info ) {
        info = heap->get_info(item);
        heap_hipify_pos(heap, info->pos);

        return item;
    }

    return NULL;
}

void * heap_del(heap_t *heap, void *item)
{
    heap_info_t info;

    if( heap->get_info ) {
        info = heap->get_info(item);

        return heap_del_pos(heap, info->pos);
    }

    return NULL;
}

static void * heap_del_pos(heap_t *heap, long pos)
{
    void *item = NULL;

    if( pos >= 0 && pos < heap->size ) {
        item = heap->heap[pos];
        if( pos == heap->size-1 ) {
            heap->size--;
        }
        else {
            heap_move(heap, pos, heap->heap[--heap->size]);
            heap_hipify_pos(heap, pos);
        }

        if( heap->heap_size - heap->size > HEAP_BLOCK_SIZE )
            heap_resize(heap, heap->size);
    }

    return item;
}

int heap_check_properties(heap_t *heap)
{
    heap_info_t info;
    long parent, left, right;
    long i;

    for( i = 0 ; i < heap->size ; i++ ) {
        if( heap->get_info ) {
            info = heap->get_info(heap->heap[i]);
            if( info->pos != i )
                return 0;
        }

        parent = HEAP_PARENT_POS(i);
        if( parent > 0 && !(heap->cmp(heap->heap[parent], heap->heap[i]) <= 0) )
            return 0;

        left = HEAP_LEFT_POS(i);
        if( left < heap->size && !(heap->cmp(heap->heap[left], heap->heap[i]) >= 0) )
            return 0;

        right = HEAP_RIGHT_POS(i);
        if( right < heap->size && !(heap->cmp(heap->heap[right], heap->heap[i]) >= 0) )
            return 0;
    }

    return 1;
}

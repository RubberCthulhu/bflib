
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <check.h>

#include "heap.h"
#include "config.h"

heap_t *heap = NULL;

struct HeapItem {
    int value;
    heap_info_t heap_info;
};

#define RANDOM_ARRAY_SIZE 1000
struct HeapItem random_array[RANDOM_ARRAY_SIZE];

int cmp_int(const void *a, const void *b)
{
    if( *((int *)a) < *((int *)b) )
        return -1;
    else if( *((int *)a) > *((int *)b) )
        return 1;

    return 0;
}

int match_int(const void *a, const void *b)
{
    return cmp_int(a, b) == 0;
}

void init_testcase(void)
{
    heap = heap_create(cmp_int);
}

void end_testcase(void)
{
    heap_destroy(heap, NULL);
    heap = NULL;
}

heap_info_t heap_item_get_info(const void *p)
{
    return ((struct HeapItem *)p)->heap_info;
}

int cmp_heap_item(const void *a, const void *b)
{
    return cmp_int(&((struct HeapItem *)a)->value, &((struct HeapItem *)b)->value);
}

void init_testcase_random_data_with_info(void)
{
    unsigned int seed = time(NULL);
    long i;

    heap = heap_create(cmp_heap_item);
    heap_set_info_accessor(heap, heap_item_get_info);

    memset(random_array, 0, sizeof(struct HeapItem)*RANDOM_ARRAY_SIZE);
    srandom(seed);
    for( i = 0 ; i < RANDOM_ARRAY_SIZE ; i++ ) {
        heap_info_init(&random_array[i].heap_info);
        random_array[i].value = random();
        heap_insert(heap, &random_array[i]);
    }
}

void end_testcase_random_data_with_info(void)
{
    int i;

    for( i = 0 ; i < RANDOM_ARRAY_SIZE ; i++ ) {
        heap_info_destroy(&random_array[i].heap_info);
    }

    heap_destroy(heap, NULL);
    heap = NULL;
}

START_TEST(test_heap_create)
{
    heap_t *heap = heap_create(cmp_int);
    ck_assert_ptr_ne(heap, NULL);
    ck_assert_int_eq(heap_size(heap), 0);
    heap_destroy(heap, NULL);
}
END_TEST

START_TEST(test_heap_basics)
{
    int seven = 7, one = 1, three = 3;
    ck_assert_int_eq(heap_size(heap), 0);

    ck_assert_int_eq(heap_insert(heap, &seven), 1);
    ck_assert_int_eq(heap_size(heap), 1);
    ck_assert_int_eq(*(int *)heap_min(heap), seven);

    ck_assert_int_eq(heap_insert(heap, &one), 2);
    ck_assert_int_eq(heap_size(heap), 2);
    ck_assert_int_eq(*(int *)heap_min(heap), one);

    ck_assert_int_eq(heap_insert(heap, &three), 3);
    ck_assert_int_eq(heap_size(heap), 3);
    ck_assert_int_eq(*(int *)heap_min(heap), one);

    ck_assert_int_eq(*(int *)heap_shift(heap), one);
    ck_assert_int_eq(heap_size(heap), 2);

    ck_assert_int_eq(*(int *)heap_shift(heap), three);
    ck_assert_int_eq(heap_size(heap), 1);

    ck_assert_int_eq(*(int *)heap_shift(heap), seven);
    ck_assert_int_eq(heap_size(heap), 0);
}
END_TEST

START_TEST(test_heap_empty)
{
    ck_assert_int_eq(heap_size(heap), 0);
    ck_assert_ptr_eq(heap_min(heap), NULL);
    ck_assert_ptr_eq(heap_shift(heap), NULL);
}
END_TEST

START_TEST(test_heap_random)
{
    unsigned int seed = time(NULL);
    int array[10], sorted[10], i;

    srandom(seed);
    for( i = 0 ; i < 10 ; i++ ) {
        array[i] = random();
    }

    for( i = 0 ; i < 10 ; i++ ) {
        heap_insert(heap, &array[i]);
    }

    memcpy(sorted, array, sizeof(int)*10);
    qsort(sorted, 10, sizeof(int), cmp_int);

    for( i = 0 ; i < 10 ; i++ ) {
        ck_assert_int_eq(heap_size(heap), 10-i);
        ck_assert_int_eq(*(int *)heap_shift(heap), sorted[i]);
    }
}
END_TEST

START_TEST(test_heap_del_item)
{
    unsigned int seed = time(NULL);
    int array[100], sorted[100], item, i;

    srandom(seed);
    item = random();
    for( i = 0 ; i < 10 ; i++ ) {
        array[i] = random();
    }

    heap_insert(heap, &item);
    for( i = 0 ; i < 10 ; i++ ) {
        heap_insert(heap, &array[i]);
    }

    memcpy(sorted, array, sizeof(int)*10);
    qsort(sorted, 10, sizeof(int), cmp_int);

    ck_assert_int_eq(*(int *)heap_del_item(heap, match_int, &item), item);

    for( i = 0 ; i < 10 ; i++ ) {
        ck_assert_int_eq(heap_size(heap), 10-i);
        ck_assert_int_eq(*(int *)heap_shift(heap), sorted[i]);
    }
}
END_TEST

START_TEST(test_heap_build)
{
    heap_t *heap;
    unsigned int seed = time(NULL);
    int array[10], sorted[10], i;
    void *array_ptr[10];

    srandom(seed);
    for( i = 0 ; i < 10 ; i++ ) {
        array[i] = random();
        array_ptr[i] = &array[i];
    }

    heap = heap_build(cmp_int, array_ptr, 10);
    ck_assert_ptr_ne(heap, NULL);

    memcpy(sorted, array, sizeof(int)*10);
    qsort(sorted, 10, sizeof(int), cmp_int);

    for( i = 0 ; i < 10 ; i++ ) {
        ck_assert_int_eq(heap_size(heap), 10-i);
        ck_assert_int_eq(*(int *)heap_shift(heap), sorted[i]);
    }

    heap_destroy(heap, NULL);
}
END_TEST

START_TEST(test_heap_check_properties)
{
    ck_assert_int_gt(heap_check_properties(heap), 0);
}
END_TEST

START_TEST(test_heap_hipify)
{
    int i;
    unsigned int seed = time(NULL);

    srandom(seed);
    for( i = 0 ; i < RANDOM_ARRAY_SIZE ; i++ ) {
        random_array[i].value = random();
        ck_assert_ptr_eq(heap_hipify(heap, &random_array[i]), &random_array[i]);
        ck_assert_int_gt(heap_check_properties(heap), 0);
    }
}
END_TEST

START_TEST(test_heap_del)
{
    int i;

    for( i = 0 ; i < RANDOM_ARRAY_SIZE ; i++ ) {
        ck_assert_ptr_eq(heap_del(heap, &random_array[i]), &random_array[i]);
        ck_assert_int_eq(heap_size(heap), RANDOM_ARRAY_SIZE-i-1);
        ck_assert_int_gt(heap_check_properties(heap), 0);
    }
}
END_TEST

Suite * heap_suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("Binary Heap");

    tc = tcase_create("Heap create");
    tcase_add_test(tc, test_heap_create);
    suite_add_tcase(s, tc);

    tc = tcase_create("Heap operations");
    tcase_add_checked_fixture(tc, init_testcase, end_testcase);
    tcase_add_test(tc, test_heap_basics);
    tcase_add_test(tc, test_heap_empty);
    tcase_add_test(tc, test_heap_random);
    tcase_add_test(tc, test_heap_del_item);
    suite_add_tcase(s, tc);

    tc = tcase_create("Heap build");
    tcase_add_test(tc, test_heap_build);
    suite_add_tcase(s, tc);

    tc = tcase_create("Heap info");
    tcase_add_checked_fixture(tc,
        init_testcase_random_data_with_info,
        end_testcase_random_data_with_info);
    tcase_add_test(tc, test_heap_check_properties);
    tcase_add_test(tc, test_heap_hipify);
    tcase_add_test(tc, test_heap_del);
    suite_add_tcase(s, tc);

    return s;
}

int main(int argc, char **argv)
{
    int failed;
    Suite *s;
    SRunner *sr;

    s = heap_suite();
    sr = srunner_create(s);
#ifdef CHECK_MODE_NOFORK
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

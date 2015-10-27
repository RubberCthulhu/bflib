
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <check.h>

#include "tree.h"
#include "config.h"

tree_t *tree = NULL;

#define RANDOM_ARRAY_SIZE 1000
int random_array[RANDOM_ARRAY_SIZE];

int cmp_int_lt(const void *a, const void *b)
{
    if( *((int *)a) < *((int *)b) )
        return -1;
    else if( *((int *)a) > *((int *)b) )
        return 1;

    return 0;
}

int cmp_int_gt(const void *a, const void *b)
{
    return cmp_int_lt(a, b) * (-1);
}

int cmp_int(const void *a, const void *b)
{
    return cmp_int_lt(a, b);
}

void init_testcase(void)
{
    tree = tree_create(cmp_int);
}

void end_testcase(void)
{
    tree_destroy(tree, NULL);
    tree = NULL;
}

void init_testcase_random_data(void)
{
    unsigned int seed = time(NULL);
    int i;

    tree = tree_create(cmp_int);
    srandom(seed);
    for( i = 0 ; i < RANDOM_ARRAY_SIZE ; i++ ) {
        // Make sure the new element is unique.
        do {
            random_array[i] = random();
        }
        while( tree_find(tree, &random_array[i]) );
        tree_insert(tree, &random_array[i], &random_array[i]);
    }
}

void end_testcase_random_data(void)
{
    tree_destroy(tree, NULL);
    tree = NULL;
}

void * test_fold_cb(void *key, void *value, void *acc)
{
    int *arr = (int *)acc;

    arr[++arr[0]] = *(int *)value;
    return arr;
}

START_TEST(test_tree_create)
{
    tree_t *tree = tree_create(cmp_int);
    ck_assert_ptr_ne(tree, NULL);
    ck_assert_int_eq(tree_size(tree), 0);
    tree_destroy(tree, NULL);
}
END_TEST

START_TEST(test_tree_basics)
{
    int seven = 7, one = 1, three = 3;

    ck_assert_int_eq(tree_size(tree), 0);

    ck_assert_int_eq(*(int *)tree_insert(tree, &seven, &seven), seven);
    ck_assert_int_eq(tree_size(tree), 1);

    ck_assert_int_eq(*(int *)tree_insert(tree, &one, &one), one);
    ck_assert_int_eq(tree_size(tree), 2);

    ck_assert_int_eq(*(int *)tree_insert(tree, &three, &three), three);
    ck_assert_int_eq(tree_size(tree), 3);

    ck_assert_int_eq(*(int *)tree_find(tree, &seven), seven);
    ck_assert_int_eq(*(int *)tree_find(tree, &one), one);
    ck_assert_int_eq(*(int *)tree_find(tree, &three), three);

    ck_assert_int_eq(*(int *)tree_delete(tree, &seven), seven);
    ck_assert_int_eq(tree_size(tree), 2);

    ck_assert_int_eq(*(int *)tree_delete(tree, &one), one);
    ck_assert_int_eq(tree_size(tree), 1);

    ck_assert_int_eq(*(int *)tree_delete(tree, &three), three);
    ck_assert_int_eq(tree_size(tree), 0);
}
END_TEST

START_TEST(test_tree_foldl)
{
    int sorted[RANDOM_ARRAY_SIZE], acc[RANDOM_ARRAY_SIZE+1];
    int i;

    memcpy(sorted, random_array, sizeof(sorted));
    qsort(sorted, RANDOM_ARRAY_SIZE, sizeof(int), cmp_int_lt);
    memset(acc, 0, sizeof(acc));

    ck_assert_ptr_eq(tree_foldl(tree, test_fold_cb, acc), acc);
    ck_assert_int_eq(acc[0], RANDOM_ARRAY_SIZE);
    for( i = 0 ; i < RANDOM_ARRAY_SIZE ; i++ ) {
        ck_assert_int_eq(acc[i+1], sorted[i]);
    }
}
END_TEST

START_TEST(test_tree_foldr)
{
    int sorted[RANDOM_ARRAY_SIZE], acc[RANDOM_ARRAY_SIZE+1];
    int i;

    memcpy(sorted, random_array, sizeof(sorted));
    qsort(sorted, RANDOM_ARRAY_SIZE, sizeof(int), cmp_int_gt);
    memset(acc, 0, sizeof(acc));

    ck_assert_ptr_eq(tree_foldr(tree, test_fold_cb, acc), acc);
    ck_assert_int_eq(acc[0], RANDOM_ARRAY_SIZE);
    for( i = 0 ; i < RANDOM_ARRAY_SIZE ; i++ ) {
        ck_assert_int_eq(acc[i+1], sorted[i]);
    }
}
END_TEST

START_TEST(test_tree_properties)
{
    tree_info_t info;

    ck_assert_ptr_eq(tree_info(tree, &info), &info);
    ck_assert_int_eq(info.size, tree_size(tree));
    ck_assert_int_le(info.height, info.min_height*2);
}
END_TEST

START_TEST(test_tree_integrity)
{
    ck_assert_int_gt(tree_check_integrity(tree), 0);
}
END_TEST

Suite * tree_suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("Red-black Tree");

    tc = tcase_create("Tree create");
    tcase_add_test(tc, test_tree_create);
    suite_add_tcase(s, tc);

    tc = tcase_create("Tree basics");
    tcase_add_checked_fixture(tc, init_testcase, end_testcase);
    tcase_add_test(tc, test_tree_basics);
    suite_add_tcase(s, tc);

    tc = tcase_create("Tree fold");
    tcase_add_checked_fixture(tc, init_testcase_random_data, end_testcase_random_data);
    tcase_add_test(tc, test_tree_foldl);
    tcase_add_test(tc, test_tree_foldr);
    suite_add_tcase(s, tc);

    tc = tcase_create("Tree properties");
    tcase_add_checked_fixture(tc, init_testcase_random_data, end_testcase_random_data);
    tcase_add_test(tc, test_tree_properties);
    tcase_add_test(tc, test_tree_integrity);
    suite_add_tcase(s, tc);

    return s;
}

int main(int argc, char **argv)
{
    int failed;
    Suite *s;
    SRunner *sr;

    s = tree_suite();
    sr = srunner_create(s);
#ifdef CHECK_MODE_NOFORK
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


#include <stdio.h>
#include <stdlib.h>
#include <check.h>

#include "addrwatch.h"
#include "config.h"

#define WATCHER_TIMEOUT 3000

void watcher_cb(addr_watcher_t *w, char *addr, int up, void *ud)
{
    int *p = (int *)ud;
    *p = up;
    addr_watcher_breakloop(w);
}

START_TEST(test_addrwatch)
{
    ck_assert_int_gt(addrwatch("127.0.0.1"), 0);
    ck_assert_int_eq(addrwatch("128.0.0.1"), 0);
    ck_assert_int_lt(addrwatch("qwerty"), 0);
}
END_TEST

START_TEST(test_watcher_init)
{
    addr_watcher_t *w;

    w = addr_watcher_create(watcher_cb, NULL, WATCHER_TIMEOUT);
    ck_assert_ptr_ne(w, NULL);

    ck_assert_int_eq(addr_watcher_add_addr(w, "127.0.0.1"), 1);
    ck_assert_int_eq(addr_watcher_del_addr(w, "127.0.0.1"), 0);

    addr_watcher_destroy(w);
}
END_TEST

START_TEST(test_watch)
{
    int up = -1;
    addr_watcher_t *w = addr_watcher_create(watcher_cb, &up, WATCHER_TIMEOUT);

    addr_watcher_add_addr(w, "127.0.0.1");
    addr_watcher_watch(w);

    ck_assert_int_gt(up, 0);

    addr_watcher_destroy(w);
}
END_TEST

START_TEST(test_watchloop)
{
    int up = -1;
    addr_watcher_t *w = addr_watcher_create(watcher_cb, &up, WATCHER_TIMEOUT);

    addr_watcher_add_addr(w, "127.0.0.1");
    addr_watcher_watchloop(w);

    ck_assert_int_gt(up, 0);

    addr_watcher_destroy(w);
}
END_TEST

Suite * addrwatch_suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("Bind Watcher");

    tc = tcase_create("Check Addr");
    tcase_add_test(tc, test_addrwatch);
    suite_add_tcase(s, tc);

    tc = tcase_create("Watcher");
    tcase_add_test(tc, test_watcher_init);
    suite_add_tcase(s, tc);

    tc = tcase_create("Single watch");
    tcase_set_timeout(tc, WATCHER_TIMEOUT/1000 * 2);
    tcase_add_test(tc, test_watch);
    suite_add_tcase(s, tc);

    tc = tcase_create("Loop watch");
    tcase_set_timeout(tc, WATCHER_TIMEOUT/1000 * 2);
    tcase_add_test(tc, test_watchloop);
    suite_add_tcase(s, tc);

    return s;
}

int main(int argc, char **argv)
{
    int failed;
    Suite *s;
    SRunner *sr;

    s = addrwatch_suite();
    sr = srunner_create(s);
#ifdef CHECK_MODE_NOFORK
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

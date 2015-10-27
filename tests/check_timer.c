
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <check.h>

#include "timer.h"
#include "config.h"

struct timer_event {
    int cancelled;
    struct timeval deadline;
    struct timeval finish;
};

etimer_t *timer = NULL;
int events_number = 0;
int events_counter = 0;

void timer_cb_dummy(etimer_t *timer, etimer_ref_t ref, void *ud, int cancelled)
{

}

void timer_cb(etimer_t *timer, etimer_ref_t ref, void *ud, int cancelled)
{
    struct timer_event *event = (struct timer_event *)ud;

    event->cancelled = cancelled;
    gettimeofday(&event->finish, NULL);

    if( ++events_counter >= events_number ) {
        etimer_stop(timer);
    }
}

void init_testcase(void)
{
    timer = etimer_create();
}

void end_testcase(void)
{
    etimer_destroy(timer);
    timer = NULL;
}

START_TEST(test_timer_basics)
{
    int ud = 0;
    etimer_ref_t ref;

    etimer_t *timer = etimer_create();
    ck_assert_ptr_ne(timer, NULL);
    ck_assert_int_ne(etimer_empty(timer), 0);

    ref = etimer_after(timer, 10, timer_cb_dummy, &ud);
    ck_assert_ptr_ne(ref, NULL);
    ck_assert_int_eq(etimer_empty(timer), 0);

    etimer_destroy(timer);
}
END_TEST

START_TEST(test_timer_run)
{
    struct timer_event events[3];
    struct timeval now, timeout;
    int i;

    events_number = 3;
    events_counter = 0;
    memset(events, 0, sizeof(struct timer_event)*events_number);
    memset(&timeout, 0, sizeof(timeout));
    gettimeofday(&now, NULL);

    timeradd(&now, &timeout, &events[0].deadline);
    etimer_after(timer, 0, timer_cb, &events[0]);

    timeout.tv_usec = 250000;
    timeradd(&now, &timeout, &events[1].deadline);
    etimer_mafter(timer, 250, timer_cb, &events[1]);

    timeout.tv_usec = 500000;
    timeradd(&now, &timeout, &events[2].deadline);
    etimer_deadline(timer, &events[2].deadline, timer_cb, &events[2]);

    etimer_run(timer);

    ck_assert_int_ne(etimer_empty(timer), 0);

    for( i = 0 ; i < events_number ; i++ ) {
        ck_assert_int_eq(events[i].cancelled, 0);
        ck_assert_int_ne(timercmp(&events[i].deadline, &events[i].finish, <), 0);
    }
}
END_TEST

START_TEST(test_timer_cancel)
{
    struct timer_event events[3];
    struct timeval now, timeout;
    etimer_ref_t ref[3];
    int i;

    events_number = 3;
    events_counter = 0;
    memset(events, 0, sizeof(struct timer_event)*events_number);
    memset(&timeout, 0, sizeof(timeout));
    gettimeofday(&now, NULL);

    timeradd(&now, &timeout, &events[0].deadline);
    ref[0] = etimer_after(timer, 0, timer_cb, &events[0]);

    timeout.tv_sec = 250000;
    timeradd(&now, &timeout, &events[1].deadline);
    ref[1] = etimer_mafter(timer, 250, timer_cb, &events[1]);

    timeout.tv_sec = 500000;
    timeradd(&now, &timeout, &events[2].deadline);
    ref[2] = etimer_deadline(timer, &events[2].deadline, timer_cb, &events[2]);

    for( i = 0 ; i < events_number ; i++ ) {
        etimer_cancel(timer, ref[i]);
    }

    ck_assert_int_ne(etimer_empty(timer), 0);

    for( i = 0 ; i < events_number ; i++ ) {
        ck_assert_int_eq(events[i].cancelled, 1);
    }
}
END_TEST

Suite * timer_suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("Timer");

    tc = tcase_create("Timer basics");
    tcase_add_test(tc, test_timer_basics);
    suite_add_tcase(s, tc);

    tc = tcase_create("Timer run");
    tcase_add_checked_fixture(tc, init_testcase, end_testcase);
    tcase_set_timeout(tc, 10);
    tcase_add_test(tc, test_timer_run);
    tcase_add_test(tc, test_timer_cancel);
    suite_add_tcase(s, tc);

    return s;
}

int main(int argc, char **argv)
{
    int failed;
    Suite *s;
    SRunner *sr;

    s = timer_suite();
    sr = srunner_create(s);
#ifdef CHECK_MODE_NOFORK
    srunner_set_fork_status(sr, CK_NOFORK);
#endif
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

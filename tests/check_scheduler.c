/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_scheduler.c -- Unit tests for basic scheduler functionality.
 */

#include <check.h>

#include "config.h"

#include "check_check.h"
#include "../src/scheduler.h"

/**
 * Ensure that time progresses when tick/tock is called in an empty system.
 */
START_TEST (test_time_progresses)
{
	// Number of ticks to do
	const ticks_t num_ticks = 10;
	
	scheduler_t s;
	scheduler_init(&s);
	
	for (int i = 0; i < num_ticks; i++) {
		ck_assert_int_eq(scheduler_get_ticks(&s), i);
		scheduler_tick_tock(&s);
	}
	
	// Finish at time num_ticks
	ck_assert_int_eq(scheduler_get_ticks(&s), num_ticks);
	
	scheduler_destroy(&s);
}
END_TEST


/**
 * For use as a callback for in test_schedule. Increments the integer pointed to
 * by the argument.
 */
void
incrementer(void *counter)
{
	(*((int *)counter))++;
}


/**
 * Ensure that we can schedule several things occurring at different periods.
 */
START_TEST (test_schedule)
{
	// Number of ticks to do
	const int num_ticks = 50;
	
	// Range of periods to set up
	const int max_period = 4;
	
	// Number of processes for each period
	const int num_processes = 2;
	
	// Sets of counters for the tick/tock events for each process of each period
	int tick_cnt[max_period * num_processes];
	int tock_cnt[max_period * num_processes];
	
	// Reset the counters
	for (int i = 1; i < max_period*num_processes; i++) {
		tick_cnt[i] = 0;
		tock_cnt[i] = 0;
	}
	
	scheduler_t s;
	scheduler_init(&s);
	
	// Increment the tick/tock counters for each period.
	for (int period = 1; period < max_period; period++) {
		for (int process = 0; process < num_processes; process++) {
			scheduler_schedule( &s, period
			                  , incrementer, tick_cnt + (num_processes*period) + process
			                  , incrementer, tock_cnt + (num_processes*period) + process
			                  );
		}
	}
	
	// Complete the specifed number of ticks
	for (int i = 0; i < num_ticks; i++)
		scheduler_tick_tock(&s);
	ck_assert_int_eq(scheduler_get_ticks(&s), num_ticks);
	
	// Check that the number of ticks and tocks is always the same
	for (int i = 1; i < max_period*num_processes; i++) {
		ck_assert_int_eq(tick_cnt[i], tock_cnt[i]);
	}
	
	// Check that the number of ticks happened an appropriate number of times
	// given the period and the runtime of the simulation. Since at time 0 all
	// periods will run, round up the number of expected executions rather than
	// down.
	for (int period = 1; period < max_period; period++) {
		for (int process = 0; process < num_processes; process++) {
			ck_assert_int_eq( tick_cnt[(num_processes*period) + process]
			                , (num_ticks+period-1) / period);
		}
	}
	
	scheduler_destroy(&s);
}
END_TEST


Suite *
make_scheduler_suite(void)
{
	Suite *s = suite_create("scheduler");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_time_progresses);
	tcase_add_test(tc_core, test_schedule);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}


/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_delay.c -- Unit tests for basic delay functionality.
 */

#include <check.h>

#include "config.h"

#include "check_check.h"
#include "../src/scheduler.h"
#include "../src/buffer.h"
#include "../src/delay.h"

scheduler_t s;

delay_t d;

buffer_t input;
buffer_t output;

#define BUFF_SIZE 5
#define PERIOD 3
#define DELAY 4


void
check_delay_setup(void)
{
	scheduler_init(&s);
	buffer_init(&input, BUFF_SIZE);
	buffer_init(&output, BUFF_SIZE);
	delay_init(&d, &s, PERIOD, DELAY, &input, &output);
}


void
check_delay_teardown(void)
{
	scheduler_destroy(&s);
	buffer_destroy(&input);
	buffer_destroy(&output);
	delay_destroy(&d);
}

/**
 * Test to see if packets are delayed appropriately when the output is not
 * blocked.
 */
START_TEST (test_unblocked_forwarding)
{
	// Fill the input buffer with things to send
	for (int i = 0; i < BUFF_SIZE; i++)
		buffer_push(&input, (void *)i);
	
	// Run the simulation to see if things arrive at the correct time
	for (int i = 0; i < BUFF_SIZE; i++) {
		for (int j = 0; j < PERIOD*DELAY; j++)
			scheduler_tick_tock(&s);
		
		// Check that the correct value was forwarded (and nothing more)
		ck_assert(!buffer_is_empty(&output));
		ck_assert((int)buffer_pop(&output) == i);
		ck_assert(buffer_is_empty(&output));
	}
}
END_TEST


/**
 * Test to see if packets are delayed appropriately when the output is blocked.
 */
START_TEST (test_blocked_forwarding)
{
	// Place a single thing to forward in the input buffer
	buffer_push(&input, (void *)1234);
	
	// Fill the output buffer to block it up
	for (int i = 0; i < BUFF_SIZE; i++)
		buffer_push(&output, NULL);
	
	// Run the simulation to see nothing happens
	for (int j = 0; j < PERIOD*DELAY; j++)
		scheduler_tick_tock(&s);
	ck_assert(!buffer_is_empty(&input));
	
	// Take something out of the output buffer and see if things arrive instantly
	buffer_pop(&output);
	for (int j = 0; j < PERIOD; j++)
		scheduler_tick_tock(&s);
	ck_assert(buffer_is_empty(&input));
	ck_assert(buffer_is_full(&output));
}
END_TEST


Suite *
make_delay_suite(void)
{
	Suite *s = suite_create("delay");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, check_delay_setup, check_delay_teardown);
	tcase_add_test(tc_core, test_unblocked_forwarding);
	tcase_add_test(tc_core, test_blocked_forwarding);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}



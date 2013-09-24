/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_arbiter.c -- Unit tests for basic arbiter functionality.
 */

#include <check.h>

#include "config.h"

#include "check_check.h"
#include "../src/scheduler.h"
#include "../src/buffer.h"
#include "../src/arbiter.h"

// Number of elements in each buffer
const size_t buf_len = 4;

// Period of the arbiter
const ticks_t period = 3;

// The scheduler to use
scheduler_t *s;

// The arbiter
arbiter_t *a;

// Input buffers (and the number of them)
#define NUM_INPUTS 3
buffer_t inputs[NUM_INPUTS];
buffer_t *inputs_p[NUM_INPUTS];

// Output buffer
buffer_t output;



/**
 * Set up the arbiter and its inputs before every test.
 */
void
check_arbiter_setup(void)
{
	s = scheduler_create();
	
	// Create input buffers
	for (int i = 0; i < NUM_INPUTS; i++) {
		buffer_init(&(inputs[i]), buf_len);
		inputs_p[i] = &(inputs[i]);
	}
	
	// Create output buffer
	buffer_init(&output, buf_len);
	
	// Create arbiter
	a = arbiter_create(s, period, inputs_p, NUM_INPUTS, &output);
}


void
check_arbiter_teardown(void)
{
	// Free evrything up
	for (int i = 0; i < NUM_INPUTS; i++) {
		buffer_destroy(&(inputs[i]));
	}
	buffer_destroy(&output);
	arbiter_free(a);
	scheduler_free(s);
}


/**
 * Ensure that a full buffer of values propagates through one-per-period
 */
START_TEST (test_single_period_forwarding)
{
	// Fill up the first input buffer (don't care about the others here)
	for (int i = 0; i < buf_len; i++) {
		buffer_push(&(inputs[0]), NULL);
	}
	
	// Run the simulation for exactly the correct number of cycles (it should not
	// push through the last packet until the last tick.
	for (int i = 0; i < (buf_len-1)*period + 1; i++) {
		// Input should still have stuff in until after the last tick
		ck_assert(!buffer_is_empty(&(inputs[0])));
		// Output buffer should never end up full until after the last tick
		ck_assert(!buffer_is_full(&output));
		
		scheduler_tick_tock(s);
	}
	
	// After the last tick, the input should have emptied into the output
	ck_assert(buffer_is_empty(&(inputs[0])));
	ck_assert(buffer_is_full(&output));
}
END_TEST


/**
 * Check that there is a round-robbin behaviour.
 */
START_TEST (test_round_robbin)
{
	// Require that the output buffer is at least long enough for one item from
	// each input
	ck_assert(NUM_INPUTS <= buf_len);
	
	// Place a couple of values in each input buffer (so that after pulling one
	// item out of each buffer the buffer is still not empty. As a bodge, identify
	// each input by casting the input number as the void pointer to send...
	for (int i = 0; i < NUM_INPUTS; i++) {
		buffer_push(&(inputs[i]), (void *)i);
		buffer_push(&(inputs[i]), (void *)i);
	}
	
	// Run the simulation for exactly the correct number of cycles
	for (int i = 0; i < (NUM_INPUTS-1)*period + 1; i++) {
		scheduler_tick_tock(s);
	}
	
	// Check the values in the output buffer correspond to each input in turn.
	for (int i = 0; i < NUM_INPUTS; i++) {
		ck_assert(!buffer_is_empty(&output));
		ck_assert((int)buffer_pop(&output) == i);
	}
	
	// We should have grabbed everything there was to grab!
	ck_assert(buffer_is_empty(&output));
}
END_TEST


/**
 * Check that when the output buffer is full, no packets propagate.
 */
START_TEST (test_output_blocked)
{
	// Place some value in the first input buffer which will not be able to
	// progress
	buffer_push(&(inputs[0]), NULL);
	
	// Fill up the output buffer to make it block
	for (int i = 0; i < buf_len; i++) {
		buffer_push(&output, (void *)i);
	}
	ck_assert(buffer_is_full(&output));
	
	// Run the simulation for one period allowing it one attempt at clearing the
	// packet.
	for (int i = 0; i < period + 1; i++) {
		scheduler_tick_tock(s);
	}
	
	// Check that the input buffer did not empty
	ck_assert(!buffer_is_empty(&(inputs[0])));
	ck_assert(buffer_is_full(&output));
	
	// Unblock it and make sure it goes again
	buffer_pop(&output);
	ck_assert(!buffer_is_full(&output));
	
	// Run the simulation for one period allowing it one attempt at clearing the
	// packet.
	for (int i = 0; i < period + 1; i++) {
		scheduler_tick_tock(s);
	}
	
	// Check that the input buffer did empty this time
	ck_assert(buffer_is_empty(&(inputs[0])));
	ck_assert(buffer_is_full(&output));
}
END_TEST


Suite *
make_arbiter_suite(void)
{
	Suite *s = suite_create("arbiter");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, check_arbiter_setup, check_arbiter_teardown);
	tcase_add_test(tc_core, test_single_period_forwarding);
	tcase_add_test(tc_core, test_round_robbin);
	tcase_add_test(tc_core, test_output_blocked);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}



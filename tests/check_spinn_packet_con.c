/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_spinn_packet_con.c -- Unit tests for the packet consumer.
 */

#include <check.h>

#include "config.h"

#include "check_check.h"

#include "../src/scheduler.h"
#include "../src/buffer.h"

#include "../src/spinn.h"
#include "../src/spinn_packet.h"

/******************************************************************************
 * Testbench
 ******************************************************************************/

#define PERIOD 3

#define BUFFER_SIZE 10

scheduler_t s;
buffer_t b;
spinn_packet_pool_t pool;
spinn_packet_con_t c;

int packets_received;

void
check_spinn_packet_con_setup(void)
{
	scheduler_init(&s);
	buffer_init(&b, BUFFER_SIZE);
	spinn_packet_pool_init(&pool);
	packets_received = 0;
}


void
check_spinn_packet_con_teardown(void)
{
	scheduler_destroy(&s);
	buffer_destroy(&b);
	spinn_packet_pool_destroy(&pool);
	spinn_packet_con_destroy(&c);
}


void
on_packet_con(spinn_packet_t *p, void *data)
{
	ck_assert(p != NULL);
	ck_assert_int_eq((int)data, 1234);
	
	packets_received++;
}

/******************************************************************************
 * Tests
 ******************************************************************************/

// Create a packet generator with most arguments set to sensible defaults.
#define INIT_CON(bernoulli_prob) \
	spinn_packet_con_init( &c, &s, &b, &pool \
	                     , PERIOD, (bernoulli_prob) \
	                     , on_packet_con, (void *)1234 \
	                     )


/**
 * Make sure nothing dies if the generator is given a zero probability of
 * accepting a packet and left to run for a while.
 */
START_TEST (test_idle)
{
	INIT_CON(0.0);
	
	// Fill the buffer with packets which are not to be accepted
	for (int i = 0; i < BUFFER_SIZE; i++)
		buffer_push(&b, spinn_packet_pool_palloc(&pool));
	
	for (int i = 0; i < PERIOD * 10; i++)
		scheduler_tick_tock(&s);
	
	// Make sure nothing got received...
	ck_assert(buffer_is_full(&b));
	
	ck_assert_int_eq(packets_received, 0);
}
END_TEST


/**
 * Make sure everything is accepted if the probability of acceptance is 1.0.
 */
START_TEST (test_active)
{
	INIT_CON(1.0);
	
	// Fill the buffer with packets which will all be accepted
	for (int i = 0; i < BUFFER_SIZE; i++)
		buffer_push(&b, spinn_packet_pool_palloc(&pool));
	
	// Make sure all packets are accepted in the expected timeframe
	for (int i = 0; i < PERIOD * BUFFER_SIZE; i++)
		scheduler_tick_tock(&s);
	ck_assert(buffer_is_empty(&b));
	
	ck_assert_int_eq(packets_received, BUFFER_SIZE);
	
	// Make sure nothing catches fire when the buffer is empty
	for (int i = 0; i < PERIOD * BUFFER_SIZE; i++)
		scheduler_tick_tock(&s);
	
	ck_assert_int_eq(packets_received, BUFFER_SIZE);
}
END_TEST


/**
 * Make sure things are sometimes accepted and sometimes not.
 *
 * Note this test assumes (incorrectly, but practically) that the probability of
 * all zeros or all ones is 0.0. If the test fails, double check that the random
 * numbers generated aren't the cause.
 */
START_TEST (test_50_50)
{
	INIT_CON(0.5);
	
	// Fill the buffer with packets, some of which will be accepted
	for (int i = 0; i < BUFFER_SIZE; i++)
		buffer_push(&b, spinn_packet_pool_palloc(&pool));
	
	for (int i = 0; i < PERIOD * BUFFER_SIZE; i++)
		scheduler_tick_tock(&s);
	
	// Make sure some packets got accepted and others didn't
	ck_assert(!buffer_is_empty(&b));
	ck_assert(!buffer_is_full(&b));
	
	ck_assert(packets_received > 0);
	ck_assert(packets_received < BUFFER_SIZE);
}
END_TEST


Suite *
make_spinn_packet_con_suite(void)
{
	Suite *s = suite_create("spinn_packet_con");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, check_spinn_packet_con_setup, check_spinn_packet_con_teardown);
	tcase_add_test(tc_core, test_idle);
	tcase_add_test(tc_core, test_active);
	tcase_add_test(tc_core, test_50_50);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}



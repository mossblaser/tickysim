/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_spinn_packet_pool.c -- Unit tests for the SpiNNaker packet pool
 */

#include <check.h>

#include "config.h"

#include "check_check.h"

#include "../src/spinn.h"
#include "../src/spinn_packet.h"

// Number of times to repeat each experiment
#define NUM_REPEATS 5

// Largest number of packets to create/free
#define NUM_PACKETS 128

spinn_packet_pool_t pool;

void
check_spinn_packet_pool_setup(void)
{
	spinn_packet_pool_init(&pool);
}


void
check_spinn_packet_pool_teardown(void)
{
	spinn_packet_pool_destroy(&pool);
}


/**
 * Test that creating and freeing an un-used packet pool works.
 */
START_TEST (test_create_free)
{
	// Don't use the pool
}
END_TEST


/**
 * Test that not freeing a packet doesn't break the freeing of the pool.
 */
START_TEST (test_no_pfree)
{
	// Get a packet which is never pfreed.
	spinn_packet_pool_palloc(&pool);
}
END_TEST


/**
 * Test that creating and freeing an un-used packet pool works and that the same
 * packet is returned each time (i.e. the allocation behaves like a stack).
 */
START_TEST (test_single_packet)
{
	spinn_packet_t *p = spinn_packet_pool_palloc(&pool);
	spinn_packet_t *last_p = p;
	spinn_packet_pool_pfree(&pool, p);
	
	for (int i = 0; i < NUM_REPEATS; i++) {
		p = spinn_packet_pool_palloc(&pool);
		ck_assert(p == last_p);
		last_p = p;
		spinn_packet_pool_pfree(&pool, p);
	}
}
END_TEST


/**
 * Test that a number of packets can be alloc'd and then freed.
 */
START_TEST (test_many_packets)
{
	spinn_packet_t *ps[NUM_PACKETS];
	
	for (int _ = 0; _ < NUM_REPEATS; _++) {
		// Create some packets
		for (int i = 0; i < NUM_PACKETS; i++) {
			ps[i] = spinn_packet_pool_palloc(&pool);
			// Make sure the packet isn't equal to any requested before.
			for (int j = 0; j < i; j++) {
				ck_assert(ps[i] != ps[j]);
			}
		}
		
		// Free some packets alternating between packets alloced most and least
		// recently just to mix things up a bit.
		for (int i_ = 0; i_ < NUM_PACKETS; i_++) {
			int i = (i_%2) ? (i_) : ((NUM_PACKETS - i_) - 2);
			spinn_packet_pool_pfree(&pool, ps[i]);
		}
	}
}
END_TEST


Suite *
make_spinn_packet_pool_suite(void)
{
	Suite *s = suite_create("spinn_packet_pool");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, check_spinn_packet_pool_setup, check_spinn_packet_pool_teardown);
	tcase_add_test(tc_core, test_create_free);
	tcase_add_test(tc_core, test_no_pfree);
	tcase_add_test(tc_core, test_single_packet);
	tcase_add_test(tc_core, test_many_packets);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}



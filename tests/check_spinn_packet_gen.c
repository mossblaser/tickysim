/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_spinn_packet_gen.c -- Unit tests for the packet generator.
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

#define POSITION    ((spinn_coord_t){2,3})
#define SYSTEM_SIZE_X 4
#define SYSTEM_SIZE_Y 7
#define SYSTEM_SIZE ((spinn_coord_t){SYSTEM_SIZE_X,SYSTEM_SIZE_Y})

scheduler_t *s;
buffer_t *b;
spinn_packet_pool_t *pool;
spinn_packet_gen_t *g;

int packets_sent;

void
check_spinn_packet_gen_setup(void)
{
	s    = scheduler_create();
	b    = buffer_create(BUFFER_SIZE);
	pool = spinn_packet_pool_create();
	g    = NULL;
	packets_sent = 0;
}


void
check_spinn_packet_gen_teardown(void)
{
	scheduler_free(s);
	buffer_free(b);
	spinn_packet_pool_free(pool);
	spinn_packet_gen_free(g);
}


void *
on_packet_gen(spinn_packet_t *p, void *data)
{
	ck_assert(p != NULL);
	ck_assert_int_eq((int)data, 1234);
	
	packets_sent++;
	
	return (void *)4321;
}

/******************************************************************************
 * Tests
 ******************************************************************************/

// Create a packet generator with most arguments set to sensible defaults.
#define INIT_GEN(create_func, bernoulli_prob) \
	g = (create_func)( s, b, pool \
	                 , POSITION, SYSTEM_SIZE \
	                 , PERIOD, (bernoulli_prob) \
	                 , on_packet_gen, (void *)1234 \
	                 )


/**
 * Make sure nothing dies if the generator is given a zero probability of
 * producing a packet and left to run for a while.
 */
START_TEST (test_idle)
{
	switch (_i) {
		default:
		case 0: INIT_GEN(spinn_packet_gen_cyclic_create, 0.0); break;
		case 1: INIT_GEN(spinn_packet_gen_uniform_create, 0.0); break;
	}
	
	for (int i = 0; i < PERIOD * 10; i++)
		scheduler_tick_tock(s);
	
	// Make sure nothing got sent...
	ck_assert(buffer_is_empty(b));
	ck_assert_int_eq(packets_sent, 0);
}
END_TEST


/**
 * Ensure that when packets are always generated when the probability is set to
 * 1.0. Also makes sure that the generator stops when the buffer is full.
 */
START_TEST (test_certain)
{
	switch (_i) {
		default:
		case 0: INIT_GEN(spinn_packet_gen_cyclic_create, 1.0); break;
		case 1: INIT_GEN(spinn_packet_gen_uniform_create, 1.0); break;
	}
	
	// Run for long enough that the buffer ends up full
	for (int i = 0; i < BUFFER_SIZE + 1; i++) {
		// Run the generator for a single period
		for (int j = 0; j < PERIOD; j++)
			scheduler_tick_tock(s);
		
		// A packet should have arrived
		ck_assert(!buffer_is_empty(b));
		
		// If the number of packets sent is larger than the buffer then the buffer
		// should be full.
		ck_assert(!!buffer_is_full(b) == !!((i+1) >= BUFFER_SIZE));
	}
	
	ck_assert_int_eq(packets_sent, BUFFER_SIZE);
}
END_TEST


/**
 * Ensure that when packets are sent with probability 50/50 that sometimes they
 * are sent and sometimes they are not. This test assumes (incorrectly, but
 * practically) that the probability of getting all 1s or all 0s is 0.0. If this
 * test fails, this possibility should be accounted for.
 */
START_TEST (test_50_50)
{
	switch (_i) {
		default:
		case 0: INIT_GEN(spinn_packet_gen_cyclic_create, 0.5); break;
		case 1: INIT_GEN(spinn_packet_gen_uniform_create, 0.5); break;
	}
	
	// Run for long enough that the buffer would end up full if the probability
	// was 1.0.
	for (int i = 0; i < PERIOD*BUFFER_SIZE; i++)
		scheduler_tick_tock(s);
	
	// Should have sent less than the maximum and more than the minimum
	ck_assert(!buffer_is_empty(b));
	ck_assert(!buffer_is_full(b));
	
	ck_assert(packets_sent > 0);
	ck_assert(packets_sent < BUFFER_SIZE);
}
END_TEST


/**
 * Ensure that the cyclic distribution sends a packet to each node exactly twice
 * given a number of iterations equal to the number of nodes.
 */
START_TEST (test_cyclic_dist)
{
	INIT_GEN(spinn_packet_gen_cyclic_create, 1.0);
	
	// A count of the number of times a node is visited
	int visited_nodes[SYSTEM_SIZE_X][SYSTEM_SIZE_Y] = {{0}};
	
	// Run for long enough that every node should be visited twice
	for (int i = 0; i < SYSTEM_SIZE.x*SYSTEM_SIZE.y*2; i++) {
		for (int j = 0; j < PERIOD; j++)
			scheduler_tick_tock(s);
		
		// A packet should have arrived, note its posiiton
		ck_assert(!buffer_is_empty(b));
		spinn_packet_t *p = (spinn_packet_t *)buffer_pop(b);
		visited_nodes[p->destination.x][p->destination.y]++;
		
		// Check the payload added by the callback is correct
		ck_assert_int_eq((int)p->payload, 4321);
		
		// Free the packet resource
		spinn_packet_pool_pfree(pool, p);
	}
	
	// Check how many times each node is visited
	for (int x = 0; x < SYSTEM_SIZE.x; x++) {
		for (int y = 0; y < SYSTEM_SIZE.y; y++) {
			ck_assert_int_eq(visited_nodes[x][y], 2);
		}
	}
	
	ck_assert_int_eq(packets_sent, SYSTEM_SIZE_X*SYSTEM_SIZE_Y*2);
}
END_TEST


Suite *
make_spinn_packet_gen_suite(void)
{
	Suite *s = suite_create("spinn_packet_gen");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, check_spinn_packet_gen_setup, check_spinn_packet_gen_teardown);
	tcase_add_loop_test(tc_core, test_idle, 0, 2);
	tcase_add_loop_test(tc_core, test_certain, 0, 2);
	tcase_add_loop_test(tc_core, test_50_50, 0, 2);
	tcase_add_test(tc_core, test_cyclic_dist);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}



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

#define INTERVAL 7

#define REPEATS 10

scheduler_t s;
buffer_t b;
spinn_packet_pool_t pool;
spinn_packet_gen_t g;

int packets_blocked;
int packets_sent;

void
check_spinn_packet_gen_setup(void)
{
	scheduler_init(&s);
	buffer_init(&b, BUFFER_SIZE);
	spinn_packet_pool_init(&pool);
	packets_blocked = 0;
	packets_sent = 0;
}


void
check_spinn_packet_gen_teardown(void)
{
	scheduler_destroy(&s);
	buffer_destroy(&b);
	spinn_packet_pool_destroy(&pool);
	spinn_packet_gen_destroy(&g);
}


void *
on_packet_gen(spinn_packet_t *p, void *data)
{
	ck_assert_int_eq((int)data, 1234);
	
	if (p == NULL)
		packets_blocked++;
	else
		packets_sent++;
	
	return (void *)4321;
}

bool
dest_filter(const spinn_coord_t *dest, void *_allow_local)
{
	bool allow_local = (bool)_allow_local;
	return !(!allow_local && dest->x == POSITION.x && dest->y == POSITION.y);
}

/******************************************************************************
 * Tests
 ******************************************************************************/

// Create a packet generator with most arguments set to sensible defaults.
#define INIT_GEN(allow_local) \
	spinn_packet_gen_init( &g, &s, &b, &pool \
	                     , POSITION, SYSTEM_SIZE \
	                     , PERIOD \
	                     , true \
	                     , dest_filter \
	                     , (void *)(allow_local) \
	                     , on_packet_gen, (void *)1234 \
	                     )

#define SET_GEN_BERNOULLI(prob) spinn_packet_gen_set_temporal_dist_bernoulli(&g, (prob))
#define SET_GEN_PERIODIC(interval) spinn_packet_gen_set_temporal_dist_periodic(&g, (interval))

#define SET_GEN_CYCLIC() spinn_packet_gen_set_spatial_dist_cyclic(&g)
#define SET_GEN_UNIFORM() spinn_packet_gen_set_spatial_dist_uniform(&g)
#define SET_GEN_P2P(target) spinn_packet_gen_set_spatial_dist_p2p(&g,(target))


/**
 * Make sure nothing dies if the generator is given a zero probability of
 * producing a packet and left to run for a while.
 */
START_TEST (test_idle)
{
	switch (_i) {
		default:
		case 0: INIT_GEN(true); SET_GEN_BERNOULLI(0.0); SET_GEN_CYCLIC(); break;
		case 1: INIT_GEN(true); SET_GEN_BERNOULLI(0.0); SET_GEN_UNIFORM(); break;
	}
	
	for (int i = 0; i < PERIOD * 10; i++)
		scheduler_tick_tock(&s);
	
	// Make sure nothing got sent...
	ck_assert(buffer_is_empty(&b));
	ck_assert_int_eq(packets_sent, 0);
	ck_assert_int_eq(packets_blocked, 0);
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
		case 0: INIT_GEN(true); SET_GEN_BERNOULLI(1.0); SET_GEN_CYCLIC(); break;
		case 1: INIT_GEN(true); SET_GEN_BERNOULLI(1.0); SET_GEN_UNIFORM(); break;
	}
	
	// Run for long enough that the buffer ends up full
	for (int i = 0; i < BUFFER_SIZE + 1; i++) {
		// Run the generator for a single period
		for (int j = 0; j < PERIOD; j++)
			scheduler_tick_tock(&s);
		
		// A packet should have arrived
		ck_assert(!buffer_is_empty(&b));
		
		// If the number of packets sent is larger than the buffer then the buffer
		// should be full.
		ck_assert(!!buffer_is_full(&b) == !!((i+1) >= BUFFER_SIZE));
	}
	
	ck_assert_int_eq(packets_sent, BUFFER_SIZE);
	ck_assert_int_eq(packets_blocked, 1);
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
		case 0: INIT_GEN(true); SET_GEN_BERNOULLI(0.5); SET_GEN_CYCLIC(); break;
		case 1: INIT_GEN(true); SET_GEN_BERNOULLI(0.5); SET_GEN_UNIFORM(); break;
	}
	
	// Run for long enough that the buffer would end up full if the probability
	// was 1.0.
	for (int i = 0; i < PERIOD*BUFFER_SIZE; i++)
		scheduler_tick_tock(&s);
	
	// Should have sent less than the maximum and more than the minimum
	ck_assert(!buffer_is_empty(&b));
	ck_assert(!buffer_is_full(&b));
	
	ck_assert(packets_sent > 0);
	ck_assert(packets_sent < BUFFER_SIZE);
	ck_assert_int_eq(packets_blocked, 0);
}
END_TEST

/**
 * Ensure that the cyclic distribution sends a packet to each node exactly twice
 * given a number of iterations equal to the number of nodes. Also tests the
 * allow_local option (by running with this both enabled and disabled).
 */
START_TEST (test_cyclic_dist)
{
	bool allow_local = _i != 0;
	INIT_GEN(allow_local); SET_GEN_BERNOULLI(1.0); SET_GEN_CYCLIC();
	
	// A count of the number of times a node is visited
	int visited_nodes[SYSTEM_SIZE_X][SYSTEM_SIZE_Y] = {{0}};
	
	// Run for long enough that every node should be visited twice
	for (int i = 0; i < (SYSTEM_SIZE.x*SYSTEM_SIZE.y*2) - (allow_local?0:2); i++) {
		for (int j = 0; j < PERIOD; j++)
			scheduler_tick_tock(&s);
		
		// A packet should have arrived, note its posiiton
		ck_assert(!buffer_is_empty(&b));
		spinn_packet_t *p = (spinn_packet_t *)buffer_pop(&b);
		visited_nodes[p->destination.x][p->destination.y]++;
		
		// Check the payload added by the callback is correct
		ck_assert_int_eq((int)p->payload, 4321);
		
		// Free the packet resource
		spinn_packet_pool_pfree(&pool, p);
	}
	
	// Check how many times each node is visited
	for (int x = 0; x < SYSTEM_SIZE.x; x++) {
		for (int y = 0; y < SYSTEM_SIZE.y; y++) {
			if (allow_local) {
				ck_assert_int_eq(visited_nodes[x][y], 2);
			} else {
				ck_assert_int_eq(visited_nodes[x][y], (POSITION.x==x && POSITION.y==y)?0:2);
			}
		}
	}
	
	ck_assert_int_eq(packets_sent, SYSTEM_SIZE_X*SYSTEM_SIZE_Y*2 - (allow_local?0:2));
	ck_assert_int_eq(packets_blocked, 0);
}
END_TEST

/**
 * Ensure that the p2p distribution sends a packets to only the specified node
 * or none at all if the specified node is (-1,-1).
 */
START_TEST (test_p2p_dist)
{
	bool null_destination = _i != 0;
	INIT_GEN(true); SET_GEN_BERNOULLI(1.0);
	if (!null_destination)
		SET_GEN_P2P(((spinn_coord_t){0,0}));
	else
		SET_GEN_P2P(((spinn_coord_t){-1,-1}));
	
	// Run to allow REPEATS packets to be generated.
	for (int i = 0; i < REPEATS; i++) {
		for (int j = 0; j < PERIOD; j++)
			scheduler_tick_tock(&s);
		
		if (!null_destination) {
			// A packet should have arrived, note its posiiton
			ck_assert(!buffer_is_empty(&b));
			spinn_packet_t *p = (spinn_packet_t *)buffer_pop(&b);
			ck_assert_int_eq(p->destination.x, 0);
			ck_assert_int_eq(p->destination.y, 0);
			
			// Check the payload added by the callback is correct
			ck_assert_int_eq((int)p->payload, 4321);
			
			// Free the packet resource
			spinn_packet_pool_pfree(&pool, p);
		} else {
			// No packets should have been generated
			ck_assert(buffer_is_empty(&b));
		}
	}
	
	if (!null_destination)
		ck_assert_int_eq(packets_sent, REPEATS);
	else
		ck_assert_int_eq(packets_sent, 0);
	ck_assert_int_eq(packets_blocked, 0);
}
END_TEST


/**
 * Ensure with a periodic interval, packets are sent at the correct times when
 * the output is not blocked.
 */
START_TEST (test_periodic_free)
{
	switch (_i) {
		default:
		case 0: INIT_GEN(true); SET_GEN_PERIODIC(INTERVAL); SET_GEN_CYCLIC(); break;
		case 1: INIT_GEN(true); SET_GEN_PERIODIC(INTERVAL); SET_GEN_UNIFORM(); break;
	}
	
	// Run for long enough that the buffer ends up full
	for (int i = 0; i < BUFFER_SIZE; i++) {
		// Run the generator for a single interval until the end of which no packets
		// should be sent
		for (int j = 0; j < INTERVAL; j++) {
			ck_assert_int_eq(packets_sent, i);
			for (int k = 0; k < PERIOD; k++) {
				scheduler_tick_tock(&s);
			}
		}
		
		// A packet should have arrived by now
		ck_assert(!buffer_is_empty(&b));
		ck_assert_int_eq(packets_sent, i+1);
	}
	
	// The buffer should end up full
	ck_assert(buffer_is_full(&b));
	
	ck_assert_int_eq(packets_sent, BUFFER_SIZE);
	ck_assert_int_eq(packets_blocked, 0);
}
END_TEST


/**
 * Ensure with a periodic interval, packets are sent at the correct times when
 * the output is blocked.
 */
START_TEST (test_periodic_blocked)
{
	switch (_i) {
		default:
		case 0: INIT_GEN(true); SET_GEN_PERIODIC(INTERVAL); SET_GEN_CYCLIC(); break;
		case 1: INIT_GEN(true); SET_GEN_PERIODIC(INTERVAL); SET_GEN_UNIFORM(); break;
	}
	
	// Fill the buffer
	for (int i = 0; i < BUFFER_SIZE; i++)
		buffer_push(&b, NULL);
	
	// Run the generator for one and a half intervals, during which time nothing
	// should be sent and we end up at what would be mid-interval had the packet
	// been sent.
	for (int j = 0; j < PERIOD*(INTERVAL + (INTERVAL/2)); j++) {
		scheduler_tick_tock(&s);
		// Nothing should have got through
		ck_assert_int_eq(packets_sent, 0);
	}
	
	// If a couple of spaces are made in the buffer, the generator should
	// immediately generate a packet.
	buffer_pop(&b);
	buffer_pop(&b);
	for (int k = 0; k < PERIOD; k++)
		scheduler_tick_tock(&s);
	ck_assert_int_eq(packets_sent, 1);
	ck_assert(!buffer_is_full(&b));
	
	// The generator should then generate a further packet exactly one interval
	// later
	for (int j = 0; j < INTERVAL; j++) {
		// The second value should not have got through yet
		ck_assert_int_eq(packets_sent, 1);
		
		for (int k = 0; k < PERIOD; k++) {
			scheduler_tick_tock(&s);
		}
	}
	ck_assert_int_eq(packets_sent, 2);
	ck_assert(buffer_is_full(&b));
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
	tcase_add_loop_test(tc_core, test_periodic_free, 0, 2);
	tcase_add_loop_test(tc_core, test_periodic_blocked, 0, 2);
	tcase_add_loop_test(tc_core, test_cyclic_dist, 0, 2);
	tcase_add_loop_test(tc_core, test_p2p_dist, 0, 2);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}



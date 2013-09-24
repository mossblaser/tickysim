/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_spinn_router.c -- Unit tests for the router engine.
 */

#include <check.h>

#include "config.h"

#include "check_check.h"

#include "../src/scheduler.h"
#include "../src/buffer.h"

#include "../src/spinn.h"
#include "../src/spinn_topology.h"
#include "../src/spinn_packet.h"
#include "../src/spinn_router.h"

/******************************************************************************
 * Testbench
 ******************************************************************************/

#define ROUTER_PERIOD 3

#define FIRST_TIMEOUT 20
#define FINAL_TIMEOUT 80

#define OUT_BUFFER_SIZE 2

scheduler_t s;
buffer_t input;
buffer_t outputs[7];
buffer_t *outputs_p[7];

// A number of packets sufficient to fill all the buffers
#define NUM_PACKETS ((7*OUT_BUFFER_SIZE*2) + 1)
spinn_packet_t packets[NUM_PACKETS];

spinn_router_t r;


// A record of a call to on_forward
typedef struct on_forward_debug_data {
	// Number of calls to on_forward
	int                num_calls;
	
	// Simulation time the last call occurred
	ticks_t            time;
	
	// Details of the packet sent
	spinn_packet_t    *packet;
} on_forward_debug_data_t;

on_forward_debug_data_t last_on_forward;


// A record of a call to on_drop
typedef struct on_drop_debug_data {
	// Number of calls to on_drop
	int                num_calls;
	
	// Simulation time the last call occurred
	ticks_t            time;
	
	// Details of the packet dropped
	spinn_packet_t    *packet;
} on_drop_debug_data_t;

on_drop_debug_data_t last_on_drop;



/**
 * Set up all the things needed by a router before each test.
 */
void
check_spinn_router_setup(void)
{
	scheduler_init(&s);
	
	// Create input buffer long enough to completely fill the output buffers and
	// then block
	buffer_init(&input, (7 * OUT_BUFFER_SIZE) + 1);
	
	for (int i = 0; i < 7; i++) {
		buffer_init(&(outputs[i]), OUT_BUFFER_SIZE);
		outputs_p[i] = &(outputs[i]);
	}
	
	last_on_forward.num_calls = 0;
	last_on_drop.num_calls = 0;
	
	last_on_forward.packet = NULL;
	last_on_drop.packet = NULL;
}


void
check_spinn_router_teardown(void)
{
	scheduler_destroy(&s);
	buffer_destroy(&input);
	for (int i = 0; i < 7; i++)
		buffer_destroy(&(outputs[i]));
	spinn_router_destroy(&r);
}

/**
 * A callback handler for forwarding which counts and records calls to itself in
 * last_on_forward.
 */
void
on_forward( spinn_router_t    *router
          , spinn_packet_t    *packet
          , void              *data
          )
{
	// Make sure the right data field is set
	ck_assert((on_forward_debug_data_t *)data == &last_on_forward);
	
	// Make sure the router is correct
	ck_assert(&r == router);
	
	// Update the last_on_forward
	last_on_forward.num_calls ++;
	last_on_forward.time      = scheduler_get_ticks(&s);
	last_on_forward.packet    = packet;
}


/**
 * A callback handler for dropping which counts and records calls to itself in
 * last_on_drop.
 */
void
on_drop( spinn_router_t *router
       , spinn_packet_t *packet
       , void           *data
       )
{
	// Make sure the right data field is set
	ck_assert((on_drop_debug_data_t *)data == &last_on_drop);
	
	// Make sure the router is correct
	ck_assert(&r == router);
	
	// Update the last_on_forward
	last_on_drop.num_calls ++;
	last_on_drop.time      = scheduler_get_ticks(&s);
	last_on_drop.packet    = packet;
}


/******************************************************************************
 * Tests
 ******************************************************************************/

// Create a router with most arguments set to sensible defaults.
#define INIT_ROUTER(use_emg_routing, on_forward, on_drop) \
	spinn_router_init( &r, &s, ROUTER_PERIOD \
	                 , &input, outputs_p \
	                 , ((spinn_coord_t){0,0}) \
	                 , (use_emg_routing) \
	                 , FIRST_TIMEOUT, FINAL_TIMEOUT \
	                 , (on_forward), (void *)&last_on_forward \
	                 , (on_drop),    (void *)&last_on_drop \
	                 )


/**
 * Make sure nothing dies if the router is left idle...
 */
START_TEST (test_idle)
{
	INIT_ROUTER(true, on_forward, on_drop);
	
	// Run for such a time that if anything tried to time out, it would have
	for (int i = 0; i < ROUTER_PERIOD*(FIRST_TIMEOUT + FINAL_TIMEOUT)*2; i++)
		scheduler_tick_tock(&s);
	
	// No callbacks should have occurred
	ck_assert_int_eq(last_on_forward.num_calls, 0);
	ck_assert_int_eq(last_on_drop.num_calls,    0);
}
END_TEST



/**
 * Test that when a packet is fed to the router which is not an emergency routed
 * packet, and is not at an inflection point it is correctly routed to the right
 * output in a single period.
 */
START_TEST (test_single_normal_packet)
{
	INIT_ROUTER(true, on_forward, on_drop);
	
	const spinn_direction_t direction = (spinn_direction_t)_i;
	
	// Create a packet going in the given direction
	spinn_packet_t p;
	p.inflection_point     = (spinn_coord_t){-1,-1};
	p.inflection_direction = SPINN_NORTH;
	p.destination          = (spinn_coord_t){-1,-1};
	p.direction            = direction;
	p.emg_state            = SPINN_EMG_NORMAL;
	p.payload              = NULL;
	
	buffer_push(&input, (void *)&p);
	
	// Lets see what happens
	scheduler_tick_tock(&s);
	
	// Make sure the right buffers are occupied
	ck_assert(buffer_is_empty(&input));
	for (int i = 0; i < 7; i++) {
		if (i == direction)
			ck_assert(!buffer_is_empty(&(outputs[i])));
		else
			ck_assert(buffer_is_empty(&(outputs[i])));
	}
	
	// Remove the packet from the output, it should be the one we put into the
	// input and should be unchanged.
	ck_assert(buffer_pop(&(outputs[direction])) == (void *)&p);
	ck_assert(p.inflection_point.x == -1);
	ck_assert(p.inflection_point.y == -1);
	ck_assert(p.inflection_direction == SPINN_NORTH);
	ck_assert(p.destination.x == -1);
	ck_assert(p.destination.y == -1);
	ck_assert(p.direction == direction);
	ck_assert(p.emg_state == SPINN_EMG_NORMAL);
	ck_assert(p.payload == NULL);
	
	// Make sure the callback happend as you'd hope.
	ck_assert_int_eq(last_on_drop.num_calls,    0);
	ck_assert_int_eq(last_on_forward.num_calls, 1);
	ck_assert_int_eq(last_on_forward.time, 0);
	ck_assert_int_eq(last_on_forward.packet, &p);
	
	// Make sure nothing else happens
	for (int i = 0; i < ROUTER_PERIOD*(FIRST_TIMEOUT + FINAL_TIMEOUT)*2; i++)
		scheduler_tick_tock(&s);
	
	// Make sure no buffers became occupied
	ck_assert(buffer_is_empty(&input));
	for (int i = 0; i < 7; i++)
		ck_assert(buffer_is_empty(&(outputs[i])));
	
	// Make sure no other callbacks happend
	ck_assert_int_eq(last_on_drop.num_calls,    0);
	ck_assert_int_eq(last_on_forward.num_calls, 1);
}
END_TEST


/**
 * Test that if a series of packets are sent they arrive in the correct places
 * and one per router cycle.
 */
START_TEST (test_multiple_normal_packet)
{
	INIT_ROUTER(true, on_forward, on_drop);
	
	// Set up a series of packets going in all directions
	spinn_packet_t *p = packets;
	for (int i = 0; i < OUT_BUFFER_SIZE; i++) {
		for (int direction = 0; direction < 6; direction++) {
			p->inflection_point     = (spinn_coord_t){-1,-1};
			p->inflection_direction = SPINN_NORTH;
			p->destination          = (spinn_coord_t){-1,-1};
			p->direction            = (spinn_direction_t)direction;
			p->emg_state            = SPINN_EMG_NORMAL;
			p->payload              = NULL;
			
			buffer_push(&input, (void *)p);
			
			// Advance to the next packet
			p++;
		}
	}
	
	// Ensure that the packets appear at the outputs in the correct order and at
	// the correct time.
	p = packets;
	for (int i = 0; i < OUT_BUFFER_SIZE; i++) {
		for (int direction = 0; direction < 6; direction++) {
			// Run the simulation for a router cycle 
			for (int j = 0; j < ROUTER_PERIOD; j++)
				scheduler_tick_tock(&s);
			
			// A packet should have been forwarded
			ck_assert(last_on_forward.packet != NULL);
			
			// Did the packet get sent?
			ck_assert_msg(last_on_forward.packet == p,
				"Expected Packet %d to arrive but packet %d arrived instead.",
				(packets - p) / sizeof(spinn_packet_t),
				(packets - last_on_forward.packet) / sizeof(spinn_packet_t)
				);
			
			// Have the correct number of packets been sent?
			ck_assert_int_eq(last_on_forward.num_calls, 1 + (i*6) + direction);
			
			// Did the packet remain in the correct state/direction?
			ck_assert_int_eq((int)p->direction, (int)direction);
			ck_assert_int_eq((int)p->emg_state, (int)SPINN_EMG_NORMAL);
			
			// Advance to the next packet
			p++;
		}
	}
}
END_TEST


/**
 * Test that packets destined for a chip are redirected to the local port.
 */
START_TEST (test_normal_packet_arrival)
{
	INIT_ROUTER(true, on_forward, on_drop);
	
	// Packet states where a packet may concievably be delivered
	const spinn_emg_state_t emg_types[] = { SPINN_EMG_NORMAL, SPINN_EMG_SECOND_LEG };
	const int emg_types_len = sizeof(emg_types) / sizeof(spinn_emg_state_t);
	
	// Set up a series of packets going in all directions and of all emergency
	// types which don't change the packet's destination.
	spinn_packet_t *p = packets;
	for (int i = 0; i < emg_types_len; i++) {
		for (int direction = 0; direction < 7; direction++) {
			p->inflection_point     = (spinn_coord_t){-1,-1};
			p->inflection_direction = SPINN_NORTH;
			p->destination          = (spinn_coord_t){0,0};
			p->direction            = (spinn_direction_t)direction;
			p->emg_state            = emg_types[i];
			p->payload              = NULL;
			
			buffer_push(&input, (void *)p);
			
			// Advance to the next packet
			p++;
		}
	}
	
	// Ensure that the packets appear at the outputs in the correct order and at
	// the correct time.
	p = packets;
	for (int i = 0; i < emg_types_len; i++) {
		for (int direction = 0; direction < 7; direction++) {
			// Run the simulation for a router cycle 
			for (int j = 0; j < ROUTER_PERIOD; j++)
				scheduler_tick_tock(&s);
			
			// A packet should have been forwarded
			ck_assert(last_on_forward.packet != NULL);
			
			// Did the packet get sent?
			ck_assert_msg(last_on_forward.packet == p,
				"Expected Packet %d to arrive but packet %d arrived instead.",
				(packets - p) / sizeof(spinn_packet_t),
				(packets - last_on_forward.packet) / sizeof(spinn_packet_t)
				);
			
			// Should have arrived in the local buffer too
			ck_assert(!buffer_is_empty(&(outputs[SPINN_LOCAL])));
			ck_assert(p == buffer_pop(&(outputs[SPINN_LOCAL])));
			
			// Have the correct number of packets been sent?
			ck_assert_int_eq(last_on_forward.num_calls, 1 + (i*7) + direction);
			
			// Did the packet end up at the local node in a non-emergency state?
			ck_assert_int_eq((int)p->direction, (int)SPINN_LOCAL);
			ck_assert_int_eq((int)p->emg_state, (int)SPINN_EMG_NORMAL);
			
			// Advance to the next packet
			p++;
		}
	}
}
END_TEST


/**
 * Test that packets destined for a blocked local port are dropped.
 * blocked. Runs once with emergency routing enabled and once with it disabled.
 */
START_TEST (test_normal_packet_drop)
{
	bool use_emg_routing = _i == 0;
	
	INIT_ROUTER(use_emg_routing, on_forward, on_drop);
	
	// Packet states
	const spinn_emg_state_t emg_types[] = { SPINN_EMG_NORMAL, SPINN_EMG_SECOND_LEG };
	const int emg_types_len = sizeof(emg_types) / sizeof(spinn_emg_state_t);
	
	// Set up a series of packets going in all directions and of all emergency
	// types which don't change the packet's destination.
	spinn_packet_t *p = packets;
	for (int i = 0; i < emg_types_len; i++) {
		for (int direction = 0; direction < 7; direction++) {
			p->inflection_point     = (spinn_coord_t){-1,-1};
			p->inflection_direction = SPINN_NORTH;
			p->destination          = (spinn_coord_t){0,0};
			p->direction            = (spinn_direction_t)direction;
			p->emg_state            = emg_types[i];
			p->payload              = NULL;
			
			buffer_push(&input, (void *)p);
			
			// Advance to the next packet
			p++;
		}
	}
	
	// Fill up all the output buffers to ensure that all packets will be dropped
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < OUT_BUFFER_SIZE; j++) {
			buffer_push(&(outputs[i]), NULL);
		}
	}
	
	int drop_cycles = use_emg_routing ? (FIRST_TIMEOUT+FINAL_TIMEOUT)
	                                  : FIRST_TIMEOUT
	                                  ;
	
	// See that the packets are duly dropped
	p = packets;
	for (int i = 0; i < emg_types_len; i++) {
		for (int direction = 0; direction < 7; direction++) {
			// Run the simulation for as long as it should take to time out
			for (int j = 0; j < (ROUTER_PERIOD * (drop_cycles + 1)); j++)
				scheduler_tick_tock(&s);
			
			// Check the packet got dropped
			ck_assert(last_on_drop.packet != NULL);
			ck_assert(last_on_drop.packet == p);
			
			// And that it got dropped on exactly this router cycle
			ck_assert_int_eq(last_on_drop.time, scheduler_get_ticks(&s) - ROUTER_PERIOD);
			
			// Advance to the next packet
			p++;
		}
	}
}
END_TEST

/**
 * Test that normal and second-leg packets are emergency routed appropriately if
 * a timeout occurrs.
 */
START_TEST (test_emg_first_leg)
{
	INIT_ROUTER(true, on_forward, on_drop);
	
	// Packet states
	const spinn_emg_state_t emg_types[] = { SPINN_EMG_NORMAL, SPINN_EMG_SECOND_LEG };
	
	// What type of packet and from what direction?
	spinn_emg_state_t emg_type  = emg_types[_i%2];
	spinn_direction_t direction = (spinn_direction_t)_i/2;
	
	// Place the packet in the input buffer
	spinn_packet_t *p = packets;
	p->inflection_point     = (spinn_coord_t){-1,-1};
	p->inflection_direction = SPINN_NORTH;
	p->destination          = (spinn_coord_t){-1,-1};
	p->direction            = direction;
	p->emg_state            = emg_type;
	p->payload              = NULL;
	
	buffer_push(&input, (void *)p);
	
	// The direction the packet would normally go
	spinn_direction_t normal_direction = (emg_type==SPINN_EMG_NORMAL) ? direction
		                                                                : spinn_next_cw(direction);
	
	// Fill up the expected output buffer
	for (int j = 0; j < OUT_BUFFER_SIZE; j++) {
		buffer_push(&(outputs[normal_direction]), NULL);
	}
	
	// See that the packet is forwarded after timing out to the emergency port
	for (int j = 0; j < (ROUTER_PERIOD * (FIRST_TIMEOUT + 1)); j++)
		scheduler_tick_tock(&s);
	
	// Make sure the correct callbacks occurred
	ck_assert_int_eq(last_on_drop.num_calls,    0);
	ck_assert_int_eq(last_on_forward.num_calls, 1);
	
	// Check the packet got emergency routed
	ck_assert(last_on_forward.packet == p);
	ck_assert_int_eq(p->emg_state, SPINN_EMG_FIRST_LEG);
	ck_assert_int_eq(p->direction, spinn_next_cw(normal_direction));
	
	// And that it got forwarded on exactly this router cycle (when it was
	// expected)
	ck_assert_int_eq(last_on_forward.time, scheduler_get_ticks(&s) - ROUTER_PERIOD);
}
END_TEST


/**
 * Test that first-leg packets are emergency routed appropriately.
 */
START_TEST (test_emg_second_leg)
{
	INIT_ROUTER(true, on_forward, on_drop);
	
	// What type of packet and from what direction?
	spinn_direction_t direction = (spinn_direction_t)_i;
	
	// Place the packet in the input buffer
	spinn_packet_t *p = packets;
	p->inflection_point     = (spinn_coord_t){-1,-1};
	p->inflection_direction = SPINN_NORTH;
	p->destination          = (spinn_coord_t){-1,-1};
	p->direction            = direction;
	p->emg_state            = SPINN_EMG_FIRST_LEG;
	p->payload              = NULL;
	
	buffer_push(&input, (void *)p);
	
	// See that the packet is forwarded immediately
	for (int j = 0; j < ROUTER_PERIOD; j++)
		scheduler_tick_tock(&s);
	
	// Make sure the correct callbacks occurred
	ck_assert_int_eq(last_on_drop.num_calls,    0);
	ck_assert_int_eq(last_on_forward.num_calls, 1);
	
	// Check the packet got emergency routed
	ck_assert(last_on_forward.packet == p);
	ck_assert_int_eq(p->emg_state, SPINN_EMG_SECOND_LEG);
	ck_assert_int_eq(p->direction, spinn_next_cw(spinn_opposite(direction)));
}
END_TEST


Suite *
make_spinn_router_suite(void)
{
	Suite *s = suite_create("spinn_router");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, check_spinn_router_setup, check_spinn_router_teardown);
	tcase_add_test(tc_core, test_idle);
	tcase_add_loop_test(tc_core, test_single_normal_packet, 0, 6);
	tcase_add_test(tc_core, test_multiple_normal_packet);
	tcase_add_test(tc_core, test_normal_packet_arrival);
	tcase_add_loop_test(tc_core, test_normal_packet_drop, 0, 2);
	tcase_add_loop_test(tc_core, test_emg_first_leg, 0, 6*2);
	tcase_add_loop_test(tc_core, test_emg_second_leg, 0, 6);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}



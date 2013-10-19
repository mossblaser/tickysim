/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_spinn_packet_init_dor.c -- Unit tests for the SpiNNaker packet DOR
 * initialisation utility function. Assumes that the topology library is
 * correct.
 */

#include <check.h>
#include <stdlib.h>

#include "config.h"

#include "check_check.h"

#include "../src/spinn.h"
#include "../src/spinn_packet.h"
#include "../src/spinn_topology.h"


/**
 * A hand-written test of a basic routing case for sanity-maintenance purposes.
 */
START_TEST (test_manual)
{
	spinn_packet_t p;
	
	// Set it to something inappropriate (to make sure it is overwritten)
	p.emg_state = SPINN_EMG_FIRST_LEG;
	
	spinn_packet_init_dor( &p
	                     , (spinn_coord_t){0,0}
	                     , (spinn_coord_t){2,1}
	                     , (spinn_coord_t){5,5}
	                     , true
	                     , (void *)1024
	                     );
	ck_assert_int_eq(p.direction, SPINN_EAST);
	
	ck_assert_int_eq(p.destination.x, 2);
	ck_assert_int_eq(p.destination.y, 1);
	
	ck_assert_int_eq(p.inflection_point.x, 1);
	ck_assert_int_eq(p.inflection_point.y, 0);
	ck_assert_int_eq(p.inflection_direction, SPINN_NORTH_EAST);
	
	ck_assert_int_eq(p.emg_state, SPINN_EMG_NORMAL);
	
	ck_assert_int_eq((int)p.payload, 1024);
}
END_TEST

/**
 * Test creation of packets between every pair of points in the system for
 * various system sizes/shapes. Assumes that if there is no inflection point,
 * the inflection point is placed at the destination.
 */
START_TEST (test_exhaustive)
{
	const spinn_coord_t test_sizes[] = {
		// Square systems
		{1,1}, {2,2}, {3,3}, {8,8}, {9,9},
		// Non-square systems
		{1,1}, {1,2}, {1,3}, {4,5}, {4,6}, {4,7}, {4,8},
		       {2,1}, {3,1}, {5,4}, {6,4}, {7,4}, {8,4},
	};
	const int num_tests = sizeof(test_sizes)/sizeof(spinn_coord_t);
	
	// For various sizes, exhaustively test the algorithm
	for (int i = 0; i < num_tests; i++) {
		for (int use_wrap_around_links = 0; use_wrap_around_links < 2; use_wrap_around_links++) {
			for (int y1 = 0; y1 < test_sizes[i].y; y1++) {
				for (int x1 = 0; x1 < test_sizes[i].x; x1++) {
					for (int y2 = 0; y2 < test_sizes[i].y; y2++) {
						for (int x2 = 0; x2 < test_sizes[i].x; x2++) {
							spinn_packet_t p;
							// Set it to something inappropriate (to make sure it is overwritten)
							p.emg_state = SPINN_EMG_FIRST_LEG;
							spinn_packet_init_dor( &p
							                     , (spinn_coord_t){x1,y1}
							                     , (spinn_coord_t){x2,y2}
							                     , test_sizes[i]
							                     , use_wrap_around_links
							                     , (void *)1024
							                     );
							
							// Check the basic essentials
							ck_assert_int_eq(p.destination.x, x2);
							ck_assert_int_eq(p.destination.y, y2);
							ck_assert_int_eq(p.emg_state, SPINN_EMG_NORMAL);
							ck_assert_int_eq((int)p.payload, 1024);
							
							// Test an assumption made by this test: if path does not have an
							// inflection point, the inflection point will be set to the
							// destination.
							if (p.inflection_direction == SPINN_LOCAL) {
								ck_assert_int_eq(p.inflection_point.x, x2);
								ck_assert_int_eq(p.inflection_point.y, y2);
							}
							
							// Find the shortest path
							spinn_full_coord_t v, v1, v2;
							if (use_wrap_around_links) {
								v = spinn_shortest_vector( (spinn_coord_t){x1, y1}
								                         , (spinn_coord_t){x2, y2}
								                         , test_sizes[i]
								                         );
								// Find the vector to and from the inflection point.
								v1 = spinn_shortest_vector( (spinn_coord_t){x1, y1}
								                          , p.inflection_point
								                          , test_sizes[i]
								                          );
								v2 = spinn_shortest_vector( p.inflection_point
								                          , (spinn_coord_t){x2, y2}
								                          , test_sizes[i]
								                          );
							} else {
								v = spinn_full_coord_minimise((spinn_full_coord_t){x2-x1, y2-y1, 0});
								v1 = spinn_full_coord_minimise((spinn_full_coord_t){
									p.inflection_point.x-x1,
									p.inflection_point.y-y1,
									0
								});
								v2 = spinn_full_coord_minimise((spinn_full_coord_t){
									x2-p.inflection_point.x,
									y2-p.inflection_point.y,
									0
								});
							}
							
							// Test that the vectors to/from the inflection point travel along
							// exactly one axis.
							int v1_axes_used = 0;
							if (v1.x != 0) v1_axes_used++;
							if (v1.y != 0) v1_axes_used++;
							if (v1.z != 0) v1_axes_used++;
							ck_assert(v1_axes_used <= 1);
							
							int v2_axes_used = 0;
							if (v2.x != 0) v2_axes_used++;
							if (v2.y != 0) v2_axes_used++;
							if (v2.z != 0) v2_axes_used++;
							ck_assert(v2_axes_used <= 1);
							
							// Test that the vectors add up to the theoretical shortest path
							ck_assert_int_eq( spinn_magnitude(v1) + spinn_magnitude(v2)
							                , spinn_magnitude(v)
							                );
							
							// Directions before and after the inflection point
							spinn_direction_t d1 = p.direction;
							spinn_direction_t d2 = p.inflection_direction;
							
							// The direction should initially be set to the vector to the
							// inflection point (or if the vector is 0s, the direction should be
							// the same as after the inflection point). When the vector is half
							// the system's width then the direction could go both ways.
							int w2 = (test_sizes[i].x%2 == 0) ? test_sizes[i].x/2 : 0;
							int h2 = (test_sizes[i].y%2 == 0) ? test_sizes[i].y/2 : 0;
							int z2 = (test_sizes[i].y == test_sizes[i].y && test_sizes[i].y%2 == 0) ? test_sizes[i].y/2 : 0;
							     if (v1.x < 0 && abs(v1.x) != w2) ck_assert_int_eq(d1, SPINN_WEST);
							else if (v1.x > 0 && abs(v1.x) != w2) ck_assert_int_eq(d1, SPINN_EAST);
							else if (v1.x !=0 && abs(v1.x) == w2) ck_assert(d1 == SPINN_EAST || d1 == SPINN_WEST);
							else if (v1.y < 0 && abs(v1.y) != h2) ck_assert_int_eq(d1, SPINN_SOUTH);
							else if (v1.y > 0 && abs(v1.y) != h2) ck_assert_int_eq(d1, SPINN_NORTH);
							else if (v1.y !=0 && abs(v1.y) == h2) ck_assert(d1 == SPINN_SOUTH || d1 == SPINN_NORTH);
							else if (v1.z < 0 && abs(v1.z) != z2) ck_assert_int_eq(d1, SPINN_NORTH_EAST);
							else if (v1.z > 0 && abs(v1.z) != z2) ck_assert_int_eq(d1, SPINN_SOUTH_WEST);
							else if (v1.z !=0 && abs(v1.z) == z2) ck_assert(d1 == SPINN_NORTH_EAST || d1 == SPINN_SOUTH_WEST);
							else                                  ck_assert_int_eq(d1, d2);
							
							// The direction after the inflection point should point in the
							// direction of the vector after the inflection point. If all 0s,
							// then the packet must be a self-loop and be destined locally. When
							// the vector is half the system's width then the direction could go
							// both ways.
							     if (v2.x < 0 && abs(v2.x) != w2) ck_assert_int_eq(d2, SPINN_WEST);
							else if (v2.x > 0 && abs(v2.x) != w2) ck_assert_int_eq(d2, SPINN_EAST);
							else if (v2.x !=0 && abs(v2.x) == w2) ck_assert(d2 == SPINN_EAST || d2 == SPINN_WEST);
							else if (v2.y < 0 && abs(v2.y) != h2) ck_assert_int_eq(d2, SPINN_SOUTH);
							else if (v2.y > 0 && abs(v2.y) != h2) ck_assert_int_eq(d2, SPINN_NORTH);
							else if (v2.y !=0 && abs(v2.y) == h2) ck_assert(d2 == SPINN_SOUTH || d2 == SPINN_NORTH);
							else if (v2.z < 0 && abs(v2.z) != z2) ck_assert_int_eq(d2, SPINN_NORTH_EAST);
							else if (v2.z > 0 && abs(v2.z) != z2) ck_assert_int_eq(d2, SPINN_SOUTH_WEST);
							else if (v2.z !=0 && abs(v2.z) == z2) ck_assert(d2 == SPINN_NORTH_EAST || d2 == SPINN_SOUTH_WEST);
							else                                  ck_assert_int_eq(d2, SPINN_LOCAL);
						}
					}
				}
			}
		}
	}
}
END_TEST


Suite *
make_spinn_packet_init_dor(void)
{
	Suite *s = suite_create("spinn_packet_init_dor");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_manual);
	tcase_add_test(tc_core, test_exhaustive);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}



/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_topology.c -- Unit tests for topology utility functions.
 */

#include <check.h>

#include "config.h"

#include "check_check.h"
#include "../src/spinn.h"
#include "../src/spinn_topology.h"

START_TEST (test_next_ccw)
{
	ck_assert_int_eq(spinn_next_ccw(SPINN_EAST),       SPINN_NORTH_EAST);
	ck_assert_int_eq(spinn_next_ccw(SPINN_NORTH_EAST), SPINN_NORTH);
	ck_assert_int_eq(spinn_next_ccw(SPINN_NORTH),      SPINN_WEST);
	ck_assert_int_eq(spinn_next_ccw(SPINN_WEST),       SPINN_SOUTH_WEST);
	ck_assert_int_eq(spinn_next_ccw(SPINN_SOUTH_WEST), SPINN_SOUTH);
	ck_assert_int_eq(spinn_next_ccw(SPINN_SOUTH),      SPINN_EAST);
}
END_TEST

START_TEST (test_next_cw)
{
	ck_assert_int_eq(spinn_next_cw(SPINN_EAST),       SPINN_SOUTH);
	ck_assert_int_eq(spinn_next_cw(SPINN_NORTH_EAST), SPINN_EAST);
	ck_assert_int_eq(spinn_next_cw(SPINN_NORTH),      SPINN_NORTH_EAST);
	ck_assert_int_eq(spinn_next_cw(SPINN_WEST),       SPINN_NORTH);
	ck_assert_int_eq(spinn_next_cw(SPINN_SOUTH_WEST), SPINN_WEST);
	ck_assert_int_eq(spinn_next_cw(SPINN_SOUTH),      SPINN_SOUTH_WEST);
}
END_TEST

START_TEST (test_opposite)
{
	ck_assert_int_eq(spinn_opposite(SPINN_EAST),       SPINN_WEST);
	ck_assert_int_eq(spinn_opposite(SPINN_NORTH_EAST), SPINN_SOUTH_WEST);
	ck_assert_int_eq(spinn_opposite(SPINN_NORTH),      SPINN_SOUTH);
	ck_assert_int_eq(spinn_opposite(SPINN_WEST),       SPINN_EAST);
	ck_assert_int_eq(spinn_opposite(SPINN_SOUTH_WEST), SPINN_NORTH_EAST);
	ck_assert_int_eq(spinn_opposite(SPINN_SOUTH),      SPINN_NORTH);
}
END_TEST

// Element-wise compare of a set of coords.
#define ck_assert_full_coord_eq(a,b) do { ck_assert_int_eq((a).x, (b).x); \
                                          ck_assert_int_eq((a).y, (b).y); \
                                          ck_assert_int_eq((a).z, (b).z); \
                                        } while (0)

// Check that a full coordinate minimised by spinn_full_coord_minimise equals
// the given value.
#define ck_assert_full_coord_minimise_eq(ax,ay,az, bx,by,bz) \
	do { \
		spinn_full_coord_t c = spinn_full_coord_minimise((spinn_full_coord_t){ax,ay,az}); \
		ck_assert_full_coord_eq(c, ((spinn_full_coord_t){bx,by,bz})); \
	} while (0)

START_TEST (test_full_coord_minimise)
{
	// Converts all-equal tuples to zeros
	ck_assert_full_coord_minimise_eq( 0, 0, 0,    0, 0, 0);
	ck_assert_full_coord_minimise_eq( 1, 1, 1,    0, 0, 0);
	ck_assert_full_coord_minimise_eq(-1,-1,-1,    0, 0, 0);
	
	// Leaves all one-non-zero values alone
	ck_assert_full_coord_minimise_eq(-1, 0, 0,   -1, 0, 0);
	ck_assert_full_coord_minimise_eq( 1, 0, 0,    1, 0, 0);
	ck_assert_full_coord_minimise_eq( 0,-1, 0,    0,-1, 0);
	ck_assert_full_coord_minimise_eq( 0, 1, 0,    0, 1, 0);
	ck_assert_full_coord_minimise_eq( 0, 0,-1,    0, 0,-1);
	ck_assert_full_coord_minimise_eq( 0, 0, 1,    0, 0, 1);
	
	// Pairs of values the same get minimised
	ck_assert_full_coord_minimise_eq(-1,-1, 0,    0, 0, 1);
	ck_assert_full_coord_minimise_eq( 1, 1, 0,    0, 0,-1);
	ck_assert_full_coord_minimise_eq(-1, 0,-1,    0, 1, 0);
	ck_assert_full_coord_minimise_eq( 1, 0, 1,    0,-1, 0);
	ck_assert_full_coord_minimise_eq( 0,-1,-1,    1, 0, 0);
	ck_assert_full_coord_minimise_eq( 0, 1, 1,   -1, 0, 0);
	
	// With no zeros things still get optimised
	ck_assert_full_coord_minimise_eq( 3, 2,  1,    1, 0,-1);
	ck_assert_full_coord_minimise_eq( 1, 2,  3,   -1, 0, 1);
	ck_assert_full_coord_minimise_eq( 1, 3,  2,   -1, 1, 0);
	ck_assert_full_coord_minimise_eq( 3, 1,  2,    1,-1, 0);
	ck_assert_full_coord_minimise_eq( 2, 3,  1,    0, 1,-1);
	ck_assert_full_coord_minimise_eq( 2, 1,  3,    0,-1, 1);
	ck_assert_full_coord_minimise_eq(-3,-2, -1,   -1, 0, 1);
	ck_assert_full_coord_minimise_eq(-1,-2, -3,    1, 0,-1);
	ck_assert_full_coord_minimise_eq(-1,-3, -2,    1,-1, 0);
	ck_assert_full_coord_minimise_eq(-3,-1, -2,   -1, 1, 0);
	ck_assert_full_coord_minimise_eq(-2,-3, -1,    0,-1, 1);
	ck_assert_full_coord_minimise_eq(-2,-1, -3,    0, 1,-1);
	
	// Already optimised things get left as they are
	ck_assert_full_coord_minimise_eq(-1, 0, 1,   -1, 0, 1);
	ck_assert_full_coord_minimise_eq( 1, 0,-1,    1, 0,-1);
	ck_assert_full_coord_minimise_eq( 0,-1, 1,    0,-1, 1);
	ck_assert_full_coord_minimise_eq( 0, 1,-1,    0, 1,-1);
	ck_assert_full_coord_minimise_eq(-1, 0, 1,   -1, 0, 1);
	ck_assert_full_coord_minimise_eq( 1, 0,-1,    1, 0,-1);
}
END_TEST

#define min(a,b) (((a)<(b)) ? (a) : (b))
#define max(a,b) (((a)>(b)) ? (a) : (b))

START_TEST (test_shortest_vector)
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
		for (int y1 = 0; y1 < test_sizes[i].y; y1++) {
			for (int x1 = 0; x1 < test_sizes[i].x; x1++) {
				for (int y2 = 0; y2 < test_sizes[i].y; y2++) {
					for (int x2 = 0; x2 < test_sizes[i].x; x2++) {
						// Figure out the vector
						spinn_full_coord_t c = spinn_shortest_vector((spinn_coord_t){x1, y1}, (spinn_coord_t){x2, y2}, test_sizes[i]);
						
						// Test that the distance matches the theoretical result by Xiao
						// in "Hexagonal and Pruned Torus Networks as Cayley Graphs" (2004)
						int l = test_sizes[i].x;
						int k = test_sizes[i].y;
						int a = ((x2-x1) + l) % l;
						int b = ((y2-y1) + k) % k;
						// distance((0,0), (a,b)) =
						int dist = min(min(min(max(a, b), max(l - a, k - b)), l - a + b), k + a - b);
						ck_assert_int_eq(spinn_magnitude(c), dist);
						
						// Test that the vector actually reaches the intended destination
						int x2_ = (((x1 + c.x) - c.z) + test_sizes[i].x) % test_sizes[i].x;
						int y2_ = (((y1 + c.y) - c.z) + test_sizes[i].y) % test_sizes[i].y;
						ck_assert_int_eq(x2, x2_);
						ck_assert_int_eq(y2, y2_);
					}
				}
			}
		}
	}
	
}
END_TEST

#define ck_assert_coord_eq(a,b) do { spinn_coord_t a_ = a; spinn_coord_t b_ = b;\
                                     ck_assert_int_eq((a_).x, (b_).x); \
                                     ck_assert_int_eq((a_).y, (b_).y); \
                                   } while (0)

START_TEST (test_dir_to_vector)
{
	ck_assert_coord_eq(spinn_dir_to_vector(SPINN_EAST),       ((spinn_coord_t){ 1, 0}));
	ck_assert_coord_eq(spinn_dir_to_vector(SPINN_NORTH_EAST), ((spinn_coord_t){ 1, 1}));
	ck_assert_coord_eq(spinn_dir_to_vector(SPINN_NORTH),      ((spinn_coord_t){ 0, 1}));
	ck_assert_coord_eq(spinn_dir_to_vector(SPINN_WEST),       ((spinn_coord_t){-1, 0}));
	ck_assert_coord_eq(spinn_dir_to_vector(SPINN_SOUTH_WEST), ((spinn_coord_t){-1,-1}));
	ck_assert_coord_eq(spinn_dir_to_vector(SPINN_SOUTH),      ((spinn_coord_t){ 0,-1}));
}
END_TEST


Suite *
make_spinn_topology_suite(void)
{
	Suite *s = suite_create("spinn_topology");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_next_ccw);
	tcase_add_test(tc_core, test_next_cw);
	tcase_add_test(tc_core, test_opposite);
	tcase_add_test(tc_core, test_full_coord_minimise);
	tcase_add_test(tc_core, test_shortest_vector);
	tcase_add_test(tc_core, test_dir_to_vector);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}



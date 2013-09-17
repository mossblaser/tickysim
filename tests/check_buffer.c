/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_buffer.c -- Unit test suite for basic TickySim buffer functionality.
 */

#include <check.h>

#include "config.h"

#include "check_check.h"
#include "../src/buffer.h"

START_TEST (test_buffer_push_pop)
{
	// A set of chars which can be pointed at
	char *pointables = "ABCD";
	
	// Length of the buffer to use in tests
	const int buf_len = strlen(pointables);
	
	// Create a buffer exactly long enough to fit pointers to all the pointables
	// in.
	buffer_t *b = buffer_create(buf_len);
	
	// Attempt to fill/empty the buffer twice to make sure that when the pointers
	// wrap nothing goes wrong.
	for (int j = 0; j < 2; j++) {
		// Initially the buffer is empty
		ck_assert(buffer_is_empty(b));
		ck_assert(!buffer_is_full(b));
		
		// Fill the buffer up
		for (int i = 0; i < buf_len; i++) {
			buffer_push(b, (void *)(pointables + i));
			// Until the last element is inserted the list should not be full
			if (i < buf_len-1) {
				ck_assert(!buffer_is_empty(b));
				ck_assert(!buffer_is_full(b));
			} else {
				ck_assert(!buffer_is_empty(b));
				ck_assert(buffer_is_full(b));
			}
		}
		
		// Pop a few items
		for (int i = 0; i < buf_len; i++) {
			char *c_peeked = buffer_peek(b);
			
			char *c = buffer_pop(b);
			
			// Check the value popped was the one put in...
			ck_assert_int_eq((int)*c,        (int)pointables[i]);
			ck_assert_int_eq((int)*c_peeked, (int)pointables[i]);
			
			// Until the last element is popped the list should not be empty
			if (i < buf_len-1) {
				ck_assert(!buffer_is_empty(b));
				ck_assert(!buffer_is_full(b));
			} else {
				ck_assert(buffer_is_empty(b));
				ck_assert(!buffer_is_full(b));
			}
		}
	}
	
	buffer_free(b);
}
END_TEST


Suite *
make_buffer_suite(void)
{
	Suite *s = suite_create("buffer");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_buffer_push_pop);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}

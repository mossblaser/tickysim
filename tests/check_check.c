/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_tickysim.c -- Unit tests for basic TickySim functionality using the
 * "Check" library.
 */

#include <check.h>

#include "config.h"
#include "check_check.h"

int
main(int argc, char *argv[])
{
	int number_failed;
	
	// We need a single master suite from which the first suite runner is created.
	Suite *master_suite = suite_create("master");
	SRunner *sr = srunner_create(master_suite);
	
	// Add all suites
	srunner_add_suite(sr, make_arbiter_suite());
	srunner_add_suite(sr, make_buffer_suite());
	srunner_add_suite(sr, make_scheduler_suite());
	
	// Run the tests
	srunner_run_all(sr, CK_NORMAL);
	
	// Collect results
	number_failed = srunner_ntests_failed(sr);
	
	srunner_free(sr);
	
	return (number_failed == 0) ? 0 : -1;
}


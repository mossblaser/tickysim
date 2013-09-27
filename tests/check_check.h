/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_check.h -- Header file defining all test-suite creation functions.
 */

#ifndef CHECK_CHECK_H
#define CHECK_CHECK_H

#include <check.h>

#include "config.h"


Suite *make_arbiter_suite(void);
Suite *make_buffer_suite(void);
Suite *make_scheduler_suite(void);
Suite *make_delay_suite(void);
Suite *make_spinn_topology_suite(void);
Suite *make_spinn_router_suite(void);
Suite *make_spinn_packet_init_dor(void);
Suite *make_spinn_packet_pool_suite(void);
Suite *make_spinn_packet_gen_suite(void);
Suite *make_spinn_packet_con_suite(void);

#endif

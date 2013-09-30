/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_main.c -- A SpiNNaker simulator implemented using TickySim.
 */

#include "config.h"

#include <stdio.h>

#include "spinn_sim.h"



/******************************************************************************
 * World starts here
 ******************************************************************************/

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s config_file\n", argv[0]);
		return -1;
	}
	
	spinn_sim_t sim;
	spinn_sim_init(&sim, argv[1]);
	spinn_sim_run(&sim);
	spinn_sim_destroy(&sim);
}

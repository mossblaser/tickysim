/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_topology.h -- Utility functions for dealing with the SpiNNaker topology.
 */

#ifndef SPINN_TOPOLOGY_H
#define SPINN_TOPOLOGY_H

#include "config.h"

#include "spinn.h"


/**
 * Returns the next direction counter-clockwise from the given direction. Result
 * is undefined for the 'local direction'.
 */
#define spinn_next_ccw(direction) (((direction)+1)%6)


/**
 * Returns the next direction clockwise from the given direction. Result is
 * undefined for the 'local direction'.
 */
#define spinn_next_cw(direction) (((direction)+6-1)%6)


/**
 * Returns the direction opposite the given direction. Result is undefined for
 * the 'local direction'.
 */
#define spinn_opposite(direction) (((direction)+3)%6)


/**
 * Convert a spinn_full_coord_t into the unique minimal-magnitude form.
 */
spinn_full_coord_t spinn_full_coord_minimise(spinn_full_coord_t coord);

#endif



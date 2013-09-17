/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn.h -- General definitions for SpiNNaker simulation entities.
 */

#ifndef SPINN_H
#define SPINN_H

/**
 * A coordinate of a chip in a SpiNNaker system with each vector being as in the
 * following:
 *
 *      | y
 *      |
 *     / \
 *  z /   \ x
 */
typedef struct spinn_coord {
	int x;
	int y;
	int z;
} spinn_coord_t;

#endif


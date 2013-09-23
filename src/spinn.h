/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn.h -- General definitions for SpiNNaker simulation entities.
 */

#ifndef SPINN_H
#define SPINN_H

#include "config.h"

/**
 * A coordinate of a chip in a SpiNNaker system with each vector being as in the
 * following:
 *
 *  y \
 *     \___ x
 *     /
 *  z /
 *
 * The 'z' vector is not given as it is non-orthogonal and is assumed to be set
 * to 0.
 */
typedef struct spinn_coord {
	int x;
	int y;
} spinn_coord_t;

/**
 * As with spinn_coord_t but including a specification of the z vector.
 */
typedef struct spinn_full_coord {
	int x;
	int y;
	int z;
} spinn_full_coord_t;


/**
 * Definitions of directions in a SpiNNaker system, in particular, this
 * typically reffers to the 6 directional connections to a chip plus the local
 * connection to the chip's cores.
 *    N
 *    __
 *NW /L \ NE
 *SW \__/ SE
 *    S
 */
typedef enum spinn_direction {
	SPINN_EAST       = 0,
	SPINN_NORTH_EAST = 1,
	SPINN_NORTH      = 2,
	SPINN_WEST       = 3,
	SPINN_SOUTH_WEST = 4,
	SPINN_SOUTH      = 5,
	SPINN_LOCAL      = 6,
} spinn_direction_t;

/**
 * Packet emergency routing state. Evaluates to "true" iff being emergency
 * routed.
 */
typedef enum spinn_emg_state {
	SPINN_EMG_NORMAL = 0,
	SPINN_EMG_FIRST_LEG,
	SPINN_EMG_SECOND_LEG,
} spinn_emg_state_t;


#endif


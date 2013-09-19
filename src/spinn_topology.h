/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_topology.h -- Utility functions for dealing with the SpiNNaker topology.
 */

#ifndef SPINN_TOPOLOGY_H
#define SPINN_TOPOLOGY_H

#include <stdlib.h>

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
 * Convert a normal coordinate to a full coordinate.
 */
#define spinn_coord_to_full_coord(c) ((spinn_full_coord_t){(c).x,(c).y,0})


/**
 * Convert a full coordinate to a normal (2D) coordinate.
 */
#define spinn_full_coord_to_coord(c) ((spinn_coord_t){ (c).x - (c).z \
                                                     , (c).y - (c).z \
                                                     })


/**
 * Get the distance represented by a full coordinate. If the coordinate is not
 * minimised then the distance will also not be minimal.
 */
#define spinn_magnitude(c) (abs((c).x) + abs((c).y) + abs((c).z))


/**
 * Convert a spinn_full_coord_t into the unique minimal-magnitude form.
 */
spinn_full_coord_t spinn_full_coord_minimise(spinn_full_coord_t coord);

/**
 * Find the shortest path vector between a source and target coordinate in a
 * SpiNNaker-style torus of th given size.
 */
spinn_full_coord_t spinn_shortest_vector( spinn_coord_t source
                                        , spinn_coord_t target
                                        , spinn_coord_t system_size
                                        );

#endif



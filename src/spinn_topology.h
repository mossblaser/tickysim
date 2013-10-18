/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_topology.h -- Utility functions for dealing with the SpiNNaker topology.
 */

#ifndef SPINN_TOPOLOGY_H
#define SPINN_TOPOLOGY_H

#include <stdlib.h>
#include <stdbool.h>

#include "config.h"

#include "spinn.h"

/**
 * State of the spinn_hexagon() generator function. Must be initialised using
 * spinn_hexagon_init().
 */
typedef struct spinn_hexagon_state spinn_hexagon_state_t;


// Concrete definitions of the above types
#include "spinn_topology_internal.h"


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
 * Get the spinn_coord_t which corresponds to each direction vector.
 */
#define spinn_dir_to_vector(d)                      \
	(((spinn_coord_t[]){                              \
		(spinn_coord_t){ 1, 0},/* SPINN_EAST */         \
		(spinn_coord_t){ 1, 1},/* SPINN_NORTH_EAST */   \
		(spinn_coord_t){ 0, 1},/* SPINN_NORTH */        \
		(spinn_coord_t){-1, 0},/* SPINN_WEST */         \
		(spinn_coord_t){-1,-1},/* SPINN_SOUTH_WEST */   \
		(spinn_coord_t){ 0,-1},/* SPINN_SOUTH */        \
	})[(d)])


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

/**
 * Initialise the data structure used by spinn_hexagon() to construct a hexagon
 * of the given number of layers. For example, the figure below shows what layer
 * each node is on.
 *
 *         3---3---3
 *        / \ / \ / \
 *       3---2---2---3
 *      / \ / \ / \ / \
 *     3---2---1---2---3
 *    / \ / \ / \ / \ / \
 *   3---2---1---1---2---3
 *    \ / \ / \ / \ / \ /
 *     3---2---2---2---3
 *      \ / \ / \ / \ /
 *       3---3---3---3
 */
void spinn_hexagon_init(spinn_hexagon_state_t *h, int num_layers);


/**
 * generate the coordinates of positions on a hexagonal arrangement as used on
 * spinnaker boards. sets the position in the spinn_coord_t indicated and
 * returns false when no more positions exist.
 *
 * Coordinates will be positive with x and y ranging from 0 to (2*num_layers_)-1
 * inclusive. The value of position when spinn_hexagon_init returns 0 is
 * undefined.
 */
bool spinn_hexagon(spinn_hexagon_state_t *h, spinn_coord_t *position);

#endif



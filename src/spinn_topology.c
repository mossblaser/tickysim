/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_topology.c -- Utility functions for dealing with the SpiNNaker topology.
 */

#include <stdlib.h>

#include "config.h"

#include "spinn.h"
#include "spinn_topology.h"


/******************************************************************************
 * Internal functions.
 ******************************************************************************/

// Swap a pair of variables of the same type.
#define SWAP(a,b) do { (a)^=(b); (b)^=(a); (a)^=(b); } while(0)

/**
 * Find the median element (by sorting with a sorting network, see:
 * http://jgamble.ripco.net/cgi-bin/nw.cgi?inputs=3&algorithm=best&output=text)
 */
static int
spinn_full_coord_median(spinn_full_coord_t coord)
{
	// Make a copy which we can shuffle around
	spinn_full_coord_t c = coord;
	
	// Sort assuming x,y,z in ascending order.
	if (c.z < c.y) { SWAP(c.z, c.y); }
	if (c.z < c.x) { SWAP(c.z, c.x); }
	if (c.y < c.x) { SWAP(c.y, c.x); }
	
	return c.y;
}


/******************************************************************************
 * Public functions.
 ******************************************************************************/

spinn_full_coord_t
spinn_full_coord_minimise(spinn_full_coord_t coord)
{
	// Subtract the median term from each value to yield a vector with at least
	// one zero term and the other two values of opposite sign (or also zero).
	int median = spinn_full_coord_median(coord);
	
	return (spinn_full_coord_t){ coord.x - median
	                           , coord.y - median
	                           , coord.z - median
	                           };
}


spinn_full_coord_t
spinn_shortest_vector( spinn_coord_t s
                     , spinn_coord_t t
                     , spinn_coord_t system_size
                     )
{
	// A terrible hack (inherited from gollywhomper). Re-center the world either
	// so the source is in the bottom left corner, the center or the top/right of
	// the system. For each, compute the shortest vector in the same way you would
	// for a normal grid. Pick the one of these which wins.
	spinn_coord_t centers[] = {
		{0,               0},
		{system_size.x/2, system_size.y/2},
		{system_size.x-1, system_size.y-1},
	};
	int num_centers = sizeof(centers)/sizeof(spinn_coord_t);
	
	spinn_full_coord_t best_path;
	
	for (int i = 0; i < num_centers; i++) {
		// Re-center the source/target
		spinn_full_coord_t rc_s = {centers[i].x, centers[i].y, 0};
		spinn_full_coord_t rc_t = { ((t.x - s.x) + centers[i].x + system_size.x) % system_size.x
		                          , ((t.y - s.y) + centers[i].y + system_size.y) % system_size.y
		                          , 0
		                          };
		spinn_full_coord_t path = spinn_full_coord_minimise((spinn_full_coord_t){
			rc_t.x - rc_s.x,
			rc_t.y - rc_s.y,
			rc_t.z - rc_s.z
		});
		
		if (i == 0 || spinn_magnitude(path) < spinn_magnitude(best_path)) {
			best_path = path;
		}
	}
	
	return best_path;
}

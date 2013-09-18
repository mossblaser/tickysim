/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_topology.c -- Utility functions for dealing with the SpiNNaker topology.
 */


#include "config.h"

#include "spinn.h"
#include "spinn_topology.h"


/******************************************************************************
 * Internal functions.
 ******************************************************************************/

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

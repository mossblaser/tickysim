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



#define TRY_VECT(vect) do { if (spinn_magnitude((vect)) < spinn_magnitude(best_vect)) { \
                              best_vect = vect; \
                            } \
                          } while (0)

spinn_full_coord_t
spinn_shortest_vector( spinn_coord_t s
                     , spinn_coord_t t
                     , spinn_coord_t system_size
                     )
{
	// Implementation of routing as used in INSEE's SpiNNaker model. Provides
	// better balanced utilisation of links than tickysim. Simply try every
	// possible direction and try the best one found.
	
	// Special case if path to self.
	if (s.x == t.x && s.y == t.y)
		return ((spinn_full_coord_t){0,0,0});
	
	// The distances between s and t for non-wrapping and always wrapping routes
	// for x and y axes respectively.
	int dx_nw = t.x - s.x;
	int dy_nw = t.y - s.y;
	int dx_aw = (dx_nw > 0) ? (dx_nw - system_size.x) : (dx_nw + system_size.x);
	int dy_aw = (dy_nw > 0) ? (dy_nw - system_size.y) : (dy_nw + system_size.y);
	
	// Try the non-wrapping possibilities using only x,z, y,z and x,y
	spinn_full_coord_t best_vect = (spinn_full_coord_t){dx_nw - dy_nw, 0, -dy_nw};
	TRY_VECT(                     ((spinn_full_coord_t){0, dy_nw - dx_nw, -dx_nw}));
	TRY_VECT(                     ((spinn_full_coord_t){dx_nw, dy_nw, 0}));
	
	// Try the x-non-wrapping, y-wrapping possibilities using only x,z, y,z and x,y
	TRY_VECT(                     ((spinn_full_coord_t){dx_nw - dy_aw, 0, -dy_aw}));
	TRY_VECT(                     ((spinn_full_coord_t){0, dy_aw - dx_nw, -dx_nw}));
	TRY_VECT(                     ((spinn_full_coord_t){dx_nw, dy_aw, 0}));
	
	// Try the x-wrapping, y-non-wrapping possibilities using only x,z, y,z and x,y
	TRY_VECT(                     ((spinn_full_coord_t){dx_aw - dy_nw, 0, -dy_nw}));
	TRY_VECT(                     ((spinn_full_coord_t){0, dy_nw - dx_aw, -dx_aw}));
	TRY_VECT(                     ((spinn_full_coord_t){dx_aw, dy_nw, 0}));
	
	// Try the x-wrapping, y-wrapping possibilities using only x,z, y,z and x,y
	TRY_VECT(                     ((spinn_full_coord_t){dx_aw - dy_aw, 0, -dy_aw}));
	TRY_VECT(                     ((spinn_full_coord_t){0, dy_aw - dx_aw, -dx_aw}));
	TRY_VECT(                     ((spinn_full_coord_t){dx_aw, dy_aw, 0}));
	
	return best_vect;
}


void
spinn_hexagon_init(spinn_hexagon_state_t *h, int num_layers)
{
	h->num_layers = num_layers;
	h->layer = 0;
	h->edge = 0;
	h->i=0;
	
	// Start at the location of the first chip in the bottom-right of the central
	// layer of the hexagon
	h->pos.x = num_layers;
	h->pos.y = num_layers-1;
}

// For use in spinn_hexagon. If still working on nodes along the given edge,
// progress to the next edge, otherwise fall-through onto the next edge.
#define SPINN_HEXAGON_CASE(edge_num, edge_length, dx,dy) \
	case (edge_num): \
		if (h->i < (edge_length)) { \
			h->pos.x += (dx); \
			h->pos.y += (dy); \
			h->i++; \
			return true; \
		} else { \
			h->i = 0; \
			h->edge++; \
		} /* Fall through to next edge. */

bool
spinn_hexagon(spinn_hexagon_state_t *h, spinn_coord_t *position)
{
	// Set the output position
	position->x = h->pos.x;
	position->y = h->pos.y;
	
	while (h->layer < h->num_layers) {
		switch(h->edge) {
			//                 Edge, Length,     dx, dy
			SPINN_HEXAGON_CASE(0,    h->layer,    0, -1)
			SPINN_HEXAGON_CASE(1,    h->layer,   -1, -1)
			SPINN_HEXAGON_CASE(2,    h->layer+1, -1,  0)
			SPINN_HEXAGON_CASE(3,    h->layer,    0,  1)
			SPINN_HEXAGON_CASE(4,    h->layer+1,  1,  1)
			SPINN_HEXAGON_CASE(5,    h->layer+1,  1,  0)
			
			// In the default case where we're actually not on any edge, move onto the
			// next layer (the outer while loop will cause things to progress nicely)
			default:
				h->edge = 0;
				h->layer++;
				break;
		}
	}
	
	// We've reached the outside of the hexagon, stop iterating
	return false;
}

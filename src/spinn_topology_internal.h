/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_topology_internal.h -- Concrete definitions of internal datastrucutres.
 * This is provided to allow the creation of these types. Users should not
 * access the fields directly. This file should only be included by
 * spinn_topology.h
 */


/**
 * State of the spinn_hexagon() generator function.
 */
struct spinn_hexagon_state {
	// The number of layers in the hexagon to be drawn
	int num_layers;
	
	// The current layer
	int layer;
	
	// The edge whose nodes are currently being given
	int edge;
	
	// The index along the specified edge
	int i;
	
	// The position of the last node generated
	spinn_coord_t pos;
};

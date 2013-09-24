/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_router_internal.h -- Concrete definitions of internal datastrucutres.
 * This is provided to allow the creation of these types. Users should not
 * access the fields directly. This file should only be included by
 * spinn_router.h
 */


/**
 * The structure representing a particular router. 
 */
struct spinn_router {
	// Ports (expected to supply/accept spinn_packet_t pointers.
	buffer_t *input;
	buffer_t *outputs[7];
	
	// Location of the router in the system
	spinn_coord_t position;
	
	// Enable emergency routing (rather than just dropping out after
	// first_timeout.
	bool use_emg_routing;
	
	// Timeouts (measured in router periods)
	int first_timeout;
	int final_timeout;
	
	// Packet-forwarded callback
	void (*on_forward)( spinn_router_t    *router
	                  , spinn_packet_t    *packet
	                  , void              *data
	                  );
	void *on_forward_data;
	
	// Packet-droping callback
	void (*on_drop)( spinn_router_t *router
	               , spinn_packet_t *packet
	               , void           *data
	               );
	void *on_drop_data;
	
	// Number of cycles the current packet at the head of the input buffer has
	// been waiting to be routed (in router cycles).
	int time_elapsed;
	
	// The direction the next packet forwarded is being sent in (for the setting
	// of the flag in the packet.
	spinn_direction_t selected_output_direction;
	
	// The emergency routing state to be assigned to the packet when it is
	// forwarded
	spinn_emg_state_t cur_packet_emg_state;
	
	// Should the currrent packet be forwarded in the next tock?
	bool forward_packet;
	
	// Should the currrent packet should be dropped in the next tock?
	bool drop_packet;
};



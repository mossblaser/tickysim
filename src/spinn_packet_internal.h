/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_packet_internal.h -- Concrete definitions of internal datastrucutres.
 * This is provided to allow the creation of these types. Users should not
 * access the fields directly. This file should only be included by
 * spinn_packet.h
 */


/**
 * A linked list of pools of packets.
 */
typedef struct spinn_packet_sub_pool {
	spinn_packet_t               *packets;
	struct spinn_packet_sub_pool *next;
} spinn_packet_sub_pool_t;


struct spinn_packet_pool {
	// A linked list of pointers to arrays of packets. Only used to free all
	// memory.
	spinn_packet_sub_pool_t *sub_pools;
	
	// A stack of pointers to free packets.
	spinn_packet_t **free_packets;
	
	// A pointer to top-most packet pointer in the free packet stack
	spinn_packet_t **free_packets_head;
	
	// The size of the free packet stack (also the total number of packets in the pool)
	size_t num_packets;
};


typedef enum spinn_packet_gen_spatial_dist {
	SPINN_GS_DIST_CYCLIC,
	SPINN_GS_DIST_UNIFORM,
	SPINN_GS_DIST_P2P,
} spinn_packet_gen_spatial_dist_t;



typedef enum spinn_packet_gen_temporal_dist {
	SPINN_GT_DIST_BERNOULLI,
	SPINN_GT_DIST_PERIODIC,
} spinn_packet_gen_temporal_dist_t;


struct spinn_packet_gen {
	// The scheduler which drives the packet generator 
	scheduler_t *scheduler;
	
	// The buffer into which packets will be generated
	buffer_t *buffer;
	
	// Pool of packets to send
	spinn_packet_pool_t *pool;
	
	// Where will the packets be inserted
	spinn_coord_t position;
	spinn_coord_t system_size;
	
	// Should wrap-around links be used?
	bool use_wrap_around_links;
	
	// Should a packet be sent during the tock phase?
	bool send_packet;
	
	// Is the output buffer full (i.e. should sending a packet fail?)?
	bool output_blocked;
	
	// Callback to filter packet destinations
	bool (*dest_filter)(const spinn_coord_t *proposed_destination, void *data);
	void *dest_filter_data;
	
	// The spatial distribution to use when generating packets.
	spinn_packet_gen_spatial_dist_t  spatial_dist;
	
	// State data used by the various packet generation schemes
	// Spatial distribution data
	union {
		
		// Uniform packet generator data
		// *No state required*
		
		// Cyclic packet generator data
		struct {
			spinn_coord_t next_dest;
		} cyclic;
		
		// P2P packet generator data
		struct {
			spinn_coord_t target;
		} p2p;
		
	} spatial_dist_data;
	
	// The temporal distribution to use when generating packets.
	spinn_packet_gen_temporal_dist_t temporal_dist;
	
	// Temporal distribution data
	union {
		
		// Bernoulli distribution
		struct {
			double prob;
		} bernoulli;
		
		// Periodic distribution
		struct {
			int interval;
			int time_elapsed;
		} periodic;
		
	} temporal_dist_data;
	
	// Callback on packet create/send
	void *(*on_packet_gen)(spinn_packet_t *packet, void *data);
	void *on_packet_gen_data;
};



typedef enum spinn_packet_con_temporal_dist {
	SPINN_CT_DIST_BERNOULLI,
	SPINN_CT_DIST_PERIODIC,
} spinn_packet_con_temporal_dist_t;


struct spinn_packet_con {
	// The buffer from which packets will be consumed
	buffer_t *buffer;
	
	// Pool of packets to send
	spinn_packet_pool_t *pool;
	
	// Should a packet be consumed during the tock phase?
	bool consume_packet;
	
	// The temporal distribution to use when generating packets.
	spinn_packet_con_temporal_dist_t temporal_dist;
	
	// Temporal distribution data
	union {
		
		// Bernoulli distribution
		struct {
			double prob;
		} bernoulli;
		
		// Periodic distribution
		struct {
			int interval;
			int time_elapsed;
		} periodic;
		
	} temporal_dist_data;
	
	// Callback on packet consumption
	void (*on_packet_con)(spinn_packet_t *packet, void *data);
	void *on_packet_con_data;
};

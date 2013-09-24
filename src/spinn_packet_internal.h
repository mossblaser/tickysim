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


typedef enum spinn_packet_gen_dist {
	SPINN_DIST_CYCLIC,
	SPINN_DIST_UNIFORM,
} spinn_packet_gen_dist_t;


struct spinn_packet_gen {
	// The buffer into which packets will be generated
	buffer_t *buffer;
	
	// Pool of packets to send
	spinn_packet_pool_t *pool;
	
	// Where will the packets be inserted
	spinn_coord_t position;
	spinn_coord_t system_size;
	
	// How frequently will packets be produced
	double bernoulli_prob;
	
	// Should a packet be sent during the tock phase?
	bool send_packet;
	
	// The distribution to use when generating packets.
	spinn_packet_gen_dist_t dist;
	
	// State data used by the various packet generation schemes
	union {
		
		// Cyclic packet generator data
		struct {
			spinn_coord_t next_dest;
		} cyclic;
		
	} dist_data;
	
	// Callback on packet create/send
	void *(*on_packet_gen)(spinn_packet_t *packet, void *data);
	void *on_packet_gen_data;
};

struct spinn_packet_con {
	// The buffer from which packets will be consumed
	buffer_t *buffer;
	
	// Pool of packets to send
	spinn_packet_pool_t *pool;
	
	// How frequently will packets be consumed
	double bernoulli_prob;
	
	// Should a packet be consumed during the tock phase?
	bool consume_packet;
	
	// Callback on packet consumption
	void (*on_packet_con)(spinn_packet_t *packet, void *data);
	void *on_packet_con_data;
};

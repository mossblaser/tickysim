/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_packet.h -- A SpiNNaker packet and packet generator/consumer models.
 */


#include "config.h"

#include <assert.h>
#include <stdbool.h>

#include "scheduler.h"
#include "buffer.h"

#include "spinn.h"
#include "spinn_packet.h"
#include "spinn_topology.h"

/******************************************************************************
 * Internal datastructures.
 ******************************************************************************/

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
};

/******************************************************************************
 * Public Utility Functions
 ******************************************************************************/

void
spinn_packet_init_dor( spinn_packet_t *p
                     , spinn_coord_t   source
                     , spinn_coord_t   destination
                     , spinn_coord_t   system_size
                     , void           *payload
                     )
{
	// Set the trivial fields
	p->destination = destination;
	p->emg_state   = SPINN_EMG_NORMAL;
	p->payload     = payload;
	
	// Find the path between the src/dest
	spinn_full_coord_t v = spinn_shortest_vector(source, destination, system_size);
	
	// The starting direction is simply the direction the vector is pointing
	     if (v.x < 0) p->direction = SPINN_WEST;
	else if (v.x > 0) p->direction = SPINN_EAST;
	else if (v.y < 0) p->direction = SPINN_SOUTH;
	else if (v.y > 0) p->direction = SPINN_NORTH;
	else if (v.z < 0) p->direction = SPINN_NORTH_EAST;
	else if (v.z > 0) p->direction = SPINN_SOUTH_WEST;
	else              p->direction = SPINN_LOCAL;
	
	
	// Find out on which axis the inflection point is along
	if (v.x != 0) {
		p->inflection_point.x = ((source.x + v.x) + system_size.x) % system_size.x;
		p->inflection_point.y = source.y;
		     if (v.y < 0) p->inflection_direction = SPINN_SOUTH;
		else if (v.y > 0) p->inflection_direction = SPINN_NORTH;
		else if (v.z < 0) p->inflection_direction = SPINN_NORTH_EAST;
		else if (v.z > 0) p->inflection_direction = SPINN_SOUTH_WEST;
		else              p->inflection_direction = SPINN_LOCAL;
	
	} else if (v.y != 0) {
		p->inflection_point.x = source.x;
		p->inflection_point.y = ((source.y + v.y) + system_size.y) % system_size.y;
		     if (v.z < 0) p->inflection_direction = SPINN_NORTH_EAST;
		else if (v.z > 0) p->inflection_direction = SPINN_SOUTH_WEST;
		else              p->inflection_direction = SPINN_LOCAL;
	
	} else {
		p->inflection_point     = destination;
		p->inflection_direction = SPINN_LOCAL;
	}
}

/******************************************************************************
 * Packet Pool
 ******************************************************************************/

spinn_packet_pool_t *
spinn_packet_pool_create(void)
{
	spinn_packet_pool_t *pool = malloc(sizeof(spinn_packet_pool_t));
	assert(pool != NULL);
	
	// Initially start with an empty pool
	pool->sub_pools = NULL;
	
	// Create an empty initial stack
	pool->free_packets = NULL;
	pool->free_packets_head = NULL;
	
	pool->num_packets = 0;
	
	return pool;
}


void
spinn_packet_pool_free(spinn_packet_pool_t *pool)
{
	// Free the sub pools
	spinn_packet_sub_pool_t *sub_pool = pool->sub_pools;
	while (sub_pool) {
		spinn_packet_sub_pool_t *next_sub_pool = sub_pool->next;
		free(sub_pool->packets);
		free(sub_pool);
		sub_pool = next_sub_pool;
	}
	
	// Free the free packet stack
	if (pool->free_packets != NULL)
		free(pool->free_packets);
	
	// Finally, free the struct
	free(pool);
}


spinn_packet_t *
spinn_packet_pool_palloc(spinn_packet_pool_t *pool)
{
	
	// If the free packet stack is empty, create some more packets (double+1 the
	// current number of packets)
	if (pool->free_packets_head == NULL || pool->free_packets_head < pool->free_packets) {
		spinn_packet_sub_pool_t *next_sub_pool = pool->sub_pools;
		pool->sub_pools = malloc(sizeof(spinn_packet_sub_pool_t));
		assert(pool->sub_pools != NULL);
		pool->sub_pools->packets = calloc(pool->num_packets + 1, sizeof(spinn_packet_t));
		assert(pool->sub_pools->packets != NULL);
		pool->sub_pools->next = next_sub_pool;
		
		// Free the old free packet stack
		if (pool->free_packets != NULL)
			free(pool->free_packets);
		
		// Create a new, larger stack and add the new packets to the stack
		pool->free_packets = calloc(pool->num_packets*2 + 1, sizeof(spinn_packet_t *));
		assert(pool->free_packets != NULL);
		pool->free_packets_head = pool->free_packets + pool->num_packets + 1 - 1;
		for (size_t i = 0; i < pool->num_packets + 1; i++)
			pool->free_packets[i] = &pool->sub_pools->packets[i];
		
		pool->num_packets = pool->num_packets*2 + 1;
	}
	
	// Return a packet from the free-packet stack
	return *(pool->free_packets_head--);
}


void
spinn_packet_pool_pfree( spinn_packet_pool_t *pool
                       , spinn_packet_t      *packet
                       )
{
	*(++pool->free_packets_head) = packet;
}



/******************************************************************************
 * Packet generators
 ******************************************************************************/

/**
 * Tock function which decides whether to send a packet (based first on the
 * availability of space in the output buffer and then the Bernoulli test.
 */
void
spinn_packet_gen_tick(void *g_)
{
	spinn_packet_gen_t *g = (spinn_packet_gen_t *)g;
	
	// XXX: TODO: Bernoulli test
	g->send_packet = !buffer_is_full(g->buffer);
}

/**
 * Tock function which decides whether to send a packet (based first on the
 * availability of space in the output buffer and then the Bernoulli test.
 */
void
spinn_packet_gen_tock(void *g_)
{
	spinn_packet_gen_t *g = (spinn_packet_gen_t *)g;
	
	// XXX: TODO: make and send a packet
}

/**
 * Internal function. Do the basic initialisation common to all packet
 * generators.
 */
spinn_packet_gen_t *
spinn_packet_gen_create_base( scheduler_t             *s
                            , buffer_t                *b
                            , spinn_packet_pool_t     *pool
                            , spinn_coord_t            position
                            , spinn_coord_t            system_size
                            , ticks_t                  period
                            , double                   bernoulli_prob
                            , spinn_packet_gen_dist_t  dist
                            )
{
	spinn_packet_gen_t *g = malloc(sizeof(spinn_packet_gen_t));
	assert(g != NULL);
	
	// Set up data-structure fields
	g->buffer         = b;
	g->pool           = pool;
	g->position       = position;
	g->system_size    = system_size;
	g->bernoulli_prob = bernoulli_prob;
	g->dist           = dist;
	
	// Set up tick/tock functions
	scheduler_schedule( s, period
	                  , spinn_packet_gen_tick, (void *)g
	                  , spinn_packet_gen_tock, (void *)g
	                  );
	
	return g;
}

spinn_packet_gen_t *
spinn_packet_gen_cyclic_create( scheduler_t         *s
                              , buffer_t            *b
                              , spinn_packet_pool_t *pool
                              , spinn_coord_t        position
                              , spinn_coord_t        system_size
                              , ticks_t              period
                              , double               bernoulli_prob
                              )
{
	spinn_packet_gen_t *g = spinn_packet_gen_create_base(
		s,b,pool,position,system_size, period, bernoulli_prob,
		SPINN_DIST_CYCLIC);
	
	// Start the cyclic message sending from the current position
	g->dist_data.cyclic.next_dest = position;
	
	return g;
}


spinn_packet_gen_t *
spinn_packet_gen_uniform_create( scheduler_t         *s
                               , buffer_t            *b
                               , spinn_packet_pool_t *pool
                               , spinn_coord_t        position
                               , spinn_coord_t        system_size
                               , ticks_t              period
                               , double               bernoulli_prob
                               )
{
	spinn_packet_gen_t *g = spinn_packet_gen_create_base(
		s,b,pool,position,system_size, period, bernoulli_prob,
		SPINN_DIST_UNIFORM);
	
	return g;
}

void
spinn_packet_gen_free(spinn_packet_gen_t *g)
{
	free(g);
}

/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_packet.h -- A SpiNNaker packet and packet generator/consumer models.
 */


#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "scheduler.h"
#include "buffer.h"

#include "spinn.h"
#include "spinn_packet.h"
#include "spinn_topology.h"


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

void
spinn_packet_pool_init(spinn_packet_pool_t *pool)
{
	// Initially start with an empty pool
	pool->sub_pools = NULL;
	
	// Create an empty initial stack
	pool->free_packets = NULL;
	pool->free_packets_head = NULL;
	
	pool->num_packets = 0;
}


void
spinn_packet_pool_destroy(spinn_packet_pool_t *pool)
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
 * Tick function which decides whether to send a packet (based on the
 * availability of space in the output buffer and then the Bernoulli trial.
 */
void
spinn_packet_gen_tick(void *g_)
{
	spinn_packet_gen_t *g = (spinn_packet_gen_t *)g_;
	
	g->send_packet = !buffer_is_full(g->buffer)
	              && (((double)rand())/((double)RAND_MAX+1.0)) <= g->bernoulli_prob;
}

/**
 * Tock function actually generate and send a packet if required.
 */
void
spinn_packet_gen_tock(void *g_)
{
	spinn_packet_gen_t *g = (spinn_packet_gen_t *)g_;
	
	// Do nothing if no packet due to be sent
	if (!g->send_packet)
		return;
	
	// Determine the packet destination based on the current distribution
	spinn_coord_t destination;
	switch (g->dist) {
		case SPINN_DIST_CYCLIC:
			destination = g->dist_data.cyclic.next_dest;
			
			// Move to the next node
			g->dist_data.cyclic.next_dest.x ++;
			if (g->dist_data.cyclic.next_dest.x >= g->system_size.x) {
				g->dist_data.cyclic.next_dest.x = 0;
				g->dist_data.cyclic.next_dest.y ++;
				if (g->dist_data.cyclic.next_dest.y >= g->system_size.y) {
					g->dist_data.cyclic.next_dest.y = 0;
				}
			}
			break;
		
		default:
		case SPINN_DIST_UNIFORM:
			destination.x = (int)((((double)rand())/((double)RAND_MAX+1.0)) * g->system_size.x);
			destination.y = (int)((((double)rand())/((double)RAND_MAX+1.0)) * g->system_size.y);
			break;
	}
	
	// Produce the packet
	spinn_packet_t *p = spinn_packet_pool_palloc(g->pool);
	spinn_packet_init_dor(p, g->position, destination, g->system_size, NULL);
	
	// Set up the payload and run the callback
	if (g->on_packet_gen)
		p->payload = g->on_packet_gen(p, g->on_packet_gen_data);
	
	// Send the packet
	buffer_push(g->buffer, (void *)p);
	
	// Clear the flag
	g->send_packet = false;
}

/**
 * Internal function. Do the basic initialisation common to all packet
 * generators.
 */
void
spinn_packet_gen_init_base( spinn_packet_gen_t *g
                          , scheduler_t             *s
                          , buffer_t                *b
                          , spinn_packet_pool_t     *pool
                          , spinn_coord_t            position
                          , spinn_coord_t            system_size
                          , ticks_t                  period
                          , double                   bernoulli_prob
                          , void *(*on_packet_gen)(spinn_packet_t *packet, void *data)
                          , void *on_packet_gen_data
                          , spinn_packet_gen_dist_t  dist
                          )
{
	// Set up data-structure fields
	g->buffer             = b;
	g->pool               = pool;
	g->position           = position;
	g->system_size        = system_size;
	g->bernoulli_prob     = bernoulli_prob;
	g->on_packet_gen      = on_packet_gen;
	g->on_packet_gen_data = on_packet_gen_data;
	g->dist               = dist;
	
	// Set up tick/tock functions
	scheduler_schedule( s, period
	                  , spinn_packet_gen_tick, (void *)g
	                  , spinn_packet_gen_tock, (void *)g
	                  );
}

void
spinn_packet_gen_cyclic_init( spinn_packet_gen_t  *g
                            , scheduler_t         *s
                            , buffer_t            *b
                            , spinn_packet_pool_t *pool
                            , spinn_coord_t        position
                            , spinn_coord_t        system_size
                            , ticks_t              period
                            , double               bernoulli_prob
                            , void *(*on_packet_gen)(spinn_packet_t *packet, void *data)
                            , void *on_packet_gen_data
                            )
{
	spinn_packet_gen_init_base(
		g,s,b,pool,position,system_size, period, bernoulli_prob, on_packet_gen, on_packet_gen_data,
		SPINN_DIST_CYCLIC);
	
	// Start the cyclic message sending from the current position
	g->dist_data.cyclic.next_dest = position;
}


void
spinn_packet_gen_uniform_init( spinn_packet_gen_t  *g
                             , scheduler_t         *s
                             , buffer_t            *b
                             , spinn_packet_pool_t *pool
                             , spinn_coord_t        position
                             , spinn_coord_t        system_size
                             , ticks_t              period
                             , double               bernoulli_prob
                             , void *(*on_packet_gen)(spinn_packet_t *packet, void *data)
                             , void *on_packet_gen_data
                             )
{
	spinn_packet_gen_init_base(
		g,s,b,pool,position,system_size, period, bernoulli_prob, on_packet_gen, on_packet_gen_data,
		SPINN_DIST_UNIFORM);
}


void
spinn_packet_gen_set_bernoulli_prob( spinn_packet_gen_t *g
                                   , double              bernoulli_prob
                                   )
{
	g->bernoulli_prob = bernoulli_prob;
}


void
spinn_packet_gen_destroy(spinn_packet_gen_t *g)
{
	// Nothing to free!
}


/******************************************************************************
 * Packet consumer
 ******************************************************************************/

/**
 * Tick function which decides whether to consume a packet (based on the
 * availability of a packet in the buffer and then a Bernoulli trial.
 */
void
spinn_packet_con_tick(void *c_)
{
	spinn_packet_con_t *c = (spinn_packet_con_t *)c_;
	
	c->consume_packet = !buffer_is_empty(c->buffer)
	                 && (((double)rand())/((double)RAND_MAX+1.0)) <= c->bernoulli_prob;
}

/**
 * Tock function: actually consume the value if required.
 */
void
spinn_packet_con_tock(void *c_)
{
	spinn_packet_con_t *c = (spinn_packet_con_t *)c_;
	
	// Do nothing if no packet due to be sent
	if (!c->consume_packet)
		return;
	
	// Consume the packet
	spinn_packet_t *p = buffer_pop(c->buffer);
	
	// Run the callback
	if (c->on_packet_con)
		c->on_packet_con(p, c->on_packet_con_data);
	
	// Clear up
	spinn_packet_pool_pfree(c->pool, p);
	
	// Clear the flag
	c->consume_packet = false;
}

void
spinn_packet_con_init( spinn_packet_con_t      *c
                     , scheduler_t             *s
                     , buffer_t                *b
                     , spinn_packet_pool_t     *pool
                     , ticks_t                  period
                     , double                   bernoulli_prob
                     , void (*on_packet_con)(spinn_packet_t *packet, void *data)
                     , void *on_packet_con_data
                     )
{
	// Set up data-structure fields
	c->buffer             = b;
	c->pool               = pool;
	c->bernoulli_prob     = bernoulli_prob;
	c->on_packet_con      = on_packet_con;
	c->on_packet_con_data = on_packet_con_data;
	
	// Set up tick/tock functions
	scheduler_schedule( s, period
	                  , spinn_packet_con_tick, (void *)c
	                  , spinn_packet_con_tock, (void *)c
	                  );
}


void
spinn_packet_con_set_bernoulli_prob( spinn_packet_con_t *c
                                   , double              bernoulli_prob
                                   )
{
	c->bernoulli_prob = bernoulli_prob;
}


void
spinn_packet_con_destroy(spinn_packet_con_t *c)
{
	// Nothing to free!
}

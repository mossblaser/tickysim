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
                     , bool            use_wrap_around_links
                     , void           *payload
                     )
{
	// Set the trivial fields
	p->source       = source;
	p->destination  = destination;
	p->emg_state    = SPINN_EMG_NORMAL;
	p->payload      = payload;
	p->num_hops     = 0;
	p->num_emg_hops = 0;
	
	// Calculate the vector to travel along
	spinn_full_coord_t v;
	if (use_wrap_around_links) {
		// Find the path between the src/dest on a torus
		v = spinn_shortest_vector(source, destination, system_size);
	} else {
		v = (spinn_full_coord_t){ destination.x - source.x
		                        , destination.y - source.y
		                        , 0
		                        };
		v = spinn_full_coord_minimise(v);

	}
	
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


int
spinn_packet_pool_get_num_packets(spinn_packet_pool_t *pool)
{
	return pool->num_packets;
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
	
	if (g->enabled) {
		switch (g->temporal_dist) {
			case SPINN_GT_DIST_BERNOULLI:
				g->send_packet |= (((double)rand())/((double)RAND_MAX+1.0)) <= g->temporal_dist_data.bernoulli.prob;
				break;
			
			case SPINN_GT_DIST_PERIODIC:
				g->send_packet = g->temporal_dist_data.periodic.time_elapsed >= g->temporal_dist_data.periodic.interval - 1;
				g->temporal_dist_data.periodic.time_elapsed++;
				break;
		
			case SPINN_GT_DIST_FIXED_DELAY:
				// If the buffer is ready, only generate a packet after the required time has elapsed
				if (!buffer_is_full(g->buffer)) {
					g->send_packet = g->temporal_dist_data.fixed_delay.time_elapsed >= g->temporal_dist_data.fixed_delay.delay - 1;
					g->temporal_dist_data.fixed_delay.time_elapsed++;
				}
				break;
			
			default:
				g->send_packet = false;
				break;
		}
	} else {
		g->send_packet = false;
	}
	
	g->output_blocked = buffer_is_full(g->buffer);
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
	
	// If the buffer is full, don't send but raise the callback with a NULL packet
	if (g->output_blocked) {
		if (g->on_packet_gen)
			g->on_packet_gen(NULL, g->on_packet_gen_data);
		return;
	}
	
	// Determine the packet destination based on the current distribution. If the
	// filter rejects the packet, loop until a destination is chosen which
	// satisfies it. If the destination picked was (-1,-1) then don't generate a
	// packet.
	spinn_coord_t destination;
	do {
		switch (g->spatial_dist) {
			case SPINN_GS_DIST_CYCLIC:
				destination = g->spatial_dist_data.cyclic.next_dest;
				
				// Move to the next node
				g->spatial_dist_data.cyclic.next_dest.x ++;
				if (g->spatial_dist_data.cyclic.next_dest.x >= g->system_size.x) {
					g->spatial_dist_data.cyclic.next_dest.x = 0;
					g->spatial_dist_data.cyclic.next_dest.y ++;
					if (g->spatial_dist_data.cyclic.next_dest.y >= g->system_size.y) {
						g->spatial_dist_data.cyclic.next_dest.y = 0;
					}
				}
				break;
			
			default:
			case SPINN_GS_DIST_UNIFORM:
				destination.x = (int)((((double)rand())/((double)RAND_MAX+1.0)) * g->system_size.x);
				destination.y = (int)((((double)rand())/((double)RAND_MAX+1.0)) * g->system_size.y);
				break;
			
			case SPINN_GS_DIST_P2P:
				destination.x = g->spatial_dist_data.p2p.target.x;
				destination.y = g->spatial_dist_data.p2p.target.y;
				break;
			
			case SPINN_GS_DIST_COMPLEMENT:
				destination.x = g->system_size.x - g->position.x - 1;
				destination.y = g->system_size.y - g->position.y - 1;
				break;
			
			case SPINN_GS_DIST_TRANSPOSE:
				destination.x = g->position.y;
				destination.y = g->position.x;
				break;
			
			case SPINN_GS_DIST_TORNADO:
				destination.x = ((g->system_size.x/2) + g->position.x) % g->system_size.x;
				destination.y = g->position.y;
				break;
		}
		
		// If destination is (-1,-1) then exit early and don't generate a packet
		if (destination.x == -1 && destination.y == -1)
			return;
	} while (g->dest_filter != NULL
	         && !g->dest_filter(&destination, g->dest_filter_data)
	        );
	
	// Produce the packet
	spinn_packet_t *p = spinn_packet_pool_palloc(g->pool);
	spinn_packet_init_dor(p, g->position, destination, g->system_size, g->use_wrap_around_links, NULL);
	p->sent_time = scheduler_get_ticks(g->scheduler);
	
	// Set up the payload and run the callback
	if (g->on_packet_gen)
		p->payload = g->on_packet_gen(p, g->on_packet_gen_data);
	
	// Send the packet
	buffer_push(g->buffer, (void *)p);
	
	// Clear the flag
	g->send_packet = false;
	
	switch (g->temporal_dist) {
		case SPINN_GT_DIST_PERIODIC:
			// Reset the timer for the periodic temporal distribution as a packet has
			// now been sent
			g->temporal_dist_data.periodic.time_elapsed = 0;
			break;
		case SPINN_GT_DIST_FIXED_DELAY:
			// Reset the timer for the fixed_delay temporal distribution as a packet
			// has now been sent
			g->temporal_dist_data.fixed_delay.time_elapsed = 0;
			break;
		
		default:
			// Nothing to do for other distributions
			break;
	}
}


void
spinn_packet_gen_init( spinn_packet_gen_t  *g
                     , scheduler_t         *s
                     , buffer_t            *b
                     , spinn_packet_pool_t *pool
                     , spinn_coord_t        position
                     , spinn_coord_t        system_size
                     , ticks_t              period
                     , bool                 use_wrap_around_links
                     , bool (*dest_filter)(const spinn_coord_t *proposed_destination, void *data)
                     , void *dest_filter_data
                     , void *(*on_packet_gen)(spinn_packet_t *packet, void *data)
                     , void *on_packet_gen_data
                     )
{
	// Set up data-structure fields
	g->scheduler             = s;
	g->buffer                = b;
	g->pool                  = pool;
	g->enabled               = true;
	g->position              = position;
	g->system_size           = system_size;
	g->use_wrap_around_links = use_wrap_around_links;
	g->dest_filter           = dest_filter;
	g->dest_filter_data      = dest_filter_data;
	g->on_packet_gen         = on_packet_gen;
	g->on_packet_gen_data    = on_packet_gen_data;
	g->send_packet           = false;
	
	// Set up tick/tock functions
	scheduler_schedule( s, period
	                  , spinn_packet_gen_tick, (void *)g
	                  , spinn_packet_gen_tock, (void *)g
	                  );
	
	// Initially leave distribution values undefined.
}


void
spinn_packet_gen_set_enabled( spinn_packet_gen_t *g
                            , bool                enabled
                            )
{
	g->enabled = enabled;
}


void
spinn_packet_gen_set_temporal_dist_bernoulli( spinn_packet_gen_t *g
                                            , double              bernoulli_prob
                                            )
{
	g->temporal_dist = SPINN_GT_DIST_BERNOULLI;
	g->temporal_dist_data.bernoulli.prob = bernoulli_prob;
}


void
spinn_packet_gen_set_temporal_dist_periodic( spinn_packet_gen_t *g
                                           , int                 interval
                                           )
{
	g->temporal_dist = SPINN_GT_DIST_PERIODIC;
	g->temporal_dist_data.periodic.interval = interval;
	g->temporal_dist_data.periodic.time_elapsed = 0;
}


void
spinn_packet_gen_set_temporal_dist_fixed_delay( spinn_packet_gen_t *g
                                              , int                 delay
                                              )
{
	g->temporal_dist = SPINN_GT_DIST_FIXED_DELAY;
	g->temporal_dist_data.fixed_delay.delay = delay;
	g->temporal_dist_data.fixed_delay.time_elapsed = 0;
}


void
spinn_packet_gen_set_spatial_dist_uniform(spinn_packet_gen_t *g)
{
	g->spatial_dist = SPINN_GS_DIST_UNIFORM;
}


void
spinn_packet_gen_set_spatial_dist_p2p( spinn_packet_gen_t *g
                                     , spinn_coord_t       target
                                     )
{
	g->spatial_dist = SPINN_GS_DIST_P2P;
	g->spatial_dist_data.p2p.target.x = target.x;
	g->spatial_dist_data.p2p.target.y = target.y;
}


void
spinn_packet_gen_set_spatial_dist_cyclic(spinn_packet_gen_t *g)
{
	g->spatial_dist = SPINN_GS_DIST_CYCLIC;
	g->spatial_dist_data.cyclic.next_dest = g->position;
}


void
spinn_packet_gen_set_spatial_dist_complement(spinn_packet_gen_t *g)
{
	g->spatial_dist = SPINN_GS_DIST_COMPLEMENT;
}


void
spinn_packet_gen_set_spatial_dist_transpose(spinn_packet_gen_t *g)
{
	g->spatial_dist = SPINN_GS_DIST_TRANSPOSE;
}


void
spinn_packet_gen_set_spatial_dist_tornado(spinn_packet_gen_t *g)
{
	g->spatial_dist = SPINN_GS_DIST_TORNADO;
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
	
	switch (c->temporal_dist) {
		case SPINN_CT_DIST_BERNOULLI:
			c->consume_packet |= (((double)rand())/((double)RAND_MAX+1.0)) <= c->temporal_dist_data.bernoulli.prob;
			break;
		
		case SPINN_CT_DIST_PERIODIC:
			c->consume_packet = c->temporal_dist_data.periodic.time_elapsed >= c->temporal_dist_data.periodic.interval - 1;
			c->temporal_dist_data.periodic.time_elapsed++;
			break;
		
		case SPINN_CT_DIST_FIXED_DELAY:
			// If a packet is available, only accept it after the required time has elapsed
			if (!buffer_is_empty(c->buffer)) {
				c->consume_packet = c->temporal_dist_data.fixed_delay.time_elapsed >= c->temporal_dist_data.fixed_delay.delay - 1;
				c->temporal_dist_data.fixed_delay.time_elapsed++;
			}
			break;
		
		default:
			c->consume_packet = false;
			break;
	}
	
	// Do nothing after all if the buffer was empty anyway
	if (buffer_is_empty(c->buffer)) {
		c->consume_packet = false;
	}
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
	
	// Clear the flag
	c->consume_packet = false;
	
	// Run the callback
	if (c->on_packet_con)
		c->on_packet_con(p, c->on_packet_con_data);
	
	// Clear up
	spinn_packet_pool_pfree(c->pool, p);
	
	switch (c->temporal_dist) {
		case SPINN_CT_DIST_PERIODIC:
			// Reset the timer for the periodic temporal distribution as a packet has
			// now been sent
			c->temporal_dist_data.periodic.time_elapsed = 0;
			break;
		case SPINN_CT_DIST_FIXED_DELAY:
			// Reset the timer for the fixed_delay temporal distribution as a packet
			// has now been sent
			c->temporal_dist_data.fixed_delay.time_elapsed = 0;
			break;
		
		default:
			// Nothing to do for other distributions
			break;
	}
}

void
spinn_packet_con_init( spinn_packet_con_t      *c
                     , scheduler_t             *s
                     , buffer_t                *b
                     , spinn_packet_pool_t     *pool
                     , ticks_t                  period
                     , void (*on_packet_con)(spinn_packet_t *packet, void *data)
                     , void *on_packet_con_data
                     )
{
	// Set up data-structure fields
	c->buffer             = b;
	c->pool               = pool;
	c->on_packet_con      = on_packet_con;
	c->on_packet_con_data = on_packet_con_data;
	c->consume_packet     = false;
	
	// Set up tick/tock functions
	scheduler_schedule( s, period
	                  , spinn_packet_con_tick, (void *)c
	                  , spinn_packet_con_tock, (void *)c
	                  );
	
	// Initially leave the distribution parameters undefined.
}


void
spinn_packet_con_set_temporal_dist_bernoulli( spinn_packet_con_t *c
                                            , double              bernoulli_prob
                                            )
{
	c->temporal_dist = SPINN_GT_DIST_BERNOULLI;
	c->temporal_dist_data.bernoulli.prob = bernoulli_prob;
}


void
spinn_packet_con_set_temporal_dist_periodic( spinn_packet_con_t *c
                                           , int                 interval
                                           )
{
	c->temporal_dist = SPINN_CT_DIST_PERIODIC;
	c->temporal_dist_data.periodic.interval = interval;
	c->temporal_dist_data.periodic.time_elapsed = 0;
}

void
spinn_packet_con_set_temporal_dist_fixed_delay( spinn_packet_con_t *c
                                              , int                 delay
                                              )
{
	c->temporal_dist = SPINN_CT_DIST_FIXED_DELAY;
	c->temporal_dist_data.fixed_delay.delay = delay;
	c->temporal_dist_data.fixed_delay.time_elapsed = 0;
}


void
spinn_packet_con_destroy(spinn_packet_con_t *c)
{
	// Nothing to free!
}

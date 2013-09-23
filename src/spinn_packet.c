/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_packet.h -- A SpiNNaker packet and packet generator/consumer models.
 */


#include "config.h"

#include <assert.h>

#include "scheduler.h"
#include "buffer.h"

#include "spinn.h"
#include "spinn_packet.h"

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

/******************************************************************************
 * Internal functions.
 ******************************************************************************/

/******************************************************************************
 * Public functions.
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

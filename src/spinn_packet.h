/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_packet.h -- A SpiNNaker packet and packet generator/consumer models.
 */

#ifndef SPINN_PACKET_H
#define SPINN_PACKET_H

#include "config.h"

#include "scheduler.h"
#include "buffer.h"

#include "spinn.h"

/******************************************************************************
 * SpiNNaker Packets
 ******************************************************************************/

/**
 * A SpiNNaker packet.
 */
typedef struct spinn_packet {
	// The intended inflection point of the packet's route
	spinn_coord_t     inflection_point;
	spinn_direction_t inflection_direction;
	
	// The the location where the packet was injected
	spinn_coord_t source;
	
	// The intended destination of the packet
	spinn_coord_t destination;
	
	// The direction the packet is currently heading (specifically, the last
	// output port the packet was sent via.
	spinn_direction_t direction;
	
	// Emergency-routing state of the packet
	spinn_emg_state_t emg_state;
	
	// Time at which the packet was sent
	ticks_t sent_time;
	
	// Number of hops (of which are emergency legs)
	ticks_t num_hops;
	ticks_t num_emg_hops;
	
	// Packet payload
	void *payload;
} spinn_packet_t;


/**
 * Convenience function. Initialise a spinn_packet_t with the appropriate values
 * to cause it to be dimension-order routed from the source to destination locations
 * in a system of the specified size. Also resets all other fields to the values
 * expected of a new packet.
 *
 * The use_wrap_around_links is a bool which sets whether the wrap around links
 * should be used or not.
 *
 * Note: Does not set the sent_time field.
 */
void spinn_packet_init_dor( spinn_packet_t *packet
                          , spinn_coord_t   source
                          , spinn_coord_t   destination
                          , spinn_coord_t   system_size
                          , bool            use_wrap_around_links
                          , void           *payload
                          );



/******************************************************************************
 * Utility function datatypes
 ******************************************************************************/

/**
 * A pool of spinn_packet_t values.
 */
typedef struct spinn_packet_pool spinn_packet_pool_t;


/**
 * The internal data-structure of a packet generator.
 */
typedef struct spinn_packet_gen spinn_packet_gen_t;


/**
 * The internal data-structure of a packet consumer.
 */
typedef struct spinn_packet_con spinn_packet_con_t;


// Concrete definitions of the above types
#include "spinn_packet_internal.h"


/******************************************************************************
 * Packet Pool
 ******************************************************************************/

/**
 * Create a packet pool.
 */
void spinn_packet_pool_init(spinn_packet_pool_t *pool);


/**
 * Recover the memory used by a packet pool and the packets it created.
 */
void spinn_packet_pool_destroy(spinn_packet_pool_t *pool);


/**
 * Get the current size of the packet pool in packets.
 */
int spinn_packet_pool_get_num_packets(spinn_packet_pool_t *pool);


/**
 * Get an uninitialised packet from the pool.
 */
spinn_packet_t *spinn_packet_pool_palloc(spinn_packet_pool_t *pool);


/**
 * Return a packet to the pool.
 */
void spinn_packet_pool_pfree(spinn_packet_pool_t *pool, spinn_packet_t *packet);

/******************************************************************************
 * Packet generators
 ******************************************************************************/


/**
 * Create a packet generator. The distributions to be used must be set using the
 * functions which follow.
 *
 * @param scheduler A scheduler into which the packet generator will schedule
 *                  itself.
 * @param buffer The buffer into which generated packets will be inserted.
 * @param packet_pool A pool of packet objects to save on malloc/free calls.
 *
 * @param position The coordinates of the router the packet generator will be
 *                 feeding.
 * @param system_size The size of the torus being simulated.
 *
 * @param period The period at which the packet generator will run.
 *
 * @param dest_filter A function which checks whether a given destination is
 *                    allowed. If NULL, allow any destination.
 * @param dest_filter_data A pointed to be passed to dest_filter.
 *
 * @param on_packet_gen Is a function called during the tock phase just after
 *                      packet creation but before it is sent. The value
 *                      returned is set as the packet payload. If NULL, the
 *                      callback is disabled. If the output is blocked but a
 *                      packet would have been generated if it wasnt, the value
 *                      of packet is NULL.
 * @param on_packet_gen_data A pointer passed to the on_packet_gen function.
 */
void spinn_packet_gen_init( spinn_packet_gen_t  *gen
                          , scheduler_t         *scheduler
                          , buffer_t            *buffer
                          , spinn_packet_pool_t *packet_pool
                          , spinn_coord_t        position
                          , spinn_coord_t        system_size
                          , ticks_t              period
                          , bool                 use_wrap_around_links
                          , bool (*dest_filter)(const spinn_coord_t *proposed_destination, void *data)
                          , void *dest_filter_data
                          , void *(*on_packet_gen)(spinn_packet_t *packet, void *data)
                          , void *on_packet_gen_data
                          );


/**
 * Set up the packet generator to use the given Bernoulli distribution to decide
 * when to generate packets.
 *
 * This should be called outside of the simulation tick/tock phases for
 * deterministic behaviour.
 */
void spinn_packet_gen_set_temporal_dist_bernoulli( spinn_packet_gen_t *packet_gen
                                                 , double              bernoulli_prob
                                                 );


/**
 * Set up the packet generator to use the given periodic process decide when to
 * generate packets. After a given interval, a packet will be sent, or if the
 * output is blocked, resend will be tried in the next period until it is sent.
 * At this point, the process repeats.
 *
 * This should be called outside of the simulation tick/tock phases for
 * deterministic behaviour.
 */
void spinn_packet_gen_set_temporal_dist_periodic( spinn_packet_gen_t *packet_gen
                                                , int                 interval
                                                );


/**
 * Set up the packet generator to send packets to uniform random destinations.
 *
 * This should be called outside of the simulation tick/tock phases for
 * deterministic behaviour.
 */
void spinn_packet_gen_set_spatial_dist_uniform(spinn_packet_gen_t *packet_gen);


/**
 * Set up the packet generator to send packets to a single, specified target.
 *
 * This should be called outside of the simulation tick/tock phases for
 * deterministic behaviour.
 */
void spinn_packet_gen_set_spatial_dist_p2p( spinn_packet_gen_t *packet_gen
                                          , spinn_coord_t       target
                                          );


/**
 * Set up the packet generator to send packets in a complement pattern where the
 * sender's X and Y coordinates are mirrored down the Y and X axes respectively
 * to produce the destination address.
 *
 * This should be called outside of the simulation tick/tock phases for
 * deterministic behaviour.
 */
void spinn_packet_gen_set_spatial_dist_complement(spinn_packet_gen_t *packet_gen);


/**
 * Set up the packet generator to send packets in a transpose pattern where the
 * sender's X and Y coordinates are swapped to produce the destination address.
 *
 * This should be called outside of the simulation tick/tock phases for
 * deterministic behaviour.
 */
void spinn_packet_gen_set_spatial_dist_transpose(spinn_packet_gen_t *packet_gen);


/**
 * Set up the packet generator to send packets in a tornado pattern where
 * packets are sent half the system's size to the right.
 *
 * This should be called outside of the simulation tick/tock phases for
 * deterministic behaviour.
 */
void spinn_packet_gen_set_spatial_dist_tornado(spinn_packet_gen_t *packet_gen);


/**
 * Set up the packet generator to send packets to each node of the system in
 * turn, starting with the current node.
 *
 * This should be called outside of the simulation tick/tock phases for
 * deterministic behaviour.
 */
void spinn_packet_gen_set_spatial_dist_cyclic(spinn_packet_gen_t *packet_gen);

/**
 * Free the resources used by a packet generator.
 */
void spinn_packet_gen_destroy(spinn_packet_gen_t *packet_gen);



/******************************************************************************
 * Packet consumers
 ******************************************************************************/

/**
 * Create a packet consumer which accepts (and immediately pfrees) packets.
 *
 * @param scheduler A scheduler into which the packet consumer will schedule
 *                  itself.
 * @param buffer The buffer out of which generated packets will be consumed.
 * @param packet_pool A pool of packet objects to save on malloc/free calls.
 *
 * @param period The period at which the packet generator will run.
 *
 * @param on_packet_con Is a function called during the tock phase just after
 *                      the packet arrives and just before it is freed. If NULL,
 *                      the callback is disabled.
 * @param on_packet_con_data A pointer passed to the on_packet_con function.
 */
void spinn_packet_con_init( spinn_packet_con_t  *con
                          , scheduler_t         *scheduler
                          , buffer_t            *buffer
                          , spinn_packet_pool_t *packet_pool
                          , ticks_t              period
                          , void (*on_packet_con)(spinn_packet_t *packet, void *data)
                          , void *on_packet_con_data
                          );


/**
 * Set up the packet consumer to use the given Bernoulli distribution to decide
 * when to consume packets.
 *
 * This should be called outside of the simulation tick/tock phases for
 * deterministic behaviour.
 */
void spinn_packet_con_set_temporal_dist_bernoulli( spinn_packet_con_t *packet_con
                                                 , double              bernoulli_prob
                                                 );


/**
 * Set up the packet consumer to use the given periodic process decide when to
 * consume packets. After a given interval, a packet will be sent, or if the
 * output is blocked, resend will be tried in the next period until it is sent.
 * At this point, the process repeats.
 *
 * This should be called outside of the simulation tick/tock phases for
 * deterministic behaviour.
 */
void spinn_packet_con_set_temporal_dist_periodic( spinn_packet_con_t *packet_con
                                                , int                 interval
                                                );


/**
 * Free the resources used by a packet consumer.
 */
void spinn_packet_con_destroy(spinn_packet_con_t *packet_con);

#endif

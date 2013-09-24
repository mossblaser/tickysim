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
	
	// The intended destination of the packet
	spinn_coord_t destination;
	
	// The direction the packet is currently heading (specifically, the last
	// output port the packet was sent via.
	spinn_direction_t direction;
	
	// Emergency-routing state of the packet
	spinn_emg_state_t emg_state;
	
	// Packet payload
	void *payload;
} spinn_packet_t;


/**
 * Convenience function. Initialise a spinn_packet_t with the appropriate values
 * to cause it to be dimension-order routed from the source to destination locations
 * in a system of the specified size. Also resets all other fields to the values
 * expected of a new packet.
 */
void spinn_packet_init_dor( spinn_packet_t *packet
                          , spinn_coord_t   source
                          , spinn_coord_t   destination
                          , spinn_coord_t   system_size
                          , void           *payload
                          );



/******************************************************************************
 * Packet Pool
 ******************************************************************************/

/**
 * A pool of spinn_packet_t values.
 */
typedef struct spinn_packet_pool spinn_packet_pool_t;


/**
 * Create a packet pool.
 */
spinn_packet_pool_t *spinn_packet_pool_create(void);


/**
 * Recover the memory used by a packet pool and the packets it created.
 */
void spinn_packet_pool_free(spinn_packet_pool_t *pool);


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
 * The internal data-structure of a packet generator.
 */
typedef struct spinn_packet_gen spinn_packet_gen_t;


/**
 * Create a packet generator which produces packets destined for a cyclic
 * pattern of destination.
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
 * @param bernoulli_prob The probability with which a packet will be generated
 *                       when the packet generator runs.
 */
spinn_packet_gen_t *spinn_packet_gen_cyclic_create( scheduler_t         *scheduler
                                                  , buffer_t            *buffer
                                                  , spinn_packet_pool_t *packet_pool
                                                  , spinn_coord_t        position
                                                  , spinn_coord_t        system_size
                                                  , ticks_t              period
                                                  , double               bernoulli_prob
                                                  );


/**
 * Create a packet generator which produces packets destined for a uniform
 * random locations.
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
 * @param bernoulli_prob The probability with which a packet will be generated
 *                       when the packet generator runs.
 */
spinn_packet_gen_t *spinn_packet_gen_uniform_create( scheduler_t         *scheduler
                                                   , buffer_t            *buffer
                                                   , spinn_packet_pool_t *packet_pool
                                                   , spinn_coord_t        position
                                                   , spinn_coord_t        system_size
                                                   , ticks_t              period
                                                   , double               bernoulli_prob
                                                   );

/**
 * Free the resources used by a packet generator.
 */
void spinn_packet_gen_free(spinn_packet_gen_t *packet_gen);



/******************************************************************************
 * Packet consumers
 ******************************************************************************/

/**
 * The internal data-structure of a packet consumer.
 */
typedef struct spinn_packet_con spinn_packet_con_t;


/**
 * Create a packet consumer which accepts (and immediately pfrees) packets.
 *
 * @param scheduler A scheduler into which the packet consumer will schedule
 *                  itself.
 * @param buffer The buffer out of which generated packets will be consumed.
 * @param packet_pool A pool of packet objects to save on malloc/free calls.
 *
 * @param period The period at which the packet generator will run.
 * @param bernoulli_prob The probability with which a packet will be accepted
 *                       when the packet consumer runs.
 */
spinn_packet_con_t *spinn_packet_con_create( scheduler_t         *scheduler
                                           , buffer_t            *buffer
                                           , spinn_packet_pool_t *packet_pool
                                           , ticks_t              period
                                           , double               bernoulli_prob
                                           );

/**
 * Free the resources used by a packet consumer.
 */
void spinn_packet_con_free(spinn_packet_con_t *packet_con);

#endif

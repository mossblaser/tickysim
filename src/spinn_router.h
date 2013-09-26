/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_router.h -- A SpiNNaker router model.
 */

#ifndef SPINN_ROUTER_H
#define SPINN_ROUTER_H

#include <stdbool.h>

#include "config.h"

#include "scheduler.h"
#include "buffer.h"

#include "spinn.h"
#include "spinn_packet.h"

/**
 * A model of a SpiNNaker router.
 */
typedef struct spinn_router spinn_router_t;


// Concrete definitions of the above types
#include "spinn_router_internal.h"


/**
 * A model of a router core in a SpiNNaker system. Takes spinn_packet_t pointers
 * from a single input buffer and routes them to one of the provided output
 * buffers at a given period.
 *
 * @param scheduler The scheduler controling the simulation.
 * @param period The period at which the router will attempt to route packets.
 *
 * @param input A single buffer containing a merged stream of packets from
 *              multiple inputs.
 * @param outputs An set of 7 output buffers, one per output direction.
 *
 * @param position The coordinates of the router in the system's overall mesh.
 *
 * @param use_emg_routing Enable or disable emergency routing
 *
 * @param first_timeout The timeout before trying emergency routing (or
 *                      dropping if emergency routing is disabled). Measured in
 *                      multiples of the router period.
 * 
 * @param final_timeout The additional timeout before dropping a packet while
 *                      trying to emergency route. Measured in multiples of the
 *                      router period.
 *
 * @param on_forward A callback function to be called when a packet is forwarded
 *                   (called in the "tock" simulator phase). The arguments
 *                   passed are a references to the router object, the packet
 *                   that was dropped and a user-specified value.  If NULL, the
 *                   callback is disabled.
 * @param on_forward_data The user-defined data to pass along with the
 *                        on_forward callback.
 *
 * @param on_drop A callback function to be called when a packet is dropped
 *                (called in the "tock" simulator phase). The arguments passed
 *                are a references to the router object, the packet that was
 *                dropped and a user-specified value. If NULL, the callback is
 *                disabled.
 * @param on_drop_data The user-defined data to pass along with the on_drop
 *                     callback.
 */
void spinn_router_init( spinn_router_t *router
                      , scheduler_t    *scheduler
                      , ticks_t         period
                      , int             num_pipeline_stages
                      , buffer_t       *input
                      , buffer_t       *outputs[7]
                      , spinn_coord_t   position
                      , bool            use_emg_routing
                      , int             first_timeout
                      , int             final_timeout
                      , void            (*on_forward)( spinn_router_t    *router
                                                     , spinn_packet_t    *packet
                                                     , void              *data
                                                     )
                      , void            *on_forward_data
                      , void            (*on_drop)( spinn_router_t *router
                                                  , spinn_packet_t *packet
                                                  , void           *data
                                                  )
                      , void            *on_drop_data
                      );


/**
 * Free resources used by the router. Callbacks registered with the scheduler
 * will become invalid and so the scheduler should not be used after a call to
 * this function.
 */
void spinn_router_destroy(spinn_router_t *router);

#endif

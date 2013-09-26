/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_router.h -- A SpiNNaker router model.
 *
 * The router, once per period, will check its input buffer for a new packet
 * and, if the output port is not blocked, will pop the packet from the input
 * and push it into the output. The throughput and the latency of the router are
 * therefore one packet per cycle and one cycle respectively.
 */


#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "scheduler.h"
#include "buffer.h"

#include "spinn.h"
#include "spinn_topology.h"
#include "spinn_packet.h"
#include "spinn_router.h"

/******************************************************************************
 * Internal functions.
 ******************************************************************************/

/**
 * Work out which port the packet should be sent from, assuming that it is being
 * routed normally.
 */
spinn_direction_t
get_packet_output_direction(spinn_router_t *r, spinn_packet_t *p)
{
	// Except at the inflection point and endpoint, just keep moving in the same
	// direction
	if ( r->position.x == p->destination.x &&
	     r->position.y == p->destination.y)
		return SPINN_LOCAL;
	else if ( r->position.x == p->inflection_point.x &&
	          r->position.y == p->inflection_point.y)
		return p->inflection_direction;
	else if (p->emg_state == SPINN_EMG_SECOND_LEG)
		return spinn_next_cw(p->direction);
	else
		return p->direction;
}


void
spinn_router_tick(void *r_)
{
	spinn_router_t *r = (spinn_router_t *)r_;
	
	// If there is packet to route or drop, do so
	if (r->pipeline[r->num_pipeline_stages-1].valid) {
		spinn_packet_t *p = r->pipeline[r->num_pipeline_stages-1].data;
		
		// Find out the intended direction and emergency mode of the packet
		switch (p->emg_state) {
			case SPINN_EMG_NORMAL:
			case SPINN_EMG_SECOND_LEG:
				r->selected_output_direction = get_packet_output_direction(r, p);
				if (r->use_emg_routing && r->time_elapsed >= r->first_timeout) {
					r->cur_packet_emg_state      = SPINN_EMG_FIRST_LEG;
					r->selected_output_direction = spinn_next_cw(r->selected_output_direction);
				} else {
					r->cur_packet_emg_state = SPINN_EMG_NORMAL;
				}
				break;
			
			case SPINN_EMG_FIRST_LEG:
				r->cur_packet_emg_state = SPINN_EMG_SECOND_LEG;
				r->selected_output_direction = spinn_next_cw(spinn_opposite(p->direction));
				break;
		}
		
		r->forward_packet = false;
		r->drop_packet    = false;
		
		if (!buffer_is_full(r->outputs[r->selected_output_direction])) {
			// Is the output available? Forward the packet to this port!
			r->forward_packet = true;
		} else if (!r->use_emg_routing &&
		           p->emg_state != SPINN_EMG_FIRST_LEG &&
		           r->time_elapsed >= r->first_timeout) {
			// Drop the packet as emergency routing is disabled and it has timed out
			r->drop_packet = true;
		} else if ( (p->emg_state == SPINN_EMG_FIRST_LEG &&
		             r->time_elapsed > r->first_timeout) ||
		           r->time_elapsed >= r->first_timeout + r->final_timeout){
			// TODO: Drop the timed-out emergency routed packet
			r->drop_packet = true;
		}
	}
	
	// If a packet is available it may be possible to add it to the pipeline
	r->accept_packet = !buffer_is_empty(r->input);
}


void
spinn_router_tock(void *r_)
{
	spinn_router_t *r = (spinn_router_t *)r_;
	
	// Deal with sending of packets
	if (!r->pipeline[r->num_pipeline_stages-1].valid) {
		// No packet at the end of the pipeline: do nothing
	} else if (!r->forward_packet && !r->drop_packet) {
		// If no forwarding/dropping to do, just advance the clock
		r->time_elapsed++;
	} else {
		// Grab the packet from the end of the pipeline (and invalidate the value
		// there to allow the pipeline to advance)
		spinn_packet_t *p = r->pipeline[r->num_pipeline_stages-1].data;
		r->pipeline[r->num_pipeline_stages-1].valid = false;
		
		if (r->forward_packet) {
			// Set the packet flags
			p->direction = r->selected_output_direction;
			p->emg_state = r->cur_packet_emg_state;
			
			// Forward the current packet to the output
			buffer_push(r->outputs[r->selected_output_direction], p);
			
			// Raise the forwarding callback
			if (r->on_forward != NULL)
				r->on_forward(r, p, r->on_forward_data);
		} else if (r->drop_packet && r->on_drop != NULL) {
			// Drop the packet (just call the callback)
			r->on_drop(r, p, r->on_drop_data);
		}
		
		// Reset router state ready for next packet
		r->time_elapsed = 0;
	}
	
	// Advance the pipeline
	for (int i = r->num_pipeline_stages-1; i >= 1; i--) {
		if (!r->pipeline[i].valid) {
			r->pipeline[i] = r->pipeline[i-1];
			r->pipeline[i-1].valid = false;
		}
	}
	
	// Attempt to accept a packet if possible
	if (r->accept_packet && !r->pipeline[0].valid) {
		r->pipeline[0].valid = true;
		r->pipeline[0].data  = buffer_pop(r->input);
	}
}


/******************************************************************************
 * Public functions.
 ******************************************************************************/

void
spinn_router_init( spinn_router_t *r
                 , scheduler_t    *s
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
                 )
{
	// Initialise internal fields
	r->time_elapsed              = 0;
	
	// Set up the pipeline
	r->num_pipeline_stages = num_pipeline_stages;
	r->pipeline = calloc(r->num_pipeline_stages, sizeof(spinn_router_pipeline_t));
	assert(r->pipeline != NULL);
	for (int i = 0; i < r->num_pipeline_stages; i++)
		r->pipeline[i].valid = false;
	
	// Copy fields from parameters
	r->input = input;
	memcpy(r->outputs, outputs, sizeof(buffer_t *) * 7);
	
	r->position    = position;
	
	r->use_emg_routing = use_emg_routing;
	r->first_timeout   = first_timeout;
	r->final_timeout   = final_timeout;
	
	r->on_forward      = on_forward;
	r->on_forward_data = on_forward_data;
	
	r->on_drop      = on_drop;
	r->on_drop_data = on_drop_data;
	
	// Set up tick/tock callbacks in the scheduler
	scheduler_schedule( s, period
	                  , spinn_router_tick, (void *)r
	                  , spinn_router_tock, (void *)r
	                  );
}


void
spinn_router_destroy(spinn_router_t *r)
{
	free(r->pipeline);
}




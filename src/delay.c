/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * delay.h -- A block which connects two buffers and will forward a single value
 * to a second buffer after it has been waiting in the first buffer for a given
 * number of cycles.
 */


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "config.h"

#include "scheduler.h"
#include "buffer.h"
#include "delay.h"


/******************************************************************************
 * Private functions.
 ******************************************************************************/

/**
 * Internal function.
 *
 * Check to see if a value is available for forwarding and forwarding is
 * allowed/possible.
 */
void
delay_tick(void *d_)
{
	delay_t *d = (delay_t *)d_;
	
	d->forward = false;
	
	// The input and output buffers are both ready!
	if (!buffer_is_empty(d->input) && !buffer_is_full(d->output)) {
		d->time_elapsed++;
		
		// If the value has been waiting long enough forward it.
		if (d->time_elapsed >= d->delay) {
			d->forward = true;
			d->time_elapsed = 0;
		}
	}
}



/**
 * Internal function.
 *
 * Actually do the forwarding.
 */
void
delay_tock(void *d_)
{
	delay_t *d = (delay_t *)d_;
	
	if (d->forward)
		buffer_push(d->output, buffer_pop(d->input));
}


/******************************************************************************
 * Public functions.
 ******************************************************************************/

void
delay_init( delay_t     *d
          , scheduler_t *s
          , ticks_t      period
          , int          delay
          , buffer_t    *input
          , buffer_t    *output
          )
{
	// Set up struct values
	d->input  = input;
	d->output = output;
	
	d->delay    = delay;
	d->time_elapsed = 0;
	
	// Schedule the arbiter tick/tock functions to occur at the specified
	// interval.
	scheduler_schedule( s, period
	                  , delay_tick, (void *)d
	                  , delay_tock, (void *)d
	                  );
}


void
delay_set_delay(delay_t *d, int delay)
{
	d->delay = delay;
}


void
delay_destroy( delay_t *d)
{
	// Nothing to do!
}

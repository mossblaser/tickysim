/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * arbiter.c -- A round-robbin arbiter which at a regular period attempts to
 * forward values from a set of input buffers into a single output buffer using
 * a round-robbin approach in the case of contention.
 */


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "config.h"

#include "scheduler.h"
#include "buffer.h"
#include "arbiter.h"


/******************************************************************************
 * Internal datastructures.
 ******************************************************************************/

/**
 * The structure representing a particular arbiter. Includes the index of the
 * last input handled.
 *
 * The field handle_input is a bool set by the tick function and specifies
 * whether a value from the input is to be forwarded by the tock function. If it
 * is true, the input indicated by last_input will have its value forwarded to
 * the output.
 */
struct arbiter {
	buffer_t **inputs;
	size_t     num_inputs;
	buffer_t  *output;
	
	size_t last_input;
	
	bool handle_input;
};


/******************************************************************************
 * Private functions.
 ******************************************************************************/

/**
 * Internal function.
 *
 * Arbiter "tick" callback. Checks to see if a value should be forwarded.
 */
void
arbiter_tick(void *a_)
{
	arbiter_t *a = (arbiter_t *)a_;
	
	// Immediately stop if the output is blocked
	if (buffer_is_full(a->output))
		return;
	
	// Iterate over all the inputs
	for (int i_ = 0; i_ < a->num_inputs; i_++) {
		// Starting after the last one to be handled
		int i = (i_+a->last_input+1)%a->num_inputs;
		
		if (!buffer_is_empty(a->inputs[i])) {
			// There is a value ready at this input, set it to be forwarded during the
			// tock phase.
			a->last_input   = i;
			a->handle_input = true;
			return;
		}
	}
	
	// No inputs were ready, do nothing!
}

/**
 * Internal function.
 *
 * Arbiter "tock" callback. Forwards a packet if requested by tick.
 */
void
arbiter_tock(void *a_)
{
	arbiter_t *a = (arbiter_t *)a_;
	
	if (a->handle_input) {
		void *value = buffer_pop(a->inputs[a->last_input]);
		buffer_push(a->output, value);
		
		a->handle_input = false;
	}
}


/******************************************************************************
 * Public functions.
 ******************************************************************************/

arbiter_t *
arbiter_create( scheduler_t *s
              , ticks_t      period
              , buffer_t   **inputs
              , size_t       num_inputs
              , buffer_t    *output
              )
{
	arbiter_t *a = malloc(sizeof(arbiter_t));
	assert(a != NULL);
	
	// Copy the input array into a local copy
	a->inputs = calloc(num_inputs, sizeof(buffer_t *));
	assert(a->inputs != NULL);
	memcpy(a->inputs, inputs, num_inputs * sizeof(buffer_t *));
	
	a->num_inputs = num_inputs;
	
	a->output = output;
	
	// Initialised so that if the inputs are immediately contended, input 0 will
	// go first. This isn't neccessary (since the decision is arbitary) but won't
	// surprise someone watching progress externally.
	a->last_input = num_inputs-1;
	
	a->handle_input = false;
	
	// Schedule the arbiter tick/tock functions to occur at the specified
	// interval.
	scheduler_schedule( s, period
	                  , arbiter_tick, (void *)a
	                  , arbiter_tock, (void *)a
	                  );
	
	return a;
}





void
arbiter_free( arbiter_t *a)
{
	free(a->inputs);
	free(a);
}

/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * arbiter.h -- A round-robbin arbiter which at a regular period attempts to
 * forward values from a set of input buffers into a single output buffer using
 * a round-robbin approach in the case of contention.
 */

#ifndef ARBITER_H
#define ARBITER_H

#include <stdlib.h>

#include "config.h"

#include "scheduler.h"
#include "buffer.h"

/**
 * A data structure defining an arbiter.
 */
typedef struct arbiter arbiter_t;


/**
 * Create a new arbiter. Adds itself to the scheduler with the requesteed period.
 *
 * @param scheduler The scheduler controling the simulation.
 * @param period The period at which the arbiter will attempt to forward one
 *               value.
 * @param inputs An array of input buffers to arbitrate between. This array will
 *               be copied.
 * @param num_inputs The length of the inputs array.
 * @param output The output buffer into which values should be placed.
 */
arbiter_t *arbiter_create( scheduler_t *scheduler
                         , ticks_t      period
                         , buffer_t   **inputs
                         , size_t       num_inputs
                         , buffer_t    *output
                         );


/**
 * Free the resources from an arbiter. Note that the scheduler this was
 * registered with must also be freed as it will be left holding a reference to
 * invalid tick/tock functions.
 */
void arbiter_free(arbiter_t *arbiter);

#endif

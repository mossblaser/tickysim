/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * delay.h -- A block which connects two buffers and will forward a single value
 * to a second buffer after it has been waiting in the first buffer for a given
 * number of cycles.
 */

#ifndef DELAY_H
#define DELAY_H

#include <stdlib.h>

#include "config.h"

#include "scheduler.h"
#include "buffer.h"

/**
 * A data structure defining a delay.
 */
typedef struct delay delay_t;


// Concrete definitions of the above types
#include "delay_internal.h"


/**
 * Initialise a new delay. Adds itself to the scheduler with the requesteed period.
 *
 * @param scheduler The scheduler controling the simulation.
 * @param period The period at which the delay will attempt to forward one
 *               value.
 * @param delay The number of periods a value must wait in the input buffer
 *              before being forwarded.
 * @param input The input buffer.
 * @param output The output buffer.
 */
void delay_init( delay_t     *d
               , scheduler_t *s
               , ticks_t      period
               , int          delay
               , buffer_t    *input
               , buffer_t    *output
               );


/**
 * Change the number of delay ticks.
 */
void delay_set_delay(delay_t *d, int delay);



/**
 * Free the resources from an delay. Note that the scheduler this was registered
 * with must also be freed as it will be left holding a reference to invalid
 * tick/tock functions.
 */
void delay_destroy(delay_t *d);

#endif


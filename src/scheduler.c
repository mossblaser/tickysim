/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * scheduler.c -- A scheduler, the generic heart of TickySim.
 *
 * See header file for an introduction to the scheduler's use.
 *
 * The scheduler works on a scheduler_t which contains the current simulation
 * time in ticks and a list of schedule_t structs. The schedule_t structs
 * each correspond to a list of event_t structs and the period of which calls to
 * all the events should occur. The event_t structs simply contain callbacks for
 * for the tick and tock phases.
 */

#include <stdlib.h>
#include <assert.h>

#include "config.h"

#include "scheduler.h"

/******************************************************************************
 * Internal datastructures.
 ******************************************************************************/


/**
 * Internal datastructure.
 *
 * An event which can be added to the schedule. Consists of two callable
 * functions to be called at a regular interval along with void pointer to be
 * passed to the functions.
 *
 * All "tick" functions will be called before any "tock" function is called.
 * These should typically correspond to reading and writing state. Functions may
 * be NULL in which case they won't be called.
 *
 * Internally these structs form an element of a linked list of events.
 */
typedef struct event {
	void (*tick)(void *data);
	void *tick_data;
	
	void (*tock)(void *data);
	void *tock_data;
	
	struct event *next_event;
} event_t;


/**
 * Internal datastructure.
 *
 * A linked list of period/event_t tuples.
 */
typedef struct schedule {
	ticks_t  period;
	event_t *events;
	
	struct schedule *next_schedule;
} schedule_t;


/**
 * The "main" data-structure of a scheduler.
 */
struct scheduler {
	/* A pointer to the start of the linked list of schedule_t structs. */
	schedule_t *schedules;
	
	/* The current simulation time */
	ticks_t ticks;
};


/******************************************************************************
 * Internal functions.
 ******************************************************************************/

/**
 * Internal function.
 *
 * Get a pointer to a schedule with the speficied period. If it doesn't exist,
 * creates it.
 */
schedule_t *
get_schedule(scheduler_t *s, ticks_t period)
{
	// Try and find a schedule with the requested period
	schedule_t *next_schedule = s->schedules;
	while (next_schedule != NULL) {
		if (next_schedule->period == period)
			return next_schedule;
		next_schedule = next_schedule->next_schedule;
	}
	
	// No matching schedule found, create a new one at the start of the list
	schedule_t *new_schedule = malloc(sizeof(schedule_t));
	assert(new_schedule != NULL);
	
	new_schedule->period        = period;
	new_schedule->events        = NULL;
	new_schedule->next_schedule = s->schedules;
	
	s->schedules = new_schedule;
	
	return new_schedule;
}


/******************************************************************************
 * Publicly accessible functions.
 ******************************************************************************/


scheduler_t *
scheduler_create(void)
{
	scheduler_t *s = malloc(sizeof(scheduler_t));
	assert(s != NULL);
	
	// Initialise the structure
	s->ticks     = 0;
	s->schedules = NULL;
	
	return s;
}


void
scheduler_schedule( scheduler_t *s
                  , ticks_t period
                  , void (*tick)(void *)
                  , void *tick_data
                  , void (*tock)(void *)
                  , void *tock_data
                  )
{
	// Get the schedule for this period
	schedule_t *schedule = get_schedule(s, period);
	
	// Create new event and add it to the start of the period's schedule
	event_t *new_event = malloc(sizeof(event_t));
	assert(new_event != NULL);
	
	new_event->tick       = tick;
	new_event->tick_data  = tick_data;
	new_event->tock       = tock;
	new_event->tock_data  = tock_data;
	new_event->next_event = schedule->events;
	
	schedule->events = new_event;
}


ticks_t
scheduler_get_ticks(scheduler_t *s)
{
	return s->ticks;
}


void
scheduler_tick_tock(scheduler_t *s)
{
	schedule_t *next_schedule;
	event_t    *next_event;
	
	// Tick
	next_schedule = s->schedules;
	while (next_schedule != NULL) {
		// Go through events only at the specified period
		if (s->ticks % next_schedule->period == 0) {
			next_event = next_schedule->events;
			while (next_event != NULL) {
				// Run the tick function if defined.
				if (next_event->tick)
					next_event->tick(next_event->tick_data);
				next_event = next_event->next_event;
			}
		}
		next_schedule = next_schedule->next_schedule;
	}
	
	// Tock
	next_schedule = s->schedules;
	while (next_schedule != NULL) {
		// Go through events only at the specified period
		if (s->ticks % next_schedule->period == 0) {
			next_event = next_schedule->events;
			while (next_event != NULL) {
				// Run the tock function if defined.
				if (next_event->tock)
					next_event->tock(next_event->tock_data);
				next_event = next_event->next_event;
			}
		}
		next_schedule = next_schedule->next_schedule;
	}
	
	// Advance time
	s->ticks ++;
}

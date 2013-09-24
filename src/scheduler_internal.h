/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * scheduler_internal.h -- Concrete definitions of internal datastrucutres. This
 * is provided to allow the creation of these types. Users should not access the
 * fields directly. This file should only be included by scheduler.h
 */

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


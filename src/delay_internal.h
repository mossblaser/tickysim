/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * delay_internal.h -- Concrete definitions of internal datastrucutres. This is
 * provided to allow the creation of these types. Users should not access the
 * fields directly. This file should only be included by delay.h
 */

struct delay {
	// The input and output buffers
	buffer_t *input;
	buffer_t *output;
	
	// The minimum number of periods which must have elapsed before a value is
	// forwarded to the next buffer.
	int delay;
	
	// The number of periods the first buffer has contained a value which hasn't
	// been forwarded.
	int current_delay;
	
	// Should the value in the first buffer be popped and placed in the next
	// buffer? (Set in the tick phase and read in the tock phase).
	bool forward;
};

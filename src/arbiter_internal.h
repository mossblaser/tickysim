/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * arbiter_internal.h -- Concrete definitions of internal datastrucutres. This
 * is provided to allow the creation of these types. Users should not access the
 * fields directly. This file should only be included by arbiter.h
 */


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


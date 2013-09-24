/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * buffer.h -- A generic buffer implementation.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <stdbool.h>

#include "config.h"

/**
 * *** Do not access these fields directly. ***
 *
 * A structure defining a particular instance of a buffer.
 *
 * Contains an array of values size+1 in length with pointers to the head and
 * tail of values in the buffer. The head points to the next empty space and the
 * tail points to the next value to be retreived from the buffer.
 *
 * Empty: (pointers overlapping)
 *   ,-----------------,
 *   |  |  |  |  |  |  |
 *   '-----------------'
 *     |
 *    tail+head
 *
 * Part-filled: (pointers not overlapping or adjacent)
 *   ,-----------------,
 *   |##|##|##|  |  |  |
 *   '-----------------'
 *     |        |
 *    tail     head
 *
 * Full: (pointers adjacent)
 *   ,-----------------,
 *   |##|##|##|##|##|  |
 *   '-----------------'
 *     |              |
 *    tail          head
 */
typedef struct buffer {
	void   **values;
	size_t   size;
	int      head;
	int      tail;
} buffer_t;


/**
 * Initialise a buffer of the specified length.
 */
void buffer_init(buffer_t *buffer, size_t size);

/**
 * Free the buffer from memory.
 */
void buffer_destroy(buffer_t *buffer);

/**
 * Test whether the buffer is full.
 */
bool buffer_is_full(buffer_t *buffer);

/**
 * Test whether the buffer is empty.
 */
bool buffer_is_empty(buffer_t *buffer);

/**
 * Insert a value into the buffer.
 */
void buffer_push(buffer_t *buffer, void *value);

/**
 * Retreive a value from the buffer.
 */
void *buffer_pop(buffer_t *buffer);

/**
 * Get the value of the item which will be popped next. If the buffer_is_empty()
 * is true, the behaviour is undefined.
 */
void *buffer_peek(buffer_t *buffer);


#endif

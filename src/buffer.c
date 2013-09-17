/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * buffer.c -- A generic buffer implementation.
 */

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "config.h"

#include "buffer.h"

/******************************************************************************
 * Internal datastructures.
 ******************************************************************************/

/**
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
struct buffer {
	void   **values;
	size_t   size;
	int      head;
	int      tail;
};


/******************************************************************************
 * Public Functions
 ******************************************************************************/

buffer_t *
buffer_create(size_t size)
{
	buffer_t *b = malloc(sizeof(buffer_t));
	assert(b != NULL);
	
	b->values = calloc(size+1, sizeof(void *));
	assert(b->values != NULL);
	b->size = size;
	b->head = 0;
	b->tail = 0;
	
	return b;
}


void
buffer_free(buffer_t *b)
{
	free(b->values);
	free(b);
}


bool
buffer_is_full(buffer_t *b)
{
	return (b->head+1)%(b->size+1) == (b->tail);
}


bool
buffer_is_empty(buffer_t *b)
{
	return b->head == b->tail;
}


void
buffer_push(buffer_t *b, void *value)
{
	assert(!buffer_is_full(b));
	
	b->values[b->head] = value;
	b->head = (b->head+1)%(b->size + 1);
}


void *
buffer_pop(buffer_t *b)
{
	assert(!buffer_is_empty(b));
	
	void *value = b->values[b->tail];
	b->tail = (b->tail+1)%(b->size + 1);
	
	return value;
}


void *
buffer_peek(buffer_t *b)
{
	assert(!buffer_is_empty(b));
	
	return b->values[b->tail];
}


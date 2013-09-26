/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * buffer.c -- A generic buffer implementation.
 *
 * These buffers are not clocked in any way. They can push and pop an arbitary
 * number of packets per cycle.
 */

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "config.h"

#include "buffer.h"


/******************************************************************************
 * Public Functions
 ******************************************************************************/

void
buffer_init(buffer_t *b, size_t size)
{
	b->values = calloc(size+1, sizeof(void *));
	assert(b->values != NULL);
	b->size = size;
	b->head = 0;
	b->tail = 0;
}


void
buffer_destroy(buffer_t *b)
{
	free(b->values);
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


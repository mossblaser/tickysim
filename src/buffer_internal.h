/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * buffer_internal.h -- Concrete definitions of internal datastrucutres. This is
 * provided to allow the creation of these types. Users should not access the
 * fields directly. This file should only be included by buffer.h
 */

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
struct buffer {
	void   **values;
	size_t   size;
	int      head;
	int      tail;
};


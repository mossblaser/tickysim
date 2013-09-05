/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * main.c -- World starts here!
 */

#include <stdio.h>

#include "scheduler.h"
#include "buffer.h"

void print_tick(void *msg){printf("    Tick: %s\n",(char*)msg);}
void print_tock(void *msg){printf("    Tock: %s\n",(char*)msg);}

int
main(int argc, char *argv[])
{
	
	printf("Testing the scheduler:\n");
	char *tick_msg1 = "1.";
	char *tick_msg2 = "2.";
	char *tock_msg1 = "1!";
	char *tock_msg2 = "2!";
	
	scheduler_t *s = scheduler_create();
	scheduler_schedule(s, 1, print_tick, tick_msg1, print_tock, tock_msg1);
	scheduler_schedule(s, 1, print_tick, tick_msg1, print_tock, tock_msg1);
	scheduler_schedule(s, 2, print_tick, tick_msg2, print_tock, tock_msg2);
	
	for (int i = 0; i < 3; i++) {
		printf("  Hello, world @ time %d!\n", scheduler_get_ticks(s));
		scheduler_tick_tock(s);
	}
	
	printf("\nTesting the buffer:\n");
	char *buf_elements = "ABCDEFGH";
	
	buffer_t *b = buffer_create(4);
	printf("  Buffer is initially empty=%d and full=%d\n", buffer_is_empty(b), buffer_is_full(b));
	for (int j = 0; j < 2; j++) {
		// Insert a few items
		for (int i = 0; i < 4; i++) {
			buffer_push(b, (void *)buf_elements + i);
			printf("  Buffer on insert [%d]=%c, empty=%d and full=%d\n",
			       i, buf_elements[i], buffer_is_empty(b), buffer_is_full(b));
		}
		// Pop a few items
		for (int i = 0; i < 4; i++) {
			char *c = buffer_pop(b);
			printf("  Buffer on pop [%d]=%c, empty=%d and full=%d\n",
			       i, *c, buffer_is_empty(b), buffer_is_full(b));
		}
	}
	
	return 0;
}

/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * main.c -- World starts here!
 */

#include <stdio.h>

#include "scheduler.h"

void print_tick(void *msg){printf("Tick: %s\n",(char*)msg);}
void print_tock(void *msg){printf("Tock: %s\n",(char*)msg);}

int
main(int argc, char *argv[])
{
	char *tick_msg1 = "1.";
	char *tick_msg2 = "2.";
	char *tock_msg1 = "1!";
	char *tock_msg2 = "2!";
	
	scheduler_t *s = scheduler_create();
	scheduler_schedule(s, 1, print_tick, tick_msg1, print_tock, tock_msg1);
	scheduler_schedule(s, 1, print_tick, tick_msg1, print_tock, tock_msg1);
	scheduler_schedule(s, 1, print_tick, tick_msg1, print_tock, tock_msg1);
	scheduler_schedule(s, 2, print_tick, tick_msg2, print_tock, tock_msg2);
	
	for (int i = 0; i < 3; i++) {
		printf("Hello, world @ time %d!\n", scheduler_get_ticks(s));
		scheduler_tick_tock(s);
	}
	return 0;
}

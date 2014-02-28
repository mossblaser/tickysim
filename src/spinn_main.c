/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_main.c -- A SpiNNaker simulator implemented using TickySim.
 */

#include "config.h"

#include <stdio.h>
#include <time.h>

#include "spinn_packet.h"
#include "spinn_router.h"
#include "spinn_topology.h"

// An internal function in spinn_router.c
spinn_direction_t
get_packet_output_direction(spinn_router_t *r, spinn_packet_t *p);



/******************************************************************************
 * World starts here
 ******************************************************************************/

#define WIDTH  12
#define HEIGHT 12

unsigned int routing_tables[WIDTH][HEIGHT][1024][2];


void
load_routing_tables(void)
{
	// Load routing tables from file
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) { 
			char filename[32];
			filename[31] = '\0';
			snprintf(filename, 31, "spinn144tables/spin144.%d.%d", x, y);
			FILE *f = fopen(filename, "r");
			
			int i = 0;
			int num_matched = 2;
			while (num_matched == 2 && i < 1024) {
				num_matched = fscanf(f, "%x, %x\n"
				                      , &(routing_tables[x][y][i][0])
				                      , &(routing_tables[x][y][i][1])
				                      );
				if (num_matched == 2)
					i++;
			}
			for (; i < 1024; i++) {
				routing_tables[x][y][i][0] = -1;
				routing_tables[x][y][i][1] = -1;
			}
		}
	}
}


spinn_direction_t
get_routing_table_direction(spinn_router_t *r, spinn_packet_t *p)
{
	// XXX: X and Y are the wrong way around in Javier's files.
	unsigned int key = p->destination.y<<8 | p->destination.x;
	
	int x = r->position.x;
	int y = r->position.y;
	
	for (int i = 0; i < 1024; i++) {
		if (routing_tables[x][y][i][0] == key) {
			switch (routing_tables[x][y][i][1]) {
				case 1u<<0: return SPINN_EAST;
				case 1u<<1: return SPINN_NORTH_EAST;
				case 1u<<2: return SPINN_NORTH;
				case 1u<<3: return SPINN_WEST;
				case 1u<<4: return SPINN_SOUTH_WEST;
				case 1u<<5: return SPINN_SOUTH;
				default   : return SPINN_LOCAL;
			}
		}
	}
	
	fprintf(stdout, "ERROR: No routing key at %d,%d for destination %d,%d!\n"
	              , r->position.x, r->position.y
	              , p->destination.x, p->destination.y
	              );
	return SPINN_LOCAL;
}


int
main(int argc, char *argv[])
{
	load_routing_tables();
	
	//srand((int)time(NULL));
	
	// Test all possible routes
	for (int x1 = 0; x1 < WIDTH; x1++) {
		fprintf(stderr, "%d/%d...\n", x1, WIDTH);
		for (int y1 = 0; y1 < HEIGHT; y1++) {
			for (int x2 = 0; x2 < WIDTH; x2++) {
				for (int y2 = 0; y2 < HEIGHT; y2++) {
					// Work out route
					spinn_packet_t p;
					spinn_packet_init_dor( &p
					                     , ((spinn_coord_t){x1,y1})
					                     , ((spinn_coord_t){x2,y2})
					                     , ((spinn_coord_t){WIDTH,HEIGHT})
					                     , true
					                     , NULL
					                     );
					
					// Follow the route
					// This router is the current router the packet is at
					spinn_router_t r;
					r.position.x = x1;
					r.position.y = y1;
					while (true) {
						spinn_direction_t direction = get_packet_output_direction(&r, &p);
						spinn_direction_t table_direction = get_routing_table_direction(&r, &p);
						
						// Check against routing tables
						if (table_direction != direction) {
							spinn_full_coord_t full_vector = spinn_shortest_vector(r.position, p.destination, ((spinn_coord_t){WIDTH,HEIGHT}));
							spinn_coord_t vector = spinn_full_coord_to_coord(full_vector);
							
							fprintf(stdout, "ERROR: Packet %d,%d -> %d,%d routed %d rather than %d in table at %d,%d (shortest vector: %d,%d,%d aka %d,%d)!\n"
							              , p.source.x, p.source.y
							              , p.destination.x, p.destination.y
							              , direction
							              , table_direction
							              , r.position.x, r.position.y
							              , full_vector.x, full_vector.y, full_vector.z
							              , vector.x, vector.y
							              );
							break;
							//direction = table_direction;
						}
						
						// Advance the packet along
						if (direction == SPINN_LOCAL) {
							if (r.position.x != x2 || r.position.y != y2) {
								fprintf(stdout, "ERROR: Packet %d,%d -> %d,%d reached wrong destination %d,%d!\n"
								              , p.source.x, p.source.y
								              , p.destination.x, p.destination.y
								              , r.position.x, r.position.y
								              );
							}
							// Packet arrived at destination
							break;
						} else {
							// Move on
							spinn_coord_t delta = spinn_dir_to_vector(direction);
							r.position.x += delta.x + WIDTH;
							r.position.y += delta.y + HEIGHT;
							r.position.x %= WIDTH;
							r.position.y %= HEIGHT;
							p.direction = direction;
						}
					}
				}
			}
		}
	}
}

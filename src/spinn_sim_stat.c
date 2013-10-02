/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_sim_stat.c -- Statistic monitoring functions for various SpiNNaker models.
 */


#include "config.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "spinn_sim.h"
#include "spinn_sim_stat.h"
#include "spinn_sim_config.h"



/******************************************************************************
 * Callback functions
 ******************************************************************************/

/**
 * Internal function which loggs the arrival of a packet.
 */
void
spinn_sim_stat_log_packet( bool delivered
                         , spinn_packet_t *packet
                         , spinn_node_t *node
                         )
{
	// Do nothing during warmup
	if (node->sim->repeat_num == -1)
		return;
	
	fprintf( node->sim->stat_file_packet_details
	       , "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n"
	       , node->sim->repeat_num + 1
	       , delivered
	       , packet->source.x,      packet->source.y
	       , packet->destination.x, packet->destination.y
	       , packet->sent_time
	       , scheduler_get_ticks(&(node->sim->scheduler)) - packet->sent_time
	       , packet->num_hops
	       , packet->num_emg_hops
	       );
}


void *
spinn_sim_stat_on_packet_gen(spinn_packet_t *packet, void *node_)
{
	spinn_node_t *node = (spinn_node_t *)node_;
	
	node->stat_packets_offered++;
	
	// If a packet is not accepted by the network, this callback gets called with
	// a NULL packet.
	if (packet != NULL)
		node->stat_packets_accepted++;
	
	return NULL;
}


void
spinn_sim_stat_on_packet_con(spinn_packet_t *packet, void *node_)
{
	spinn_node_t *node = (spinn_node_t *)node_;
	
	node->stat_packets_arrived++;
	
	if (node->sim->stat_log_delivered_packets)
		spinn_sim_stat_log_packet(true, packet, node);
}


void
spinn_sim_stat_on_drop(spinn_router_t *router, spinn_packet_t *packet, void *node_)
{
	spinn_node_t *node = (spinn_node_t *)node_;
	
	node->stat_packets_dropped++;
	
	if (node->sim->stat_log_dropped_packets)
		spinn_sim_stat_log_packet(false, packet, node);
}



/******************************************************************************
 * Initialisation Functions
 ******************************************************************************/

void
spinn_sim_stat_init_global_counters(spinn_sim_t *sim)
{
	const char *basename   = "global_counters.dat";
	const char *result_dir = spinn_sim_config_lookup_string(sim, "measurements.results_directory");
	
	// A string long enough for the filename
	char *filename = calloc(strlen(result_dir) + strlen(basename) + 1, sizeof(char));
	assert(filename != NULL);
	strcpy(filename, result_dir);
	strcat(filename, basename);
	
	// What global stats are being counted?
	bool glbl_packets_offered = spinn_sim_config_lookup_bool(sim,
		"measurements.global_counters.packets_offered");
	bool glbl_packets_accepted = spinn_sim_config_lookup_bool(sim,
		"measurements.global_counters.packets_accepted");
	bool glbl_packets_arrived = spinn_sim_config_lookup_bool(sim,
		"measurements.global_counters.packets_arrived");
	bool glbl_packets_dropped = spinn_sim_config_lookup_bool(sim,
		"measurements.global_counters.packets_dropped");
	
	sim->stat_file_global_counters = NULL;
	
	// Open the global counters file if some are being kept
	if (glbl_packets_offered || glbl_packets_accepted ||
			glbl_packets_arrived || glbl_packets_dropped) {
		sim->stat_file_global_counters = fopen(filename, "w");
		if (sim->stat_file_global_counters == NULL) {
			fprintf(stderr, "Couldn't open %s for writing!\n", filename);
			exit(-1);
		}
		
		// Add the header
		fprintf(sim->stat_file_global_counters, "repeat\tnum_cycles");
		if (glbl_packets_offered)  fprintf(sim->stat_file_global_counters, "\tpackets_offered");
		if (glbl_packets_accepted) fprintf(sim->stat_file_global_counters, "\tpackets_accepted");
		if (glbl_packets_arrived)  fprintf(sim->stat_file_global_counters, "\tpackets_arrived");
		if (glbl_packets_dropped)  fprintf(sim->stat_file_global_counters, "\tpackets_dropped");
		fprintf(sim->stat_file_global_counters, "\n");
	}
	
	// Clean up
	free(filename);
}


void
spinn_sim_stat_init_per_node_counters(spinn_sim_t *sim)
{
	const char *basename   = "per_node_counters.dat";
	const char *result_dir = spinn_sim_config_lookup_string(sim, "measurements.results_directory");
	
	// A string long enough for the filename
	char *filename = calloc(strlen(result_dir) + strlen(basename) + 1, sizeof(char));
	assert(filename != NULL);
	strcpy(filename, result_dir);
	strcat(filename, basename);
	
	// What global stats are being counted?
	bool per_node_packets_offered = spinn_sim_config_lookup_bool(sim,
		"measurements.per_node_counters.packets_offered");
	bool per_node_packets_accepted = spinn_sim_config_lookup_bool(sim,
		"measurements.per_node_counters.packets_accepted");
	bool per_node_packets_arrived = spinn_sim_config_lookup_bool(sim,
		"measurements.per_node_counters.packets_arrived");
	bool per_node_packets_dropped = spinn_sim_config_lookup_bool(sim,
		"measurements.per_node_counters.packets_dropped");
	
	sim->stat_file_per_node_counters = NULL;
	
	// Open the per-node counters file if some are being kept
	if (per_node_packets_offered || per_node_packets_accepted ||
			per_node_packets_arrived || per_node_packets_dropped) {
		sim->stat_file_per_node_counters = fopen(filename, "w");
		if (sim->stat_file_per_node_counters == NULL) {
			fprintf(stderr, "Couldn't open %s for writing!\n", filename);
			exit(-1);
		}
		
		// Add the header
		fprintf(sim->stat_file_per_node_counters, "repeat\tnum_cycles\tnode_x\tnode_y");
		if (per_node_packets_offered)  fprintf(sim->stat_file_per_node_counters, "\tpackets_offered");
		if (per_node_packets_accepted) fprintf(sim->stat_file_per_node_counters, "\tpackets_accepted");
		if (per_node_packets_arrived)  fprintf(sim->stat_file_per_node_counters, "\tpackets_arrived");
		if (per_node_packets_dropped)  fprintf(sim->stat_file_per_node_counters, "\tpackets_dropped");
		fprintf(sim->stat_file_per_node_counters, "\n");
	}
	
	// Clean up
	free(filename);
}


void
spinn_sim_stat_init_packet_details(spinn_sim_t *sim)
{
	const char *basename   = "packet_details.dat";
	const char *result_dir = spinn_sim_config_lookup_string(sim, "measurements.results_directory");
	
	// A string long enough for the filename
	char *filename = calloc(strlen(result_dir) + strlen(basename) + 1, sizeof(char));
	assert(filename != NULL);
	strcpy(filename, result_dir);
	strcat(filename, basename);
	
	// What global stats are being counted?
	sim->stat_log_delivered_packets = spinn_sim_config_lookup_bool(sim,
		"measurements.packet_details.delivered_packets");
	sim->stat_log_dropped_packets = spinn_sim_config_lookup_bool(sim,
		"measurements.packet_details.dropped_packets");
	
	sim->stat_file_packet_details = NULL;
	
	// Open the per-node counters file if some are being kept
	if (sim->stat_log_delivered_packets || sim->stat_log_dropped_packets) {
		sim->stat_file_packet_details = fopen(filename, "w");
		if (sim->stat_file_packet_details == NULL) {
			fprintf(stderr, "Couldn't open %s for writing!\n", filename);
			exit(-1);
		}
		
		// Add the header
		fprintf(sim->stat_file_packet_details,
		        "repeat\tdelivered\t"
		        "source_x\tsource_y\tdest_x\tdest_y\t"
		        "sent_time\tlatency\tnum_hops\temg_hops\n"
		        );
	}
	
	// Clean up
	free(filename);
}


/******************************************************************************
 * Destroy Functions
 ******************************************************************************/

void
spinn_sim_stat_destroy_global_counters(spinn_sim_t *sim)
{
	if (sim->stat_file_global_counters != NULL)
		if (fclose(sim->stat_file_global_counters) != 0)
			fprintf(stderr, "Error closing global counters data file.\n");
}


void
spinn_sim_stat_destroy_per_node_counters(spinn_sim_t *sim)
{
	if (sim->stat_file_per_node_counters != NULL)
		if (fclose(sim->stat_file_per_node_counters) != 0)
			fprintf(stderr, "Error closing per-node counters data file.\n");
}


void
spinn_sim_stat_destroy_packet_details(spinn_sim_t *sim)
{
	if (sim->stat_file_packet_details != NULL)
		if (fclose(sim->stat_file_packet_details) != 0)
			fprintf(stderr, "Error closing packet details data file.\n");
}


/******************************************************************************
 * Start Functions
 ******************************************************************************/

void
spinn_sim_stat_start_global_counters(spinn_sim_t *sim)
{
	// Nothing to do (all counters are those from the per-node counters)
}


void
spinn_sim_stat_start_per_node_counters(spinn_sim_t *sim)
{
	// Reset all counters
	for (size_t i = 0; i < sim->system_size.x*sim->system_size.y; i++) {
		sim->nodes[i].stat_packets_offered  = 0;
		sim->nodes[i].stat_packets_accepted = 0;
		sim->nodes[i].stat_packets_arrived  = 0;
		sim->nodes[i].stat_packets_dropped  = 0;
	}
}


void
spinn_sim_stat_start_packet_details(spinn_sim_t *sim)
{
	// Nothing to do
}


/******************************************************************************
 * End Functions
 ******************************************************************************/

void
spinn_sim_stat_end_global_counters(spinn_sim_t *sim)
{
	// What global stats are being counted?
	bool glbl_packets_offered = spinn_sim_config_lookup_bool(sim,
		"measurements.global_counters.packets_offered");
	bool glbl_packets_accepted = spinn_sim_config_lookup_bool(sim,
		"measurements.global_counters.packets_accepted");
	bool glbl_packets_arrived = spinn_sim_config_lookup_bool(sim,
		"measurements.global_counters.packets_arrived");
	bool glbl_packets_dropped = spinn_sim_config_lookup_bool(sim,
		"measurements.global_counters.packets_dropped");
	
	// Dump into file
	if (glbl_packets_offered || glbl_packets_accepted ||
			glbl_packets_arrived || glbl_packets_dropped) {
		int stat_packets_offered  = 0;
		int stat_packets_accepted = 0;
		int stat_packets_arrived  = 0;
		int stat_packets_dropped  = 0;
		
		// Sum up all values
		for (size_t i = 0; i < sim->system_size.x*sim->system_size.y; i++) {
			stat_packets_offered  += sim->nodes[i].stat_packets_offered;
			stat_packets_accepted += sim->nodes[i].stat_packets_accepted;
			stat_packets_arrived  += sim->nodes[i].stat_packets_arrived;
			stat_packets_dropped  += sim->nodes[i].stat_packets_dropped;
		}
	
		// Add the header
		fprintf(sim->stat_file_global_counters, "%d\t%d"
		       , sim->repeat_num + 1
		       , spinn_sim_config_lookup_int(sim, "simulation.sample_duration")
		       );
		
		if (glbl_packets_offered)
			fprintf(sim->stat_file_global_counters, "\t%d", stat_packets_offered);
		if (glbl_packets_accepted)
			fprintf(sim->stat_file_global_counters, "\t%d", stat_packets_accepted);
		if (glbl_packets_arrived)
			fprintf(sim->stat_file_global_counters, "\t%d", stat_packets_arrived);
		if (glbl_packets_dropped)
			fprintf(sim->stat_file_global_counters, "\t%d", stat_packets_dropped);
		
		fprintf(sim->stat_file_global_counters, "\n");
	}
}


void
spinn_sim_stat_end_per_node_counters(spinn_sim_t *sim)
{
	// What per-node stats are being counted?
	bool per_node_packets_offered = spinn_sim_config_lookup_bool(sim,
		"measurements.per_node_counters.packets_offered");
	bool per_node_packets_accepted = spinn_sim_config_lookup_bool(sim,
		"measurements.per_node_counters.packets_accepted");
	bool per_node_packets_arrived = spinn_sim_config_lookup_bool(sim,
		"measurements.per_node_counters.packets_arrived");
	bool per_node_packets_dropped = spinn_sim_config_lookup_bool(sim,
		"measurements.per_node_counters.packets_dropped");
	
	// Dump into file
	if (per_node_packets_offered || per_node_packets_accepted ||
			per_node_packets_arrived || per_node_packets_dropped) {
		
		// Iterate over all nodes
		for (int y = 0; y < sim->system_size.y; y++) {
			for (int x = 0; x < sim->system_size.x; x++) {
				// Add the header
				fprintf(sim->stat_file_per_node_counters, "%d\t%d\t%d\t%d"
				       , sim->repeat_num + 1
				       , spinn_sim_config_lookup_int(sim, "simulation.sample_duration")
				       , x, y
				       );
				
				spinn_node_t *node = sim->nodes + (x + (sim->system_size.x * y));
				
				if (per_node_packets_offered)
					fprintf(sim->stat_file_per_node_counters, "\t%d", node->stat_packets_offered);
				if (per_node_packets_accepted)
					fprintf(sim->stat_file_per_node_counters, "\t%d", node->stat_packets_accepted);
				if (per_node_packets_arrived)
					fprintf(sim->stat_file_per_node_counters, "\t%d", node->stat_packets_arrived);
				if (per_node_packets_dropped)
					fprintf(sim->stat_file_per_node_counters, "\t%d", node->stat_packets_dropped);
				
				fprintf(sim->stat_file_per_node_counters, "\n");
			}
		}
	}
}


void
spinn_sim_stat_end_packet_details(spinn_sim_t *sim)
{
	// Nothing to do
}



/******************************************************************************
 * Public-facing functions
 ******************************************************************************/

void
spinn_sim_stat_init(spinn_sim_t *sim)
{
	spinn_sim_stat_init_global_counters(sim);
	spinn_sim_stat_init_per_node_counters(sim);
	spinn_sim_stat_init_packet_details(sim);
}


void
spinn_sim_stat_destroy(spinn_sim_t *sim)
{
	spinn_sim_stat_destroy_global_counters(sim);
	spinn_sim_stat_destroy_per_node_counters(sim);
	spinn_sim_stat_destroy_packet_details(sim);
}


void
spinn_sim_stat_start(spinn_sim_t *sim)
{
	spinn_sim_stat_start_global_counters(sim);
	spinn_sim_stat_start_per_node_counters(sim);
	spinn_sim_stat_start_packet_details(sim);
}


void
spinn_sim_stat_end(spinn_sim_t *sim)
{
	spinn_sim_stat_end_global_counters(sim);
	spinn_sim_stat_end_per_node_counters(sim);
	spinn_sim_stat_end_packet_details(sim);
}


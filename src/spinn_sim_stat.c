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
#include <sys/time.h>

#include "spinn_sim.h"
#include "spinn_sim_stat.h"
#include "spinn_sim_config.h"




/******************************************************************************
 * Internal Standard-field Printing functions
 ******************************************************************************/

/**
 * Print header columns (without terminating \t) for all common fields.
 */
void
fprint_standard_fields_headers(spinn_sim_t *sim, FILE *file)
{
	// Standard, hard-coded simulation columns
	fprintf(file, "group\tsample");
	
	// The independent variables
	for (int i = 0; i < sim->num_ivars; i++)
		fprintf(file, "\t%s", sim->ivar_names[i]);
}


/**
 * Print columns (without terminating \t) for all common fields.
 */
void
fprint_standard_fields(spinn_sim_t *sim, FILE *file)
{
	// Standard, hard-coded simulation columns
	fprintf(file, "%d\t%d", sim->cur_group+1, sim->cur_sample+1);
	
	// The independent variables
	for (int i = 0; i < sim->num_ivars; i++) {
		config_setting_t *setting = sim->ivar_settings[i];
		
		switch (config_setting_type(setting)) {
			case CONFIG_TYPE_INT:
				fprintf(file, "\t%d", config_setting_get_int(setting));
				break;
			case CONFIG_TYPE_INT64:
				fprintf(file, "\t%lld", config_setting_get_int64(setting));
				break;
			case CONFIG_TYPE_FLOAT:
				fprintf(file, "\t%f", config_setting_get_float(setting));
				break;
			case CONFIG_TYPE_STRING:
				fprintf(file, "\t%s", config_setting_get_string(setting));
				break;
			case CONFIG_TYPE_BOOL:
				fprintf(file, "\t%s", config_setting_get_bool(setting) ? "True" : "False");
				break;
			default:
				// Shouldn't have any other type of independent variable
				assert(0);
		}
	}
}


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
	// Do nothing when not sampling
	if (!node->sim->stat_started)
		return;
	
	fprint_standard_fields(node->sim, node->sim->stat_file_packet_details);
	fprintf( node->sim->stat_file_packet_details
	       , "\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n"
	       , delivered
	       , packet->source.x,      packet->source.y
	       , packet->destination.x, packet->destination.y
	       , packet->sent_time - node->sim->stat_start_ticks
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
	
	// Free the packet now we're done with it
	spinn_packet_pool_pfree(&(node->sim->pool), packet);
}


void
spinn_sim_stat_on_forward(spinn_router_t *router, spinn_packet_t *packet, void *node_)
{
	spinn_node_t *node = (spinn_node_t *)node_;
	
	node->stat_packets_forwarded++;
}


/******************************************************************************
 * Initialisation Functions
 ******************************************************************************/

void
spinn_sim_stat_open_global_counters(spinn_sim_t *sim)
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
	bool glbl_packets_forwarded = spinn_sim_config_lookup_bool(sim,
		"measurements.global_counters.packets_forwarded");
	
	sim->stat_file_global_counters = NULL;
	
	// Open the global counters file if some are being kept
	if (glbl_packets_offered || glbl_packets_accepted ||
	    glbl_packets_arrived || glbl_packets_dropped ||
	    glbl_packets_forwarded) {
		sim->stat_file_global_counters = fopen(filename, "w");
		if (sim->stat_file_global_counters == NULL) {
			fprintf(stderr, "Couldn't open %s for writing!\n", filename);
			exit(-1);
		}
		
		// Add the header
		fprint_standard_fields_headers(sim, sim->stat_file_global_counters);
		if (glbl_packets_offered)  fprintf(sim->stat_file_global_counters, "\tpackets_offered");
		if (glbl_packets_accepted) fprintf(sim->stat_file_global_counters, "\tpackets_accepted");
		if (glbl_packets_arrived)  fprintf(sim->stat_file_global_counters, "\tpackets_arrived");
		if (glbl_packets_dropped)  fprintf(sim->stat_file_global_counters, "\tpackets_dropped");
		if (glbl_packets_forwarded)fprintf(sim->stat_file_global_counters, "\tpackets_forwarded");
		fprintf(sim->stat_file_global_counters, "\n");
	}
	
	// Clean up
	free(filename);
}


void
spinn_sim_stat_open_per_node_counters(spinn_sim_t *sim)
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
	bool per_node_packets_forwarded = spinn_sim_config_lookup_bool(sim,
		"measurements.per_node_counters.packets_forwarded");
	
	sim->stat_file_per_node_counters = NULL;
	
	// Open the per-node counters file if some are being kept
	if (per_node_packets_offered || per_node_packets_accepted ||
	    per_node_packets_arrived || per_node_packets_dropped ||
	    per_node_packets_forwarded) {
		sim->stat_file_per_node_counters = fopen(filename, "w");
		if (sim->stat_file_per_node_counters == NULL) {
			fprintf(stderr, "Couldn't open %s for writing!\n", filename);
			exit(-1);
		}
		
		// Add the header
		fprint_standard_fields_headers(sim, sim->stat_file_per_node_counters);
		fprintf(sim->stat_file_per_node_counters, "\tnode_x\tnode_y");
		if (per_node_packets_offered)  fprintf(sim->stat_file_per_node_counters, "\tpackets_offered");
		if (per_node_packets_accepted) fprintf(sim->stat_file_per_node_counters, "\tpackets_accepted");
		if (per_node_packets_arrived)  fprintf(sim->stat_file_per_node_counters, "\tpackets_arrived");
		if (per_node_packets_dropped)  fprintf(sim->stat_file_per_node_counters, "\tpackets_dropped");
		if (per_node_packets_forwarded)fprintf(sim->stat_file_per_node_counters, "\tpackets_forwarded");
		fprintf(sim->stat_file_per_node_counters, "\n");
	}
	
	// Clean up
	free(filename);
}


void
spinn_sim_stat_open_packet_details(spinn_sim_t *sim)
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
		fprint_standard_fields_headers(sim, sim->stat_file_packet_details);
		fprintf(sim->stat_file_packet_details,
		        "\tdelivered\t"
		        "source_x\tsource_y\tdest_x\tdest_y\t"
		        "sent_time\tlatency\tnum_hops\temg_hops\n"
		        );
	}
	
	// Clean up
	free(filename);
}


void
spinn_sim_stat_open_simulator(spinn_sim_t *sim)
{
	const char *basename   = "simulator.dat";
	const char *result_dir = spinn_sim_config_lookup_string(sim, "measurements.results_directory");
	
	// A string long enough for the filename
	char *filename = calloc(strlen(result_dir) + strlen(basename) + 1, sizeof(char));
	assert(filename != NULL);
	strcpy(filename, result_dir);
	strcat(filename, basename);
	
	// What global stats are being counted?
	bool warmup_ticks = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.warmup_ticks");
	bool warmup_duration = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.warmup_duration");
	bool warmup_packet_pool_size = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.warmup_packet_pool_size");
	bool sample_ticks = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.sample_ticks");
	bool sample_duration = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.sample_duration");
	bool sample_packet_pool_size = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.sample_packet_pool_size");
	
	sim->stat_file_simulator = NULL;
	
	// Open the per-node counters file if some are being kept
	if (warmup_duration || sample_duration ||
			warmup_packet_pool_size || sample_packet_pool_size ||
			warmup_ticks || sample_ticks) {
		sim->stat_file_simulator = fopen(filename, "w");
		if (sim->stat_file_simulator == NULL) {
			fprintf(stderr, "Couldn't open %s for writing!\n", filename);
			exit(-1);
		}
		
		// Add the header
		fprint_standard_fields_headers(sim, sim->stat_file_simulator);
		if (warmup_ticks)            fprintf(sim->stat_file_simulator, "\twarmup_ticks");
		if (warmup_duration)         fprintf(sim->stat_file_simulator, "\twarmup_duration");
		if (warmup_packet_pool_size) fprintf(sim->stat_file_simulator, "\twarmup_packet_pool_size");
		if (sample_ticks)            fprintf(sim->stat_file_simulator, "\tsample_ticks");
		if (sample_duration)         fprintf(sim->stat_file_simulator, "\tsample_duration");
		if (sample_packet_pool_size) fprintf(sim->stat_file_simulator, "\tsample_packet_pool_size");
		fprintf(sim->stat_file_simulator, "\n");
	}
	
	// Clean up
	free(filename);
}



/******************************************************************************
 * Destroy Functions
 ******************************************************************************/

void
spinn_sim_stat_close_global_counters(spinn_sim_t *sim)
{
	if (sim->stat_file_global_counters != NULL)
		if (fclose(sim->stat_file_global_counters) != 0)
			fprintf(stderr, "Error closing global counters data file.\n");
}


void
spinn_sim_stat_close_per_node_counters(spinn_sim_t *sim)
{
	if (sim->stat_file_per_node_counters != NULL)
		if (fclose(sim->stat_file_per_node_counters) != 0)
			fprintf(stderr, "Error closing per-node counters data file.\n");
}


void
spinn_sim_stat_close_packet_details(spinn_sim_t *sim)
{
	if (sim->stat_file_packet_details != NULL)
		if (fclose(sim->stat_file_packet_details) != 0)
			fprintf(stderr, "Error closing packet details data file.\n");
}


void
spinn_sim_stat_close_simulator(spinn_sim_t *sim)
{
	if (sim->stat_file_simulator != NULL)
		if (fclose(sim->stat_file_simulator) != 0)
			fprintf(stderr, "Error closing simulator data file.\n");
}


/******************************************************************************
 * Warmup Start Functions
 ******************************************************************************/

void
spinn_sim_stat_start_warmup_simulator(spinn_sim_t *sim)
{
	// Do nothing...
}


/******************************************************************************
 * Warmup End Functions
 ******************************************************************************/

void
spinn_sim_stat_end_warmup_simulator(spinn_sim_t *sim)
{
	// What simulator warmup stats are being counted?
	bool warmup_ticks = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.warmup_ticks");
	bool warmup_duration = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.warmup_duration");
	bool warmup_packet_pool_size = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.warmup_packet_pool_size");
	
	
	
	// Add standard fields, if required
	if (sim->stat_file_simulator != NULL)
		fprint_standard_fields(sim, sim->stat_file_simulator);
	
	// Produce warmup stats
	if (warmup_ticks || warmup_duration || warmup_packet_pool_size) {
		if (warmup_ticks)
			fprintf(sim->stat_file_simulator, "\t%d",
			        scheduler_get_ticks(&(sim->scheduler)) - sim->stat_start_ticks);
		
		if (warmup_duration) {
			struct timeval now;
			gettimeofday(&now, NULL);
			fprintf(sim->stat_file_simulator, "\t%0.3f",
			        (double)(((now.tv_sec - sim->stat_start_time.tv_sec)*1000000L)
			                + now.tv_usec - sim->stat_start_time.tv_usec)
			        / 1000000.0);
		}
		
		if (warmup_packet_pool_size) {
			fprintf(sim->stat_file_simulator, "\t%d",
			        spinn_packet_pool_get_num_packets(&(sim->pool)));
		}
	}
}

/******************************************************************************
 * Sample Start Functions
 ******************************************************************************/

void
spinn_sim_stat_start_sample_global_counters(spinn_sim_t *sim)
{
	// Nothing to do (all counters are those from the per-node counters)
}


void
spinn_sim_stat_start_sample_per_node_counters(spinn_sim_t *sim)
{
	// Reset all counters
	for (size_t i = 0; i < sim->system_size.x*sim->system_size.y; i++) {
		sim->nodes[i].stat_packets_offered   = 0;
		sim->nodes[i].stat_packets_accepted  = 0;
		sim->nodes[i].stat_packets_arrived   = 0;
		sim->nodes[i].stat_packets_dropped   = 0;
		sim->nodes[i].stat_packets_forwarded = 0;
	}
}


void
spinn_sim_stat_start_sample_packet_details(spinn_sim_t *sim)
{
	// Nothing to do
}


void
spinn_sim_stat_start_sample_simulator(spinn_sim_t *sim)
{
	// Nothing to do
}


/******************************************************************************
 * Sample End Functions
 ******************************************************************************/

void
spinn_sim_stat_end_sample_global_counters(spinn_sim_t *sim)
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
	bool glbl_packets_forwarded = spinn_sim_config_lookup_bool(sim,
		"measurements.global_counters.packets_forwarded");
	
	// Dump into file
	if (glbl_packets_offered || glbl_packets_accepted ||
	    glbl_packets_arrived || glbl_packets_dropped ||
	    glbl_packets_forwarded) {
		int stat_packets_offered  = 0;
		int stat_packets_accepted = 0;
		int stat_packets_arrived  = 0;
		int stat_packets_dropped  = 0;
		int stat_packets_forwarded  = 0;
		
		// Sum up all values
		for (size_t i = 0; i < sim->system_size.x*sim->system_size.y; i++) {
			stat_packets_offered   += sim->nodes[i].stat_packets_offered;
			stat_packets_accepted  += sim->nodes[i].stat_packets_accepted;
			stat_packets_arrived   += sim->nodes[i].stat_packets_arrived;
			stat_packets_dropped   += sim->nodes[i].stat_packets_dropped;
			stat_packets_forwarded += sim->nodes[i].stat_packets_forwarded;
		}
	
		fprint_standard_fields(sim, sim->stat_file_global_counters);
		if (glbl_packets_offered)
			fprintf(sim->stat_file_global_counters, "\t%d", stat_packets_offered);
		if (glbl_packets_accepted)
			fprintf(sim->stat_file_global_counters, "\t%d", stat_packets_accepted);
		if (glbl_packets_arrived)
			fprintf(sim->stat_file_global_counters, "\t%d", stat_packets_arrived);
		if (glbl_packets_dropped)
			fprintf(sim->stat_file_global_counters, "\t%d", stat_packets_dropped);
		if (glbl_packets_forwarded)
			fprintf(sim->stat_file_global_counters, "\t%d", stat_packets_forwarded);
		
		fprintf(sim->stat_file_global_counters, "\n");
		
		fflush(sim->stat_file_global_counters);
	}
}


void
spinn_sim_stat_end_sample_per_node_counters(spinn_sim_t *sim)
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
	bool per_node_packets_forwarded = spinn_sim_config_lookup_bool(sim,
		"measurements.per_node_counters.packets_forwarded");
	
	// Dump into file
	if (per_node_packets_offered || per_node_packets_accepted ||
	    per_node_packets_arrived || per_node_packets_dropped ||
	    per_node_packets_forwarded) {
		
		// Iterate over all nodes
		for (int y = 0; y < sim->system_size.y; y++) {
			for (int x = 0; x < sim->system_size.x; x++) {
				spinn_node_t *node = sim->nodes + (x + (sim->system_size.x * y));
				
				// Skip disabled nodes
				if (!node->enabled)
					continue;
				
				fprint_standard_fields(sim, sim->stat_file_per_node_counters);
				fprintf(sim->stat_file_per_node_counters, "\t%d\t%d"
				       , x, y
				       );
				
				if (per_node_packets_offered)
					fprintf(sim->stat_file_per_node_counters, "\t%d", node->stat_packets_offered);
				if (per_node_packets_accepted)
					fprintf(sim->stat_file_per_node_counters, "\t%d", node->stat_packets_accepted);
				if (per_node_packets_arrived)
					fprintf(sim->stat_file_per_node_counters, "\t%d", node->stat_packets_arrived);
				if (per_node_packets_dropped)
					fprintf(sim->stat_file_per_node_counters, "\t%d", node->stat_packets_dropped);
				if (per_node_packets_forwarded)
					fprintf(sim->stat_file_per_node_counters, "\t%d", node->stat_packets_forwarded);
				
				fprintf(sim->stat_file_per_node_counters, "\n");
			}
		}
		
		fflush(sim->stat_file_per_node_counters);
	}
}


void
spinn_sim_stat_end_sample_packet_details(spinn_sim_t *sim)
{
	// Nothing to do
	fflush(sim->stat_file_packet_details);
}


void
spinn_sim_stat_end_sample_simulator(spinn_sim_t *sim)
{
	// What simulator sample stats are being counted?
	bool sample_ticks = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.sample_ticks");
	bool sample_duration = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.sample_duration");
	bool sample_packet_pool_size = spinn_sim_config_lookup_bool(sim,
		"measurements.simulator.sample_packet_pool_size");
	
	
	// Produce sample stats
	if (sample_ticks || sample_duration || sample_packet_pool_size) {
		if (sample_ticks)
			fprintf(sim->stat_file_simulator, "\t%d",
			        scheduler_get_ticks(&(sim->scheduler)) - sim->stat_start_ticks);
		
		if (sample_duration) {
			struct timeval now;
			gettimeofday(&now, NULL);
			fprintf(sim->stat_file_simulator, "\t%0.3f",
			        (double)(((now.tv_sec - sim->stat_start_time.tv_sec)*1000000L)
			                + now.tv_usec - sim->stat_start_time.tv_usec)
			        / 1000000.0);
		}
		
		if (sample_packet_pool_size) {
			fprintf(sim->stat_file_simulator, "\t%d",
			        spinn_packet_pool_get_num_packets(&(sim->pool)));
		}
	}
	
	
	// Terminate with a newline if any field was enabled
	if (sim->stat_file_simulator != NULL) {
		fprintf(sim->stat_file_simulator, "\n");
		fflush(sim->stat_file_simulator);
	}
}



/******************************************************************************
 * Public-facing functions
 ******************************************************************************/

void
spinn_sim_stat_open(spinn_sim_t *sim)
{
	sim->stat_started = false;
	
	spinn_sim_stat_open_global_counters(sim);
	spinn_sim_stat_open_per_node_counters(sim);
	spinn_sim_stat_open_packet_details(sim);
	spinn_sim_stat_open_simulator(sim);
}


void
spinn_sim_stat_close(spinn_sim_t *sim)
{
	spinn_sim_stat_close_global_counters(sim);
	spinn_sim_stat_close_per_node_counters(sim);
	spinn_sim_stat_close_packet_details(sim);
	spinn_sim_stat_close_simulator(sim);
}


void
spinn_sim_stat_start_sample(spinn_sim_t *sim)
{
	sim->stat_started = true;
	gettimeofday(&(sim->stat_start_time), NULL);
	sim->stat_start_ticks = scheduler_get_ticks(&(sim->scheduler));
	
	spinn_sim_stat_start_sample_global_counters(sim);
	spinn_sim_stat_start_sample_per_node_counters(sim);
	spinn_sim_stat_start_sample_packet_details(sim);
	spinn_sim_stat_start_sample_simulator(sim);
}


void
spinn_sim_stat_end_sample(spinn_sim_t *sim)
{
	sim->stat_started = false;
	
	spinn_sim_stat_end_sample_global_counters(sim);
	spinn_sim_stat_end_sample_per_node_counters(sim);
	spinn_sim_stat_end_sample_packet_details(sim);
	spinn_sim_stat_end_sample_simulator(sim);
}


void
spinn_sim_stat_start_warmup(spinn_sim_t *sim)
{
	gettimeofday(&(sim->stat_start_time), NULL);
	sim->stat_start_ticks = scheduler_get_ticks(&(sim->scheduler));
	
	spinn_sim_stat_start_warmup_simulator(sim);
}


void
spinn_sim_stat_end_warmup(spinn_sim_t *sim)
{
	spinn_sim_stat_end_warmup_simulator(sim);
}


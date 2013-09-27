/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_main.c -- A SpiNNaker simulator implemented using TickySim.
 */

#include "config.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <libconfig.h>

#include "scheduler.h"
#include "buffer.h"
#include "arbiter.h"
#include "delay.h"

#include "spinn.h"
#include "spinn_topology.h"
#include "spinn_packet.h"
#include "spinn_router.h"
#include "spinn_stat.h"

/******************************************************************************
 * Data structures
 ******************************************************************************/

/**
 * Resources for a single node in the simulation.
 */
typedef struct spinn_node {
	// The router
	spinn_router_t router;
	
	// The position of the node in the system
	spinn_coord_t position;
	
	// The source/sink for packets
	spinn_packet_gen_t packet_gen;
	spinn_packet_con_t packet_con;
	
	// The arbiters
	arbiter_t arb_n_s;
	arbiter_t arb_e_w;
	arbiter_t arb_ne_sw;
	
	arbiter_t arb_n_s_e_w;
	arbiter_t arb_ne_sw_l;
	
	arbiter_t arb_last;
	
	// Buffers on either side of a delay model which (eventually, via some
	// arbiters) connect to the inputs of the router
	buffer_t front_inputs[7];
	buffer_t back_inputs[7];
	delay_t node_to_node_delay[7];
	
	// Buffer which represents the local core
	buffer_t local_out;
	
	// Buffers in the arbiter tree
	buffer_t arb_n_s_out;
	buffer_t arb_e_w_out;
	buffer_t arb_ne_sw_out;
	
	buffer_t arb_n_s_e_w_out;
	buffer_t arb_ne_sw_l_out;
	
	buffer_t arb_last_out;
} spinn_node_t;


/**
 * Resources used by a SpiNNaker system simulation.
 */
typedef struct spinn_simulation {
	// Configuration file dictating the simulation parameters
	config_t config;
	
	// Scheduler which runs the simulation
	scheduler_t scheduler;
	
	// Packet memory allocation
	spinn_packet_pool_t pool;
	
	// An array of all of the spinnaker nodes
	spinn_node_t *nodes;
	
	// The size of the simulation
	spinn_coord_t system_size;
	
	// Stat counters
	spinn_stat_inj_t  inj_stats;
	spinn_stat_con_t  con_stats;
	spinn_stat_drop_t drop_stats;
} spinn_simulation_t;


/******************************************************************************
 * Config-file utility functions
 ******************************************************************************/

/**
 * Open the config file. On error causes an exit(-1) and produces an error on
 * stderr.
 */
void
spinn_config_init( spinn_simulation_t *sim
                 , const char *filename
                 )
{
	config_init(&(sim->config));
	
	if (!config_read_file(&(sim->config), filename)) {
		fprintf( stderr, "%s:%d: %s\n"
		       , config_error_file(&(sim->config))
		       , config_error_line(&(sim->config))
		       , config_error_text(&(sim->config))
		       );
		exit(-1);
	}
}

/**
 * Free the resources used by the config file parser.
 */
void
spinn_config_destroy( spinn_simulation_t *sim
                    )
{
	config_destroy(&(sim->config));
}


/**
 * Define functions wrapper_name and wrapper_name_default which wrap
 * config_lookup_func. wrapper_name will return the value requested or if it
 * isn't present in the file, crashes with exit(-1) and produces an error
 * message. The wrapper_name_default will silently ignore any error and return
 * the default value given as an argument.
 */
#define DEFINE_CONFIG_WRAPPER( wrapper_name                                    \
                             , wrapper_name_default                            \
                             , config_lookup_func                              \
                             , value_type                                      \
                             )                                                 \
	value_type                                                                   \
	wrapper_name( spinn_simulation_t *sim                                        \
	            , const char         *path                                       \
	            )                                                                \
	{                                                                            \
		value_type value;                                                          \
		if (config_lookup_func( &(sim->config)                                     \
		                      , path                                               \
		                      , &value                                             \
		                      ))                                                   \
			return value;                                                            \
		                                                                           \
		/* The lookup failed */                                                    \
		fprintf( stderr, "Required value '%s' not found in config file!\n"         \
		       , path                                                              \
		       );                                                                  \
		exit(-1);                                                                  \
	}                                                                            \
	                                                                             \
	value_type                                                                   \
	wrapper_name_default( spinn_simulation_t *sim                                \
	                    , const char         *path                               \
	                    , value_type          def_value                          \
	                    )                                                        \
	{                                                                            \
		config_lookup_func( &(sim->config)                                         \
		                  , path                                                   \
		                  , &def_value                                             \
		                  );                                                       \
		return def_value;                                                          \
	}

DEFINE_CONFIG_WRAPPER( spinn_config_lookup_int
                     , spinn_config_lookup_int_default
                     , config_lookup_int
                     , int
                     )

DEFINE_CONFIG_WRAPPER( spinn_config_lookup_float
                     , spinn_config_lookup_float_default
                     , config_lookup_float
                     , double
                     )

DEFINE_CONFIG_WRAPPER( spinn_config_lookup_bool
                     , spinn_config_lookup_bool_default
                     , config_lookup_bool
                     , int
                     )

DEFINE_CONFIG_WRAPPER( spinn_config_lookup_string
                     , spinn_config_lookup_string_default
                     , config_lookup_string
                     , const char *
                     )


/******************************************************************************
 * Node initialisation
 ******************************************************************************/

void
spinn_node_init( spinn_simulation_t *sim
               , spinn_node_t       *node
               , spinn_coord_t       position
               )
{
	node->position = position;
	
	// Create node-to-node links
	int front_input_buffer_length = spinn_config_lookup_int(sim, "model.node_to_node_links.front_buffer_length");
	int back_input_buffer_length = spinn_config_lookup_int(sim, "model.node_to_node_links.back_buffer_length");
	int delay = spinn_config_lookup_int(sim, "model.node_to_node_links.packet_delay");
	for (int i = 0; i < 7; i++) {
		buffer_init(&(node->front_inputs[i]), front_input_buffer_length);
		buffer_init(&(node->back_inputs[i]), back_input_buffer_length);
		delay_init( &(node->node_to_node_delay[i])
		          , &(sim->scheduler)
		          , 1
		          , delay
		          , &(node->front_inputs[i])
		          , &(node->back_inputs[i])
		          );
	}
	
	// Create buffers
	int local_buffer_length = spinn_config_lookup_int(sim, "model.packet_consumer.buffer_length");
	buffer_init(&(node->local_out), local_buffer_length);
	
	int root_buffer_length = spinn_config_lookup_int(sim, "model.arbiter_tree.root.buffer_length");
	int lvl1_buffer_length = spinn_config_lookup_int(sim, "model.arbiter_tree.lvl1.buffer_length");
	int lvl2_buffer_length = spinn_config_lookup_int(sim, "model.arbiter_tree.lvl2.buffer_length");
	buffer_init(&(node->arb_last_out), root_buffer_length);
	buffer_init(&(node->arb_n_s_e_w_out), lvl1_buffer_length);
	buffer_init(&(node->arb_ne_sw_l_out), lvl1_buffer_length);
	buffer_init(&(node->arb_n_s_out), lvl2_buffer_length);
	buffer_init(&(node->arb_e_w_out), lvl2_buffer_length);
	buffer_init(&(node->arb_ne_sw_out), lvl2_buffer_length);
	
	// Create arbiter tree
	int root_period = spinn_config_lookup_int(sim, "model.arbiter_tree.root.period");
	int lvl1_period = spinn_config_lookup_int(sim, "model.arbiter_tree.lvl1.period");
	int lvl2_period = spinn_config_lookup_int(sim, "model.arbiter_tree.lvl2.period");
	
	buffer_t *arb_last_inputs[] = { &(node->arb_n_s_e_w_out) 
	                              , &(node->arb_ne_sw_l_out)
	                              };
	arbiter_init( &(node->arb_last)
	            , &(sim->scheduler)
	            , root_period
	            , arb_last_inputs, 2
	            , &(node->arb_last_out)
	            );
	
	buffer_t *arb_n_s_e_w_inputs[] = { &(node->arb_n_s_out) 
	                                 , &(node->arb_e_w_out)
	                                 };
	arbiter_init( &(node->arb_n_s_e_w)
	            , &(sim->scheduler)
	            , lvl1_period
	            , arb_n_s_e_w_inputs, 2
	            , &(node->arb_n_s_e_w_out)
	            );
	
	buffer_t *arb_ne_sw_l_inputs[] = { &(node->arb_ne_sw_out) 
	                                 , &(node->back_inputs[SPINN_LOCAL])
	                                 };
	arbiter_init( &(node->arb_ne_sw_l)
	            , &(sim->scheduler)
	            , lvl1_period
	            , arb_ne_sw_l_inputs, 2
	            , &(node->arb_ne_sw_l_out)
	            );
	
	buffer_t *arb_n_s_inputs[] = { &(node->back_inputs[SPINN_NORTH]) 
	                             , &(node->back_inputs[SPINN_SOUTH])
	                             };
	arbiter_init( &(node->arb_n_s)
	            , &(sim->scheduler)
	            , lvl2_period
	            , arb_n_s_inputs, 2
	            , &(node->arb_n_s_out)
	            );
	
	buffer_t *arb_e_w_inputs[] = { &(node->back_inputs[SPINN_EAST]) 
	                             , &(node->back_inputs[SPINN_WEST])
	                             };
	arbiter_init( &(node->arb_e_w)
	            , &(sim->scheduler)
	            , lvl2_period
	            , arb_e_w_inputs, 2
	            , &(node->arb_e_w_out)
	            );
	
	buffer_t *arb_ne_sw_inputs[] = { &(node->back_inputs[SPINN_NORTH_EAST]) 
	                               , &(node->back_inputs[SPINN_SOUTH_WEST])
	                               };
	arbiter_init( &(node->arb_ne_sw)
	            , &(sim->scheduler)
	            , lvl2_period
	            , arb_ne_sw_inputs, 2
	            , &(node->arb_ne_sw_out)
	            );
	
	
	// Packet generator
	int gen_period = spinn_config_lookup_int(sim, "model.packet_generator.period");
	double gen_prob = spinn_config_lookup_float(sim, "model.packet_generator.bernoulli_prob");
	const char *gen_cyclic = spinn_config_lookup_string(sim, "model.packet_generator.dist");
	if (strcmp(gen_cyclic, "cyclic") == 0) {
		spinn_packet_gen_cyclic_init( &(node->packet_gen)
		                            , &(sim->scheduler)
		                            , &(node->front_inputs[SPINN_LOCAL])
		                            , &(sim->pool)
		                            , node->position
		                            , sim->system_size
		                            , gen_period
		                            , gen_prob
		                            , spinn_stat_inj_on_packet_gen, (void *)&(sim->inj_stats)
		                            );
	} else if (strcmp(gen_cyclic, "uniform") == 0) {
		spinn_packet_gen_uniform_init( &(node->packet_gen)
		                             , &(sim->scheduler)
		                             , &(node->front_inputs[SPINN_LOCAL])
		                             , &(sim->pool)
		                             , node->position
		                             , sim->system_size
		                             , gen_period
		                             , gen_prob
		                             , spinn_stat_inj_on_packet_gen, (void *)&(sim->inj_stats)
		                             );
	} else {
		fprintf(stderr, "Error: packet_generator.dist not recognised!\n");
		exit(-1);
	}
	
	// Packet consumer
	int con_period = spinn_config_lookup_int(sim, "model.packet_consumer.period");
	double con_prob = spinn_config_lookup_float(sim, "model.packet_consumer.bernoulli_prob");
	spinn_packet_con_init( &(node->packet_con)
	                     , &(sim->scheduler)
	                     , &(node->local_out)
	                     , &(sim->pool)
	                     , con_period
	                     , con_prob
		                   , spinn_stat_con_on_packet_gen, (void *)&(sim->con_stats)
	                     );
	
	// Get a pointer to each of the output buffers
	buffer_t *output_buffers[7] = {0};
	spinn_direction_t directions[] = {
		SPINN_EAST,
		SPINN_NORTH_EAST,
		SPINN_NORTH,
		SPINN_WEST,
		SPINN_SOUTH_WEST,
		SPINN_SOUTH,
	};
	for (int i = 0; i < 6; i++) {
		spinn_coord_t delta = spinn_dir_to_vector(directions[i]);
		spinn_coord_t neighbour_pos;
		neighbour_pos.x = (position.x + delta.x + sim->system_size.x)
		                  % sim->system_size.x;
		neighbour_pos.y = (position.y + delta.y + sim->system_size.y)
		                  % sim->system_size.y;
		spinn_node_t *neighbour = &(sim->nodes[(neighbour_pos.y * sim->system_size.x)
		                                       + neighbour_pos.x]);
		output_buffers[i] = &(neighbour->front_inputs[spinn_opposite(directions[i])]);
	}
	output_buffers[SPINN_LOCAL] = &(node->local_out);
	
	// Set up the router
	int router_period = spinn_config_lookup_int(sim, "model.router.period");
	int router_pipeline_length = spinn_config_lookup_int(sim, "model.router.pipeline_length");
	bool use_emg_routing = spinn_config_lookup_bool(sim, "model.router.use_emergency_routing");
	int first_timeout = spinn_config_lookup_int(sim, "model.router.first_timeout");
	int final_timeout = spinn_config_lookup_int(sim, "model.router.final_timeout");
	spinn_router_init( &(node->router)
	                 , &(sim->scheduler)
	                 , router_period
	                 , router_pipeline_length
	                 , &(node->arb_last_out)
	                 , output_buffers
	                 , node->position
	                 , use_emg_routing
	                 , first_timeout
	                 , final_timeout
	                 , NULL, NULL
		               , spinn_stat_drop_on_drop, (void *)&(sim->drop_stats)
	                 );
}

void
spinn_node_destroy(spinn_node_t *node)
{
	spinn_router_destroy(&(node->router));
	
	spinn_packet_gen_destroy(&(node->packet_gen));
	spinn_packet_con_destroy(&(node->packet_con));
	
	arbiter_destroy(&(node->arb_n_s));
	arbiter_destroy(&(node->arb_e_w));
	arbiter_destroy(&(node->arb_ne_sw));
	arbiter_destroy(&(node->arb_n_s_e_w));
	arbiter_destroy(&(node->arb_ne_sw_l));
	arbiter_destroy(&(node->arb_last));
	
	for (int i = 0; i < 7; i++) {
		buffer_destroy(&(node->front_inputs[i]));
		buffer_destroy(&(node->back_inputs[i]));
		delay_destroy(&(node->node_to_node_delay[i]));
	}
	buffer_destroy(&(node->local_out));
	buffer_destroy(&(node->arb_n_s_out));
	buffer_destroy(&(node->arb_e_w_out));
	buffer_destroy(&(node->arb_ne_sw_out));
	buffer_destroy(&(node->arb_n_s_e_w_out));
	buffer_destroy(&(node->arb_ne_sw_l_out));
	buffer_destroy(&(node->arb_last_out));
}


/******************************************************************************
 * High-level simulation functions
 ******************************************************************************/

void
spinn_simulation_init(spinn_simulation_t *sim, const char *config_filename)
{
	spinn_config_init(sim, config_filename);
	
	scheduler_init(&(sim->scheduler));
	spinn_packet_pool_init(&(sim->pool));
	
	// Get the size of the system
	sim->system_size.x = spinn_config_lookup_int(sim, "model.system_size.width");
	sim->system_size.y = spinn_config_lookup_int(sim, "model.system_size.height");
	
	// Reset the stat counters
	sim->inj_stats.num_offered  = 0;
	sim->inj_stats.num_accepted = 0;
	sim->con_stats.num_packets  = 0;
	sim->drop_stats.num_packets = 0;
	
	// Create the required number of nodes
	sim->nodes = calloc( sim->system_size.x*sim->system_size.y
	                   , sizeof(spinn_node_t)
	                   );
	assert(sim->nodes != NULL);
	for (int y = 0; y < sim->system_size.y; y++) {
		for (int x = 0; x < sim->system_size.x; x++) {
			int i = (y * sim->system_size.x) + x;
			spinn_node_init(sim, &(sim->nodes[i]), (spinn_coord_t){x,y});
		}
	}
}


void
spinn_simulation_run(spinn_simulation_t *sim)
{
	ticks_t warmup_duration = spinn_config_lookup_int(sim, "simulation.warmup_duration");
	ticks_t sample_duration = spinn_config_lookup_int(sim, "simulation.sample_duration");
	int     num_samples     = spinn_config_lookup_int(sim, "simulation.num_samples");
	
	// The last time a debug message was printed
	time_t last_debug = time(NULL);
	
	// The number of ticks when the last debug message was printed
	ticks_t last_num_ticks = 0;
	
	fprintf(stderr, "Warmup starting...\n");
	
	// Warmup
	for (ticks_t t = 0; t < warmup_duration; t++) {
		scheduler_tick_tock(&(sim->scheduler));
		
		time_t now = time(NULL);
		if (last_debug != now) {
			fprintf(stderr, "Warmup: %3d%% (%7d/%7d, %d ticks/s)\n"
			              , (t*100) / warmup_duration
			              , t, warmup_duration
			              , scheduler_get_ticks(&(sim->scheduler)) - last_num_ticks
			              );
			last_debug     = now;
			last_num_ticks = scheduler_get_ticks(&(sim->scheduler));
		}
	}
	
	// Take samples
	for (int i = 0; i < num_samples; i++) {
		// TODO: Reset stats
		
		fprintf(stderr, "\nSample %2d/%2d starting...\n"
		              , i+1, num_samples
		              );
		
		// Run the sample
		for (ticks_t t = 0; t < sample_duration; t++) {
			scheduler_tick_tock(&(sim->scheduler));
			
			time_t now = time(NULL);
			if (last_debug != now) {
				fprintf(stderr, "Sample %2d/%2d: %3d%% (%7d/%7d, %d ticks/s)\n"
				              , i+1, num_samples
				              , (t*100) / warmup_duration
				              , t, warmup_duration
			                , scheduler_get_ticks(&(sim->scheduler)) - last_num_ticks
				              );
				last_debug     = now;
				last_num_ticks = scheduler_get_ticks(&(sim->scheduler));
			}
		}
		
		// TODO: Dump stats
	}
	
	fprintf(stderr, "\nSimulation terminated.\n");
}


void
spinn_simulation_destroy(spinn_simulation_t *sim)
{
	scheduler_destroy(&(sim->scheduler));
	spinn_packet_pool_destroy(&(sim->pool));
	
	for (int i = 0; i < sim->system_size.x*sim->system_size.y; i++)
		spinn_node_destroy(&(sim->nodes[i]));
	free(sim->nodes);
	
	spinn_config_destroy(sim);
}


/******************************************************************************
 * World starts here
 ******************************************************************************/

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s config_file\n", argv[0]);
		return -1;
	}
	
	spinn_simulation_t sim;
	spinn_simulation_init(&sim, argv[1]);
	spinn_simulation_run(&sim);
	spinn_simulation_destroy(&sim);
}

/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_sim_model.c -- Code for initialising models of a complete spinnaker
 * system in a spinn_sim_t.
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "scheduler.h"
#include "buffer.h"
#include "arbiter.h"
#include "delay.h"

#include "spinn.h"
#include "spinn_topology.h"
#include "spinn_packet.h"
#include "spinn_router.h"

#include "spinn_sim.h"
#include "spinn_sim_model.h"
#include "spinn_sim_config.h"
#include "spinn_sim_stat.h"

/******************************************************************************
 * Initialisation for values which can be changed mid-simulation (to save
 * duplication in the spinn_node_init and spinn_sim_model_update functions)
 ******************************************************************************/

static void
configure_node_packet_gen(spinn_node_t *node)
{
	// Set temporal distribution
	const char *gen_temporal_dist
		= spinn_sim_config_lookup_string(node->sim, "model.packet_generator.temporal.dist");
	if (strcmp(gen_temporal_dist, "bernoulli") == 0) {
		double prob
			= spinn_sim_config_lookup_float(node->sim, "model.packet_generator.temporal.bernoulli_prob");
		spinn_packet_gen_set_temporal_dist_bernoulli(&(node->packet_gen), prob);
	} else if (strcmp(gen_temporal_dist, "periodic") == 0) {
		int interval
			= spinn_sim_config_lookup_int(node->sim, "model.packet_generator.temporal.periodic_interval");
		spinn_packet_gen_set_temporal_dist_periodic(&(node->packet_gen), interval);
	} else {
		fprintf(stderr, "Error: model.packet_generator.temporal.dist not recognised!\n");
		exit(-1);
	}
	
	// Set spatial distribution
	const char *gen_spatial_dist
		= spinn_sim_config_lookup_string(node->sim, "model.packet_generator.spatial.dist");
	if (strcmp(gen_spatial_dist, "uniform") == 0) {
		spinn_packet_gen_set_spatial_dist_uniform(&(node->packet_gen));
	} else if (strcmp(gen_spatial_dist, "cyclic") == 0) {
		spinn_packet_gen_set_spatial_dist_cyclic(&(node->packet_gen));
	} else {
		fprintf(stderr, "Error: model.packet_generator.spatial.dist not recognised!\n");
		exit(-1);
	}
	
	// Set whether local messages can be sent
	bool gen_allow_local = spinn_sim_config_lookup_bool(node->sim, "model.packet_generator.spatial.allow_local");
	spinn_packet_gen_set_allow_local(&(node->packet_gen), gen_allow_local);
}

static void
configure_node_packet_con(spinn_node_t *node)
{
	// Set temporal distribution
	const char *con_temporal_dist
		= spinn_sim_config_lookup_string(node->sim, "model.packet_consumer.temporal.dist");
	if (strcmp(con_temporal_dist, "bernoulli") == 0) {
		double prob
			= spinn_sim_config_lookup_float(node->sim, "model.packet_consumer.temporal.bernoulli_prob");
		spinn_packet_con_set_temporal_dist_bernoulli(&(node->packet_con), prob);
	} else if (strcmp(con_temporal_dist, "periodic") == 0) {
		int interval
			= spinn_sim_config_lookup_int(node->sim, "model.packet_consumer.temporal.periodic_interval");
		spinn_packet_con_set_temporal_dist_periodic(&(node->packet_con), interval);
	} else {
		fprintf(stderr, "Error: model.packet_consumer.temporal.dist not recognised!\n");
		exit(-1);
	}
}

static void
configure_node_to_node_links(spinn_node_t *node)
{
	// Change the node-to-node delays
	int delay_ticks = spinn_sim_config_lookup_int(node->sim, "model.node_to_node_links.packet_delay");
	for (int i = 0; i < 6; i++)
		delay_set_delay(&(node->delays[i]), delay_ticks);
}

/******************************************************************************
 * Node initialisation
 ******************************************************************************/

static void
spinn_node_init( spinn_sim_t   *sim
               , spinn_node_t  *node
               , spinn_coord_t  position
               )
{
	node->sim = sim;
	
	node->position = position;
	
	// Create node-to-node link buffers
	int input_buffer_length = spinn_sim_config_lookup_int(sim, "model.node_to_node_links.input_buffer_length");
	int output_buffer_length = spinn_sim_config_lookup_int(sim, "model.node_to_node_links.output_buffer_length");
	for (int i = 0; i < 6; i++) {
		buffer_init(&(node->input_buffers[i]), input_buffer_length);
		buffer_init(&(node->output_buffers[i]), output_buffer_length);
	}
	
	// Create buffer for the local gen/con links
	int gen_buffer_length = spinn_sim_config_lookup_int(sim, "model.packet_generator.buffer_length");
	int con_pre_delay_buffer_length = spinn_sim_config_lookup_int(sim, "model.packet_consumer.pre_delay_buffer_length");
	int con_post_delay_buffer_length = spinn_sim_config_lookup_int(sim, "model.packet_consumer.post_delay_buffer_length");
	buffer_init(&(node->gen_buffer),            gen_buffer_length);
	buffer_init(&(node->con_pre_delay_buffer),  con_pre_delay_buffer_length);
	buffer_init(&(node->con_post_delay_buffer), con_post_delay_buffer_length);
	
	// Create buffers for the arbiter tree
	int root_buffer_length = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.root.buffer_length");
	int lvl1_buffer_length = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.lvl1.buffer_length");
	int lvl2_buffer_length = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.lvl2.buffer_length");
	buffer_init(&(node->arb_last_out), root_buffer_length);
	buffer_init(&(node->arb_e_ne_n_w_out), lvl1_buffer_length);
	buffer_init(&(node->arb_sw_s_l_out), lvl1_buffer_length);
	buffer_init(&(node->arb_e_ne_out), lvl2_buffer_length);
	buffer_init(&(node->arb_n_w_out), lvl2_buffer_length);
	buffer_init(&(node->arb_sw_s_out), lvl2_buffer_length);
	
	// Create the consumer delay element
	int con_delay = spinn_sim_config_lookup_int(sim, "model.packet_consumer.delay");
	delay_init( &(node->con_delay)
	          , &(sim->scheduler)
	          , 1
	          , con_delay
	          , &(node->con_pre_delay_buffer)
	          , &(node->con_post_delay_buffer)
	          );
	
	// Create arbiter tree which looks like this (with the levels indicated
	// below):
	//
	//        |\                                         ,------ KEY --------,
	//     E--| |_,--,_                                  |                   |
	//    NE--| | '--' |    |\                           |             |\    |
	//        |/       `----| |_,--,_                    | Merger:  ---| |__ |
	//                 ,----| | '--' |                   |          ---| |   |
	//        |\       |    |/       |   |\              |             |/    |
	//     N--| |_,--,_|             '---| |_,--,___     |                   |
	//     W--| | '--'               ,---| | '--'        | Buffer:  __,--,__ |
	//        |/            |\       |   |/              |            '--'   |
	//                 ,----| |_,--,_|                   '-------------------'
	//        |\       | ,--| | '--'
	//    SW--| |_,--,_| |  |/
	//     S--| | '--'   |
	//        |/         |
	//                   |
	//     L-------------'
	//
	//      `----v----'   `----v----'    `----v----'
	//         Lvl2           Lvl1           Root
	int root_period = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.root.period");
	int lvl1_period = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.lvl1.period");
	int lvl2_period = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.lvl2.period");
	
	// Root
	buffer_t *arb_last_inputs[] = { &(node->arb_e_ne_n_w_out) 
	                              , &(node->arb_sw_s_l_out)
	                              };
	arbiter_init( &(node->arb_last)
	            , &(sim->scheduler)
	            , root_period
	            , arb_last_inputs, 2
	            , &(node->arb_last_out)
	            );
	
	// Lvl 1
	buffer_t *arb_e_ne_n_w_inputs[] = { &(node->arb_e_ne_out) 
	                                  , &(node->arb_n_w_out)
	                                  };
	arbiter_init( &(node->arb_e_ne_n_w)
	            , &(sim->scheduler)
	            , lvl1_period
	            , arb_e_ne_n_w_inputs, 2
	            , &(node->arb_e_ne_n_w_out)
	            );
	
	buffer_t *arb_sw_s_l_inputs[] = { &(node->arb_sw_s_out) 
	                                , &(node->gen_buffer)
	                                };
	arbiter_init( &(node->arb_sw_s_l)
	            , &(sim->scheduler)
	            , lvl1_period
	            , arb_sw_s_l_inputs, 2
	            , &(node->arb_sw_s_l_out)
	            );
	
	// Lvl 2
	buffer_t *arb_e_ne_inputs[] = { &(node->input_buffers[SPINN_EAST]) 
	                              , &(node->input_buffers[SPINN_NORTH_EAST])
	                              };
	arbiter_init( &(node->arb_e_ne)
	            , &(sim->scheduler)
	            , lvl2_period
	            , arb_e_ne_inputs, 2
	            , &(node->arb_e_ne_out)
	            );
	
	buffer_t *arb_n_w_inputs[] = { &(node->input_buffers[SPINN_NORTH]) 
	                             , &(node->input_buffers[SPINN_WEST])
	                             };
	arbiter_init( &(node->arb_n_w)
	            , &(sim->scheduler)
	            , lvl2_period
	            , arb_n_w_inputs, 2
	            , &(node->arb_n_w_out)
	            );
	
	buffer_t *arb_sw_s_inputs[] = { &(node->input_buffers[SPINN_SOUTH_WEST]) 
	                              , &(node->input_buffers[SPINN_SOUTH])
	                              };
	arbiter_init( &(node->arb_sw_s)
	            , &(sim->scheduler)
	            , lvl2_period
	            , arb_sw_s_inputs, 2
	            , &(node->arb_sw_s_out)
	            );
	
	
	// Packet generator
	int gen_period = spinn_sim_config_lookup_int(sim, "model.packet_generator.period");
	bool gen_allow_local = spinn_sim_config_lookup_bool(sim, "model.packet_generator.spatial.allow_local");
	spinn_packet_gen_init( &(node->packet_gen)
	                     , &(sim->scheduler)
	                     , &(node->gen_buffer)
	                     , &(sim->pool)
	                     , node->position
	                     , sim->system_size
	                     , gen_period
	                     , gen_allow_local
	                     , spinn_sim_stat_on_packet_gen, (void *)node
	                     );

	configure_node_packet_gen(node);
	
	// Packet consumer
	int con_period = spinn_sim_config_lookup_int(sim, "model.packet_consumer.period");
	spinn_packet_con_init( &(node->packet_con)
	                     , &(sim->scheduler)
	                     , &(node->con_post_delay_buffer)
	                     , &(sim->pool)
	                     , con_period
	                     , spinn_sim_stat_on_packet_con, (void *)node
	                     );
	
	configure_node_packet_con(node);
	
	// Get a pointer to each of the output buffers
	buffer_t *output_buffers[7];
	for (int i = 0; i < 6; i++) {
		output_buffers[i] = &(node->output_buffers[i]);
	}
	output_buffers[SPINN_LOCAL] = &(node->con_pre_delay_buffer);
	
	// Set up the router
	int router_period = spinn_sim_config_lookup_int(sim, "model.router.period");
	int router_pipeline_length = spinn_sim_config_lookup_int(sim, "model.router.pipeline_length");
	bool use_emg_routing = spinn_sim_config_lookup_bool(sim, "model.router.use_emergency_routing");
	int first_timeout = spinn_sim_config_lookup_int(sim, "model.router.first_timeout");
	int final_timeout = spinn_sim_config_lookup_int(sim, "model.router.final_timeout");
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
	                 , spinn_sim_stat_on_drop, (void *)node
	                 );
}


void
spinn_node_destroy(spinn_node_t *node)
{
	spinn_router_destroy(&(node->router));
	
	spinn_packet_gen_destroy(&(node->packet_gen));
	spinn_packet_con_destroy(&(node->packet_con));
	
	arbiter_destroy(&(node->arb_e_ne));
	arbiter_destroy(&(node->arb_n_w));
	arbiter_destroy(&(node->arb_sw_s));
	arbiter_destroy(&(node->arb_e_ne_n_w));
	arbiter_destroy(&(node->arb_sw_s_l));
	arbiter_destroy(&(node->arb_last));
	
	for (int i = 0; i < 6; i++) {
		buffer_destroy(&(node->input_buffers[i]));
		buffer_destroy(&(node->output_buffers[i]));
	}
	buffer_destroy(&(node->gen_buffer));
	buffer_destroy(&(node->con_pre_delay_buffer));
	buffer_destroy(&(node->con_post_delay_buffer));
	buffer_destroy(&(node->arb_e_ne_out));
	buffer_destroy(&(node->arb_n_w_out));
	buffer_destroy(&(node->arb_sw_s_out));
	buffer_destroy(&(node->arb_e_ne_n_w_out));
	buffer_destroy(&(node->arb_sw_s_l_out));
	buffer_destroy(&(node->arb_last_out));
	
	delay_destroy(&(node->con_delay));
}


/******************************************************************************
 * System-level simulation functions
 ******************************************************************************/

void
spinn_sim_model_init(spinn_sim_t *sim)
{
	scheduler_init(&(sim->scheduler));
	spinn_packet_pool_init(&(sim->pool));
	
	// Get the size of the system
	sim->system_size.x = spinn_sim_config_lookup_int(sim, "model.system_size.width");
	sim->system_size.y = spinn_sim_config_lookup_int(sim, "model.system_size.height");
	
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
	
	// Wire-up the nodes with delays
	int delay_ticks = spinn_sim_config_lookup_int(sim, "model.node_to_node_links.packet_delay");
	for (int y = 0; y < sim->system_size.y; y++) {
		for (int x = 0; x < sim->system_size.x; x++) {
			spinn_node_t *node = &(sim->nodes[(y * sim->system_size.x) + x]);
			
			spinn_direction_t directions[] = {
			        SPINN_EAST,
			        SPINN_NORTH_EAST,
			        SPINN_NORTH,
			        SPINN_WEST,
			        SPINN_SOUTH_WEST,
			        SPINN_SOUTH,
			};
			for (int i = 0; i < 6; i++) {
				// Find the node in this direction
				spinn_coord_t delta = spinn_dir_to_vector(directions[i]);
				spinn_coord_t neighbour_pos;
				neighbour_pos.x = (x + delta.x + sim->system_size.x)
				                  % sim->system_size.x;
				neighbour_pos.y = (y + delta.y + sim->system_size.y)
				                  % sim->system_size.y;
				spinn_node_t *neighbour = &(sim->nodes[(neighbour_pos.y * sim->system_size.x)
				                                       + neighbour_pos.x]);
				
				// Find the input connected to this node's output
				buffer_t *input_buffer = &(neighbour->input_buffers[spinn_opposite(directions[i])]);
				buffer_t *output_buffer = &(node->output_buffers[i]);
				
				// Set up the delay
				delay_init( &(node->delays[i])
				          , &(sim->scheduler)
				          , 1
				          , delay_ticks
				          , output_buffer
				          , input_buffer
				          );
			}
		}
	}
}



void
spinn_sim_model_destroy(spinn_sim_t *sim)
{
	scheduler_destroy(&(sim->scheduler));
	spinn_packet_pool_destroy(&(sim->pool));
	
	for (int i = 0; i < sim->system_size.x*sim->system_size.y; i++) {
		spinn_node_destroy(&(sim->nodes[i]));
		for (int j = 0; j < 6; j++)
			delay_destroy(&(sim->nodes[i].delays[j]));
	}
	free(sim->nodes);
}


/******************************************************************************
 * System-level hot-update
 ******************************************************************************/

void
spinn_sim_model_update(spinn_sim_t *sim)
{
	for (int y = 0; y < sim->system_size.y; y++) {
		for (int x = 0; x < sim->system_size.x; x++) {
			spinn_node_t *node = &(sim->nodes[(y * sim->system_size.x) + x]);
			
			configure_node_packet_gen(node);
			configure_node_packet_con(node);
			configure_node_to_node_links(node);
		}
	}
}

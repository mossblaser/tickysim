/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_sim.c -- SpiNNaker simulation functions for simulating a whole system.
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
#include "spinn_sim_config.h"
#include "spinn_sim_stat.h"

/******************************************************************************
 * Node initialisation
 ******************************************************************************/

void
spinn_node_init( spinn_sim_t   *sim
               , spinn_node_t  *node
               , spinn_coord_t  position
               )
{
	node->sim = sim;
	
	node->position = position;
	
	// Create node-to-node links as:
	//     ,--------------,   ,-------,   ,-------------,
	//  -->| Front Buffer |-->| Delay |-->| Back Buffer |-->
	//     '--------------'   '-------'   '-------------'
	int front_input_buffer_length = spinn_sim_config_lookup_int(sim, "model.node_to_node_links.front_buffer_length");
	int back_input_buffer_length = spinn_sim_config_lookup_int(sim, "model.node_to_node_links.back_buffer_length");
	int delay = spinn_sim_config_lookup_int(sim, "model.node_to_node_links.packet_delay");
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
	
	// Create buffer for the local link
	int local_buffer_length = spinn_sim_config_lookup_int(sim, "model.packet_consumer.buffer_length");
	buffer_init(&(node->local_out), local_buffer_length);
	
	// Create buffers for the arbiter tree
	int root_buffer_length = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.root.buffer_length");
	int lvl1_buffer_length = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.lvl1.buffer_length");
	int lvl2_buffer_length = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.lvl2.buffer_length");
	buffer_init(&(node->arb_last_out), root_buffer_length);
	buffer_init(&(node->arb_n_s_e_w_out), lvl1_buffer_length);
	buffer_init(&(node->arb_ne_sw_l_out), lvl1_buffer_length);
	buffer_init(&(node->arb_n_s_out), lvl2_buffer_length);
	buffer_init(&(node->arb_e_w_out), lvl2_buffer_length);
	buffer_init(&(node->arb_ne_sw_out), lvl2_buffer_length);
	
	// Create arbiter tree which looks like this (with the levels indicated
	// below):
	//
	//        |\                                         ,------ KEY --------,
	//     N--| |_,--,_                                  |                   |
	//     S--| | '--' |    |\                           |             |\    |
	//        |/       `----| |_,--,_                    | Merger:  ---| |__ |
	//                 ,----| | '--' |                   |          ---| |   |
	//        |\       |    |/       |   |\              |             |/    |
	//     E--| |_,--,_|             '---| |_,--,___     |                   |
	//     W--| | '--'               ,---| | '--'        | Buffer:  __,--,__ |
	//        |/            |\       |   |/              |            '--'   |
	//                 ,----| |_,--,_|                   '-------------------'
	//        |\       | ,--| | '--'
	//    NE--| |_,--,_| |  |/
	//    SW--| | '--'   |
	//        |/         |
	//                   |
	//     L-------------'
	//
	//      `----v----'   `----v----'    `----v----'
	//         Lvl2           Lvl1           Root
	int root_period = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.root.period");
	int lvl1_period = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.lvl1.period");
	int lvl2_period = spinn_sim_config_lookup_int(sim, "model.arbiter_tree.lvl2.period");
	
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
	int gen_period = spinn_sim_config_lookup_int(sim, "model.packet_generator.period");
	double gen_prob = spinn_sim_config_lookup_float(sim, "model.packet_generator.bernoulli_prob");
	const char *gen_cyclic = spinn_sim_config_lookup_string(sim, "model.packet_generator.dist");
	if (strcmp(gen_cyclic, "cyclic") == 0) {
		spinn_packet_gen_cyclic_init( &(node->packet_gen)
		                            , &(sim->scheduler)
		                            , &(node->front_inputs[SPINN_LOCAL])
		                            , &(sim->pool)
		                            , node->position
		                            , sim->system_size
		                            , gen_period
		                            , gen_prob
		                            , spinn_sim_stat_on_packet_gen, (void *)node
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
		                             , spinn_sim_stat_on_packet_gen, (void *)node
		                             );
	} else {
		fprintf(stderr, "Error: packet_generator.dist not recognised!\n");
		exit(-1);
	}
	
	// Packet consumer
	int con_period = spinn_sim_config_lookup_int(sim, "model.packet_consumer.period");
	double con_prob = spinn_sim_config_lookup_float(sim, "model.packet_consumer.bernoulli_prob");
	spinn_packet_con_init( &(node->packet_con)
	                     , &(sim->scheduler)
	                     , &(node->local_out)
	                     , &(sim->pool)
	                     , con_period
	                     , con_prob
		                   , spinn_sim_stat_on_packet_con, (void *)node
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
spinn_sim_init(spinn_sim_t *sim, const char *config_filename)
{
	spinn_sim_config_init(sim, config_filename);
	
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
	
	// Set up stat counting resources
	spinn_sim_stat_init(sim);
}


void
spinn_sim_run(spinn_sim_t *sim)
{
	ticks_t warmup_duration = spinn_sim_config_lookup_int(sim, "simulation.warmup_duration");
	ticks_t sample_duration = spinn_sim_config_lookup_int(sim, "simulation.sample_duration");
	int     num_samples     = spinn_sim_config_lookup_int(sim, "simulation.num_samples");
	
	// The last time a debug message was printed
	time_t last_debug = time(NULL);
	
	// The number of ticks when the last debug message was printed
	ticks_t last_num_ticks = 0;
	
	fprintf(stderr, "Warmup starting...\n");
	
	// Warmup
	sim->repeat_num = -1;
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
	for (sim->repeat_num = 0; sim->repeat_num < num_samples; sim->repeat_num++) {
		spinn_sim_stat_start(sim);
		
		fprintf(stderr, "\nSample %2d/%2d starting...\n"
		              , sim->repeat_num+1, num_samples
		              );
		
		// Run the sample
		for (ticks_t t = 0; t < sample_duration; t++) {
			scheduler_tick_tock(&(sim->scheduler));
			
			time_t now = time(NULL);
			if (last_debug != now) {
				fprintf(stderr, "Sample %2d/%2d: %3d%% (%7d/%7d, %d ticks/s)\n"
				              , sim->repeat_num+1, num_samples
				              , (t*100) / warmup_duration
				              , t, warmup_duration
			                , scheduler_get_ticks(&(sim->scheduler)) - last_num_ticks
				              );
				last_debug     = now;
				last_num_ticks = scheduler_get_ticks(&(sim->scheduler));
			}
		}
		
		spinn_sim_stat_end(sim);
	}
	
	fprintf(stderr, "\nSimulation completed.\n");
}


void
spinn_sim_destroy(spinn_sim_t *sim)
{
	spinn_sim_stat_destroy(sim);
	
	scheduler_destroy(&(sim->scheduler));
	spinn_packet_pool_destroy(&(sim->pool));
	
	for (int i = 0; i < sim->system_size.x*sim->system_size.y; i++)
		spinn_node_destroy(&(sim->nodes[i]));
	free(sim->nodes);
	
	spinn_sim_config_destroy(sim);
}

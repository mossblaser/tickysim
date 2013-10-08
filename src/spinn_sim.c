/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_sim.c -- SpiNNaker simulation functions for simulating a whole system.
 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "scheduler.h"

#include "spinn_sim.h"
#include "spinn_sim_model.h"
#include "spinn_sim_config.h"
#include "spinn_sim_stat.h"

/******************************************************************************
 * Init/Destroy
 ******************************************************************************/

void
spinn_sim_init(spinn_sim_t *sim, const char *config_filename, int argc, char *argv[])
{
	// Load the configuration
	spinn_sim_config_init(sim, config_filename, argc, argv);
	
	// Seed the simulation (default to the time as a seed)
	srand(spinn_sim_config_lookup_int64_default(sim, "experiment.seed", time(NULL)));
	
	// Set up stat counting resources
	spinn_sim_stat_open(sim);
}


void
spinn_sim_destroy(spinn_sim_t *sim)
{
	spinn_sim_stat_close(sim);
	
	spinn_sim_config_destroy(sim);
}

/******************************************************************************
 * Experiment/Simulation Control
 ******************************************************************************/

/**
 * Run the simulator for a certain number of ticks producing simulation
 * performance metrics on stderr every second. If running interactively then the
 * status messages are cleared after simulation is completed.
 */
void
spinn_sim_run_ticks(spinn_sim_t *sim, int num_ticks)
{
	// Record the last time the simulation status was output
	time_t last_debug_time = 0;
	
	// For calculating the number of simulator ticks elapsed since the last status
	// was output.
	ticks_t last_num_ticks = scheduler_get_ticks(&(sim->scheduler));
	
	// Save the position of the cursor to allow cursor to be moved when in a
	// terminal
	if (isatty(STDERR_FILENO))
		fprintf(stderr, "\033[s");
	
	// Run the simulation for the requested number of ticks
	for (ticks_t t = 0; t < num_ticks; t++) {
		scheduler_tick_tock(&(sim->scheduler));
		
		// Show the status line once per cycle
		time_t now = time(NULL);
		if (last_debug_time != now) {
			fprintf(stderr, "%s%3d%% (%6d/%6d, %d ticks/s)%s"
			              , isatty(STDERR_FILENO) ? "\033[u\033[K" : ""
			              , (t*100) / num_ticks
			              , t, num_ticks
		                , scheduler_get_ticks(&(sim->scheduler)) - last_num_ticks
			              , isatty(STDERR_FILENO) ? "" : "\n"
			              );
			last_debug_time = now;
			last_num_ticks  = scheduler_get_ticks(&(sim->scheduler));
		}
	}
	
	// Erase the status line (if running in a terminal)
	if (isatty(STDERR_FILENO))
		fprintf(stderr, "\033[u\033[K");
}


void
spinn_sim_run(spinn_sim_t *sim)
{
	int num_groups = spinn_sim_config_get_num_exp_groups(sim);
	
	// Has the model been initialised?
	bool model_initialised = false;
	
	// Has the simulation been running for some period?
	bool model_hot = false;
	
	bool cold_group = spinn_sim_config_lookup_bool(sim, "experiment.cold_group");
	
	// Perform experiments for each group
	for (sim->cur_group = 0; sim->cur_group < num_groups; sim->cur_group++) {
		// Set up the parameters ready for this group (this is done here as it may
		// change e.g. the number of samples as an independent variable)
		spinn_sim_config_set_exp_group(sim, sim->cur_group);
		
		// Update all parameters which can be set while the simulation is hot
		if (model_initialised) {
			spinn_sim_model_update(sim);
		}
		
		bool cold_sample = spinn_sim_config_lookup_bool(sim, "experiment.cold_sample");
		
		int num_samples     = spinn_sim_config_lookup_int(sim, "experiment.num_samples");
		int sample_duration = spinn_sim_config_lookup_int(sim, "experiment.sample_duration");
		
		// If using cold_group mode then the model should be reset as we're starting
		// a new group
		if (cold_group && model_initialised) {
			spinn_sim_model_destroy(sim);
			model_initialised = false;
			model_hot = false;
		}
		
		fprintf(stderr, "Group %2d/%2d:\n"
		              , sim->cur_group + 1
		              , num_groups
		              );
		
		// Perform samples for this group
		for (sim->cur_sample = 0; sim->cur_sample < num_samples; sim->cur_sample++) {
			// If cold_sample is in use then we should reset any running model
			if (cold_sample && model_initialised) {
				spinn_sim_model_destroy(sim);
				model_initialised = false;
				model_hot = false;
			}
			
			// We're about to run a sample, make sure the model is initialised
			if (!model_initialised) {
				spinn_sim_model_init(sim);
				model_initialised = true;
			}
			
			// Warm-up if we're doing cold sampling or if this is the first sample of
			// a group (and thus we might need to hot-start after the previous group)
			if (cold_sample || sim->cur_sample == 0) {
				int warmup_time = model_hot ? spinn_sim_config_lookup_int(sim, "experiment.warmup_duration.hot")
				                            : spinn_sim_config_lookup_int(sim, "experiment.warmup_duration.cold")
				                            ;
				fprintf(stderr, "  Warming up %s  "
				              , model_hot ? "from hot " : "from cold"
				              );
				spinn_sim_run_ticks(sim, warmup_time);
				fprintf(stderr, "100%%\n");
				model_hot = true;
			}
			
			// Perform the sample
			fprintf(stderr, "  Sample %2d/%2d          "
			              , sim->cur_sample + 1
			              , num_samples
			              );
			spinn_sim_stat_start_sample(sim);
			spinn_sim_run_ticks(sim, sample_duration);
			spinn_sim_stat_end_sample(sim);
			fprintf(stderr, "100%%\n");
		}
	}
	
	// Destroy the last model used
	if (model_initialised)
		spinn_sim_model_destroy(sim);
	
	fprintf(stderr, "Simulation completed.\n");
}

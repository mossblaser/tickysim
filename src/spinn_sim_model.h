/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_sim_model.h -- Code for initialising models of a complete spinnaker
 * system in a spinn_sim_t.
 */

#ifndef SPINN_SIM_MODEL_H
#define SPINN_SIM_MODEL_H

#include "spinn_sim.h"

/**
 * Initialise the system model using values in the config.
 */
void spinn_sim_model_init(spinn_sim_t *sim);


/**
 * Update all parameters of the model which can be updated without destroying
 * and re-initialising.
 */
void spinn_sim_model_update(spinn_sim_t *sim);

/**
 * Clean up the system model.
 */
void spinn_sim_model_destroy(spinn_sim_t *sim);

#endif


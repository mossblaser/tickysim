/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_sim_stat.h -- Monitoring functions for the SpiNNaker simulation.
 */

#ifndef SPINN_SIM_STAT_H
#define SPINN_SIM_STAT_H

#include "spinn_sim.h"

/******************************************************************************
 * Callback functions
 ******************************************************************************/

/**
 * Callback for the packet generators. Expects a reference to the simulation
 * node as the data argument.
 */
void *spinn_sim_stat_on_packet_gen(spinn_packet_t *packet, void *node);

/**
 * Callback for the packet consumers. Expects a reference to the simulation node
 * as the data argument.
 */
void spinn_sim_stat_on_packet_con(spinn_packet_t *packet, void *node);

/**
 * Callback for the router's on-drop event. Expects a reference to the
 * simulation node as the data argument.
 */
void spinn_sim_stat_on_drop(spinn_router_t *router, spinn_packet_t *packet, void *node);


/******************************************************************************
 * Stat management functions
 ******************************************************************************/


/**
 * Initialise all stat monitoring components in the simulation. Opens all output
 * files for writing (erasing previous data).
 */
void spinn_sim_stat_open(spinn_sim_t *sim);


/**
 * Clean up all stat monitoring components in the simulation. Closes and flushes
 * all output files/buffers.
 */
void spinn_sim_stat_close(spinn_sim_t *sim);


/**
 * Start monitoring the simulation.
 */
void spinn_sim_stat_start_sample(spinn_sim_t *sim);


/**
 * End the monitoring of a particular sample in the simulation.
 */
void spinn_sim_stat_end_sample(spinn_sim_t *sim);


/**
 * Start monitoring the simulation's warmup.
 */
void spinn_sim_stat_start_warmup(spinn_sim_t *sim);


/**
 * End the monitoring of a particular warmup period in the simulation.
 */
void spinn_sim_stat_end_warmup(spinn_sim_t *sim);


#endif

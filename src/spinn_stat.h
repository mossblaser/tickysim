/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_stat.h -- Statistic monitoring functions for various SpiNNaker models.
 */

#ifndef SPINN_STAT_H
#define SPINN_STAT_H

#include "spinn_packet.h"

/******************************************************************************
 * Packet injection stats
 ******************************************************************************/

/**
 * Provides two simple counters of the number of packets offered and the number
 * of packets accepted.
 */
typedef struct spinn_stat_inj {
	int num_offered;
	int num_accepted;
} spinn_stat_inj_t;


/**
 * A callback function for spinn_packet_gen's on_packet_gen callback. Assumes
 * that the data passed along with it is a pointer to an initialised
 * spinn_stat_inj_t;
 */
void *spinn_stat_inj_on_packet_gen(spinn_packet_t *packet, void *data);


/******************************************************************************
 * Packet consumption stats
 ******************************************************************************/

/**
 * Provides two simple counters of the number of packets which arrived at a node.
 */
typedef struct spinn_stat_con {
	int num_packets;
} spinn_stat_con_t;


/**
 * A callback function for spinn_packet_con's on_packet_con callback. Assumes
 * that the data passed along with it is a pointer to an initialised
 * spinn_stat_con;
 */
void spinn_stat_con_on_packet_gen(spinn_packet_t *packet, void *data);

#endif

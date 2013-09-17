/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_packet.h -- A SpiNNaker packet.
 */

#ifndef SPINN_PACKET_H
#define SPINN_PACKET_H

#include "spinn.h"


/**
 * Packet emergency routing state. Evaluates to "true" iff being emergency
 * routed.
 */
typedef enum spinn_emg_state {
	SPINN_EMG_NORMAL = 0,
	SPINN_EMG_FIRST_LEG,
	SPINN_EMG_SECOND_LEG,
} spinn_emg_state_t;


/**
 * A SpiNNaker packet.
 */
typedef struct spinn_packet {
	// Emergency-routing state of the packet
	spinn_emg_state_t emg_state;
	
	// The intended destination of the packet
	spinn_coord_t destination;
	
	// Packet payload
	void *payload;
} spinn_packet_t;


#endif

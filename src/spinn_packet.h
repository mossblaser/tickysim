/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_packet.h -- A SpiNNaker packet.
 */

#ifndef SPINN_PACKET_H
#define SPINN_PACKET_H

#include "config.h"

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
	// The intended inflection point of the packet's route
	spinn_coord_t     inflection_point;
	spinn_direction_t inflection_direction;
	
	// The intended destination of the packet
	spinn_coord_t destination;
	
	// The direction the packet is currently heading (specifically, the last
	// output port the packet was sent via.
	spinn_direction_t direction;
	
	// Emergency-routing state of the packet
	spinn_emg_state_t emg_state;
	
	// Packet payload
	void *payload;
} spinn_packet_t;


#endif

/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_stat.h -- A SpiNNaker packet and packet generator/consumer models.
 */


#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "spinn.h"
#include "spinn_stat.h"
#include "spinn_packet.h"
#include "spinn_router.h"


void *
spinn_stat_inj_on_packet_gen(spinn_packet_t *packet, void *inj_)
{
	spinn_stat_inj_t *inj = (spinn_stat_inj_t *)inj_;
	
	inj->num_offered++;
	if (packet != NULL)
		inj->num_accepted++;
	
	return NULL;
}


void
spinn_stat_con_on_packet_gen(spinn_packet_t *packet, void *con_)
{
	spinn_stat_con_t *con = (spinn_stat_con_t *)con_;
	
	con->num_packets++;
}


void
spinn_stat_drop_on_drop( spinn_router_t *router
                       , spinn_packet_t *packet
                       , void *drop_
                       )
{
	spinn_stat_drop_t *drop = (spinn_stat_drop_t *)drop_;
	
	drop->num_packets++;
}

# Do plenty of checking at compile time
AM_CFLAGS = -Wall -pedantic

# Building some programs (just tickysim in this case) which will be installed in
# the "bindir".
bin_PROGRAMS = tickysim_spinnaker

# To build tickysim, compile the following sources
tickysim_spinnaker_SOURCES  = spinn_main.c

tickysim_spinnaker_SOURCES += arbiter.c arbiter.h arbiter_internal.h
tickysim_spinnaker_SOURCES += buffer.c buffer.h buffer_internal.h
tickysim_spinnaker_SOURCES += scheduler.c scheduler.h scheduler_internal.h
tickysim_spinnaker_SOURCES += delay.c delay.h delay_internal.h

tickysim_spinnaker_SOURCES += spinn.h
tickysim_spinnaker_SOURCES += spinn_topology.c spinn_topology.h
tickysim_spinnaker_SOURCES += spinn_packet.c spinn_packet.h spinn_packet_internal.h
tickysim_spinnaker_SOURCES += spinn_router.c spinn_router.h spinn_router_internal.h

tickysim_spinnaker_SOURCES += spinn_sim.c spinn_sim.h
tickysim_spinnaker_SOURCES += spinn_sim_model.c spinn_sim_model.h
tickysim_spinnaker_SOURCES += spinn_sim_config.c spinn_sim_config.h
tickysim_spinnaker_SOURCES += spinn_sim_stat.c spinn_sim_stat.h

# Include libconfig in the build
tickysim_spinnaker_CPPFLAGS = $(LIBCONFIG_CFLAGS)
tickysim_spinnaker_LDADD  = $(LIBCONFIG_LIBS)

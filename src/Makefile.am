# Do plenty of checking at compile time
AM_CFLAGS = -Wall -pedantic

# Building some programs (just tickysim in this case) which will be installed in
# the "bindir".
bin_PROGRAMS = tickysim

# To build tickysim, compile the following sources
tickysim_SOURCES  = main.c
tickysim_SOURCES += arbiter.c arbiter.h
tickysim_SOURCES += buffer.c buffer.h buffer_internal.h
tickysim_SOURCES += scheduler.c scheduler.h scheduler_internal.h
tickysim_SOURCES += spinn.h
tickysim_SOURCES += spinn_topology.c spinn_topology.h
tickysim_SOURCES += spinn_packet.c spinn_packet.h

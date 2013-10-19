# Run tests under valgrind set to report memory leaks.
TESTS_ENVIRONMENT = CK_DEFAULT_TIMEOUT=0 valgrind -q --leak-check=full

# The test programs to run when doing "make check"
TESTS = check_check

# The check (test) program to compile
check_PROGRAMS = check_check

# The test program itself
check_check_SOURCES  = check_check.c check_check.h

# Test-suites and associated lumps of program
# XXX Add _internal.h files!
check_check_SOURCES += check_arbiter.c
check_check_SOURCES += $(top_builddir)/src/arbiter.c $(top_builddir)/src/arbiter_internal.h $(top_builddir)/src/arbiter.h
check_check_SOURCES += check_buffer.c
check_check_SOURCES += $(top_builddir)/src/buffer.c $(top_builddir)/src/buffer_internal.h $(top_builddir)/src/buffer.h
check_check_SOURCES += check_scheduler.c
check_check_SOURCES += $(top_builddir)/src/scheduler.c $(top_builddir)/src/scheduler_internal.h $(top_builddir)/src/scheduler.h
check_check_SOURCES += check_delay.c
check_check_SOURCES += $(top_builddir)/src/delay.c $(top_builddir)/src/delay_internal.h $(top_builddir)/src/delay.h
check_check_SOURCES += $(top_builddir)/src/spinn.h
check_check_SOURCES += check_spinn_topology.c
check_check_SOURCES += $(top_builddir)/src/spinn_topology.c $(top_builddir)/src/spinn_topology.h $(top_builddir)/src/spinn_topology_internal.h
check_check_SOURCES += check_spinn_router.c
check_check_SOURCES += $(top_builddir)/src/spinn_router.c $(top_builddir)/src/spinn_router_internal.h $(top_builddir)/src/spinn_router.h
check_check_SOURCES += check_spinn_packet_init_dor.c
check_check_SOURCES += check_spinn_packet_pool.c
check_check_SOURCES += check_spinn_packet_gen.c
check_check_SOURCES += check_spinn_packet_con.c
check_check_SOURCES += $(top_builddir)/src/spinn_packet.c $(top_builddir)/src/spinn_packet_internal.h $(top_builddir)/src/spinn_packet.h

# Build with the check library flags and library
check_check_CFLAGS = @CHECK_CFLAGS@ -Wall -pedantic
check_check_LDADD  = @CHECK_LIBS@


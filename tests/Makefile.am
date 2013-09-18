# The test programs to run when doing "make check"
TESTS = check_check

# The check (test) program to compile
check_PROGRAMS = check_check

# The test program itself
check_check_SOURCES  = check_check.c check_check.h

# Test-suites and associated lumps of program
check_check_SOURCES += check_arbiter.c $(top_builddir)/src/arbiter.c $(top_builddir)/src/arbiter.h
check_check_SOURCES += check_buffer.c $(top_builddir)/src/buffer.c $(top_builddir)/src/buffer.h
check_check_SOURCES += check_scheduler.c $(top_builddir)/src/scheduler.c $(top_builddir)/src/scheduler.h
check_check_SOURCES += $(top_builddir)/src/spinn.h
check_check_SOURCES += check_spinn_topology.c $(top_builddir)/src/spinn_topology.c $(top_builddir)/src/spinn_topology.h

# Build with the check library flags and library
check_check_CFLAGS = @CHECK_CFLAGS@ -Wall -pedantic
check_check_LDADD  = @CHECK_LIBS@


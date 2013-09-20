TickySim
========

A timing based interconnection network simulator.

Note: In early development. Doesn't neccessarily do anything yet except compile
(hopefully) pass unit tests...

Build Instructions
------------------

1. If you're using the version of TickySim straight from the repo, you'll need to
   run `autoreconf --install` to generate the configure script.
2. `./configure CFLAGS="-O3"` (for a more optimised build, omit the
   argument for default build).
3. `make`
4. `make check` (to compile & run the automated test suite against TickySim)

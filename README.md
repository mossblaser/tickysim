TickySim
========

A timing based interconnection network simulator. In particular, currently a
simulator for a SpiNNaker like network.


Build Instructions
------------------

1. If you're using the version of TickySim straight from the repo, you'll need to
   run `autoreconf --install` to generate the configure script.
2. `./configure CFLAGS="-O3"` (this selects a more optimised build, omit the
   argument for default build).
3. `make`


Usage
-----

Simulations are run using the binary `src/tickysim_spinnaker`. This command is
used as follows:

	$ tickysim_spinnaker /path/to/config/file

An example (documented) config file can be found in `src/spinnaker.config`.
Individual paramters in this file can be overridden on the command line using
`key=value` pairs as arguments. For example, to override the system's width, the
argument `model.system_size.width=12` should be given.


Unit Test Instructions
----------------------

To run the unit test suite run `make check` after performing the `./configure`
step mentioned previously.

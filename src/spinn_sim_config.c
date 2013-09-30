/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_sim_config.c -- Utility functions for accessing the simulation's
 * configuration parameters.
 */


#include "config.h"

#include <libconfig.h>

#include "spinn_sim.h"
#include "spinn_sim_config.h"


void
spinn_sim_config_init( spinn_sim_t *sim
                     , const char *filename
                     )
{
	config_init(&(sim->config));
	
	if (!config_read_file(&(sim->config), filename)) {
		fprintf( stderr, "%s:%d: %s\n"
		       , config_error_file(&(sim->config))
		       , config_error_line(&(sim->config))
		       , config_error_text(&(sim->config))
		       );
		exit(-1);
	}
}

void
spinn_sim_config_destroy(spinn_sim_t *sim)
{
	config_destroy(&(sim->config));
}


/**
 * Define functions wrapper_name and wrapper_name_default which wrap
 * config_lookup_func. wrapper_name will return the value requested or if it
 * isn't present in the file, crashes with exit(-1) and produces an error
 * message. The wrapper_name_default will silently ignore any error and return
 * the default value given as an argument.
 */
#define DEFINE_CONFIG_WRAPPER( wrapper_name                                    \
                             , wrapper_name_default                            \
                             , config_lookup_func                              \
                             , value_type                                      \
                             )                                                 \
	value_type                                                                   \
	wrapper_name( spinn_sim_t *sim                                               \
	            , const char  *path                                              \
	            )                                                                \
	{                                                                            \
		value_type value;                                                          \
		if (config_lookup_func( &(sim->config)                                     \
		                      , path                                               \
		                      , &value                                             \
		                      ))                                                   \
			return value;                                                            \
		                                                                           \
		/* The lookup failed */                                                    \
		fprintf( stderr, "Required value '%s' not found in config file!\n"         \
		       , path                                                              \
		       );                                                                  \
		exit(-1);                                                                  \
	}                                                                            \
	                                                                             \
	value_type                                                                   \
	wrapper_name_default( spinn_sim_t *sim                                       \
	                    , const char  *path                                      \
	                    , value_type   def_value                                 \
	                    )                                                        \
	{                                                                            \
		config_lookup_func( &(sim->config)                                         \
		                  , path                                                   \
		                  , &def_value                                             \
		                  );                                                       \
		return def_value;                                                          \
	}

DEFINE_CONFIG_WRAPPER( spinn_sim_config_lookup_int
                     , spinn_sim_config_lookup_int_default
                     , config_lookup_int
                     , int
                     )

DEFINE_CONFIG_WRAPPER( spinn_sim_config_lookup_float
                     , spinn_sim_config_lookup_float_default
                     , config_lookup_float
                     , double
                     )

DEFINE_CONFIG_WRAPPER( spinn_sim_config_lookup_bool
                     , spinn_sim_config_lookup_bool_default
                     , config_lookup_bool
                     , int
                     )

DEFINE_CONFIG_WRAPPER( spinn_sim_config_lookup_string
                     , spinn_sim_config_lookup_string_default
                     , config_lookup_string
                     , const char *
                     )

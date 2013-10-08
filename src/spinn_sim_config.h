/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_sim_config.h -- Utility functions for accessing the simulation's
 * configuration parameters.
 */

#ifndef SPINN_SIM_CONFIG_H
#define SPINN_SIM_CONFIG_H

#include <libconfig.h>

#include "spinn_sim.h"

/**
 * Open the config file. On error causes an exit(-1) and produces an error on
 * stderr.
 *
 * Also takes a set of arguments of the form "key=value" which can be used to
 * override values given in the config file.
 */
void spinn_sim_config_init( spinn_sim_t *sim
                          , const char  *filename
                          , int          argc
                          , char        *argv[]
                          );

/**
 * Free the resources used by the config file parser.
 */
void spinn_sim_config_destroy(spinn_sim_t *sim);


/**
 * Define functions wrapper_name and wrapper_name_default which wrap libconfig
 * calls. wrapper_name will return the value requested or if it isn't present in
 * the file, crashes with exit(-1) and produces an error message. The
 * wrapper_name_default will silently ignore any error and return the default
 * value given as an argument.
 */
#define DEFINE_CONFIG_WRAPPER( wrapper_name                                    \
                             , wrapper_name_default                            \
                             , value_type                                      \
                             )                                                 \
	value_type                                                                   \
	wrapper_name( spinn_sim_t *sim                                               \
	            , const char  *path                                              \
	            );                                                               \
	                                                                             \
	value_type                                                                   \
	wrapper_name_default( spinn_sim_t *sim                                       \
	                    , const char  *path                                      \
	                    , value_type   def_value                                 \
	                    );

DEFINE_CONFIG_WRAPPER( spinn_sim_config_lookup_int
                     , spinn_sim_config_lookup_int_default
                     , int
                     )

DEFINE_CONFIG_WRAPPER( spinn_sim_config_lookup_int64
                     , spinn_sim_config_lookup_int64_default
                     , long long int
                     )

DEFINE_CONFIG_WRAPPER( spinn_sim_config_lookup_float
                     , spinn_sim_config_lookup_float_default
                     , double
                     )

DEFINE_CONFIG_WRAPPER( spinn_sim_config_lookup_bool
                     , spinn_sim_config_lookup_bool_default
                     , int
                     )

DEFINE_CONFIG_WRAPPER( spinn_sim_config_lookup_string
                     , spinn_sim_config_lookup_string_default
                     , const char *
                     )

#undef DEFINE_CONFIG_WRAPPER


/**
 * Get the number of experimental groups.
 */
int spinn_sim_config_get_num_exp_groups(spinn_sim_t *sim);

/**
 * Set the config values to those specified by the given group number.
 */
void spinn_sim_config_set_exp_group(spinn_sim_t *sim, int group_num);

#endif

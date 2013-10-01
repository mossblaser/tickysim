/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_sim_config.c -- Utility functions for accessing the simulation's
 * configuration parameters.
 */


#include "config.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <libconfig.h>

#include "spinn_sim.h"
#include "spinn_sim_config.h"


void
spinn_sim_config_init( spinn_sim_t *sim
                     , const char *filename
                     , int         argc
                     , char       *argv[]
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
	
	// Process override arguments
	for (int i = 0; i < argc; i++) {
		char *key = argv[i];
		// The value comes after the '='. Make sure it is present!
		char *value = index(argv[i], '=');
		if (value == NULL) {
			fprintf(stderr, "Invalid argument '%s'. Arguments must be of the form key=value!\n"
			              , argv[i]
			              );
			exit(-1);
		}
		// Terminate the key part of the string and get the pointer to the value
		// part.
		value[0] = '\0';
		value++;
		
		config_setting_t *setting = config_lookup(&(sim->config), key);
		if (setting == NULL || !config_setting_is_scalar(setting)) {
			fprintf(stderr, "Configuration key '%s' does not exist!\n", key);
			exit(-1);
		}
		
		// Parse and store the value
		int type = config_setting_type(setting);
		switch (type) {
			case CONFIG_TYPE_INT:
				assert(config_setting_set_int(setting, atoi(value)));
				break;
			case CONFIG_TYPE_INT64:
				assert(config_setting_set_int64(setting, atol(value)));
				break;
			case CONFIG_TYPE_FLOAT:
				assert(config_setting_set_float(setting, atof(value)));
				break;
			case CONFIG_TYPE_STRING:
				assert(config_setting_set_string(setting, value));
				break;
			case CONFIG_TYPE_BOOL:
				assert(config_setting_set_bool(setting, strcasecmp(value, "true") == 0));
				break;
			default:
				fprintf(stderr, "Cannot override config key '%s' of type %d.\n"
				              , key
				              , type
				              );
				exit(-1);
				break;
		}
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

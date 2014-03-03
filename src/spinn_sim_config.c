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


/******************************************************************************
 * Internal: Experiment paramter processing
 ******************************************************************************/

// A list of parameters which are allowed to be changed when cold_group ==
// false. If any other values appear in the independent variables list then the
// program will report an error.
static const char *hot_params[] = {
	"model.packet_generator.generator_mask",
	"model.packet_generator.temporal.dist",
	"model.packet_generator.temporal.bernoulli_prob",
	"model.packet_generator.temporal.periodic_interval",
	"model.packet_generator.spatial.dist",
	"model.packet_generator.spatial.allow_local",
	
	"model.packet_consumer.temporal.dist",
	"model.packet_consumer.temporal.bernoulli_prob",
	"model.packet_consumer.temporal.periodic_interval",
	
	"model.node_to_node_links.packet_delay",
};
static const int num_hot_params = sizeof(hot_params)/sizeof(char *);


/**
 * Set up the list of independent variables in the experiment.
 */
void
spinn_sim_config_init_independent_variables(spinn_sim_t *sim)
{
	// Is the experiment going to be cold started for each group?
	bool cold_group = spinn_sim_config_lookup_bool(sim, "experiment.cold_group");
	
	// Get the list of variables
	config_setting_t *ivar_list = config_lookup(&(sim->config), "experiment.independent_variables");
	if (ivar_list == NULL || config_setting_type(ivar_list) != CONFIG_TYPE_LIST) {
		fprintf(stderr, "Expected a list of independent variables in 'experiment.independent_variables'.\n");
		exit(-1);
	}
	sim->num_ivars = config_setting_length(ivar_list);
	
	// Create the lists of settings and names
	sim->ivar_settings = calloc(sim->num_ivars, sizeof(config_setting_t *));
	sim->ivar_names    = calloc(sim->num_ivars, sizeof(const char *));
	
	for (int i = 0; i < sim->num_ivars; i++) {
		// Get the key/value pair
		config_setting_t *key_name_list = config_setting_get_elem(ivar_list, i);
		if (key_name_list == NULL
		    || config_setting_type(key_name_list) != CONFIG_TYPE_LIST
		    || config_setting_length(key_name_list) != 2
		    || config_setting_type(config_setting_get_elem(key_name_list, 0)) != CONFIG_TYPE_STRING
		    || config_setting_type(config_setting_get_elem(key_name_list, 1)) != CONFIG_TYPE_STRING
		   ) {
			fprintf(stderr, "Expected 'experiment.independent_variables'"
			                " to contain a list of (key,name) string pairs.\n");
			exit(-1);
		}
		const char *key  = config_setting_get_string_elem(key_name_list, 0);
		const char *name = config_setting_get_string_elem(key_name_list, 1);
		
		// Check that the variable exists
		config_setting_t *ivar_setting = config_lookup(&(sim->config), key);
		if (ivar_setting == NULL) {
			fprintf(stderr, "Independant variable '%s' (%s) not found.\n", key, name);
			exit(-1);
		}
		
		// Check that if we're doing hot group running the variable can be changed
		// without a cold-start
		if (!cold_group) {
			bool can_hot_group = false;
			for (int j = 0; j < num_hot_params; j++) {
				if (strcmp(key, hot_params[j]) == 0) {
					can_hot_group = true;
					break;
				}
			}
			
			if (!can_hot_group) {
				fprintf(stderr, "Changing '%s' with 'cold_group' = false is not possible.\n"
				              , key
				              );
				exit(-1);
			}
		}
		
		// Add it to the list
		sim->ivar_settings[i] = ivar_setting;
		sim->ivar_names[i]    = name;
	}
}

/**
 * Free up the list of independent variables in the experiment.
 */
void
spinn_sim_config_destroy_independent_variables(spinn_sim_t *sim)
{
	free(sim->ivar_settings);
	free(sim->ivar_names);
}

/******************************************************************************
 * libconfig convenience wrappers
 ******************************************************************************/


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
	
	// Load the experiment parameters
	spinn_sim_config_init_independent_variables(sim);
}

void
spinn_sim_config_destroy(spinn_sim_t *sim)
{
	spinn_sim_config_destroy_independent_variables(sim);
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

DEFINE_CONFIG_WRAPPER( spinn_sim_config_lookup_int64
                     , spinn_sim_config_lookup_int64_default
                     , config_lookup_int64
                     , long long int
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



/******************************************************************************
 * Experement independent variable utility functions
 ******************************************************************************/


int
spinn_sim_config_get_num_exp_groups(spinn_sim_t *sim)
{
	// Get the list of groups
	config_setting_t *group_list = config_lookup(&(sim->config), "experiment.groups");
	if (group_list == NULL || config_setting_type(group_list) != CONFIG_TYPE_LIST) {
		fprintf(stderr, "Expected a list of groups in 'experiment.groups'.\n");
		exit(-1);
	}
	
	int num_groups = config_setting_length(group_list);
	if (num_groups < 1) {
		fprintf(stderr, "Experiments must have at least one group.\n");
		exit(-1);
	}
	
	return num_groups;
}


void
spinn_sim_config_set_exp_group(spinn_sim_t *sim, int group_num)
{
	// Get the list of groups
	config_setting_t *group_list = config_lookup(&(sim->config), "experiment.groups");
	if (group_list == NULL || config_setting_type(group_list) != CONFIG_TYPE_LIST) {
		fprintf(stderr, "Expected a list of groups in 'experiment.groups'.\n");
		exit(-1);
	}
	
	// Get the group we care about
	config_setting_t *group = config_setting_get_elem(group_list, group_num);
	assert(group != NULL);
	
	// Make sure it has the correct number of elements
	if (config_setting_length(group) != sim->num_ivars) {
		fprintf(stderr, "Expected group %d to contain exactly %d variable%s.\n"
		              , group_num + 1
		              , sim->num_ivars
		              , (sim->num_ivars == 1) ? "" : "s"
		              );
		exit(-1);
	}
	
	// Set each variable in turn
	for (int i = 0; i < sim->num_ivars; i++) {
		config_setting_t *ivar      = sim->ivar_settings[i];
		const char       *ivar_name = sim->ivar_names[i];
		config_setting_t *value     = config_setting_get_elem(group, i);
		
		// Check types match
		if (config_setting_type(ivar) != config_setting_type(value)) {
			fprintf(stderr, "Expected value %d (%s) of group %d to match type of the independent variable.\n"
			              , i + 1
			              , ivar_name
			              , group_num + 1
			              );
			exit(-1);
		}
		
		// Move the value accross
		switch(config_setting_type(ivar)) {
			case CONFIG_TYPE_INT:
				config_setting_set_int(ivar, config_setting_get_int(value));
				break;
			case CONFIG_TYPE_INT64:
				config_setting_set_int64(ivar, config_setting_get_int64(value));
				break;
			case CONFIG_TYPE_FLOAT:
				config_setting_set_float(ivar, config_setting_get_float(value));
				break;
			case CONFIG_TYPE_STRING:
				config_setting_set_string(ivar, config_setting_get_string(value));
				break;
			case CONFIG_TYPE_BOOL:
				config_setting_set_bool(ivar, config_setting_get_bool(value));
				break;
			default:
				fprintf(stderr, "Setting is of an unsupported type %d for an independent variable.\n"
				              , config_setting_type(ivar)
				              );
				exit(-1);
		}
	}
}

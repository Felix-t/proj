/*
 * startup.c:
 *	Program launched on startup 
 *	 - Check remaining battery power
 *	 - Schedule next startup and shutdown 
 *	 - Start acquisitions subroutines
 *  @TODO : Logging system, see syslog
 */
#define CFG_FILE "conftest"

#include "include.h"

#include <libconfig.h>
#include "cfg.h"

/* Function :
 * Params :
 * Return :
*/
static void init_cfg(config_t *cfg, config_setting_t **root)
{
	config_init(cfg);
	config_read_file(cfg, CFG_FILE);

	*root = config_root_setting(cfg);
}	
/* Function :
 * Params :
 * Return :
*/
void get_cfg_double(double *values, char **str, int32_t str_nb)
{
	int32_t i;
	config_t config;
	config_setting_t *setting_root, *path;

	init_cfg(&config, &setting_root);

	for(i = 0; i<str_nb; i++)
	{
		config_lookup_float(&config, str[i], &values[i]);
	//	if((path = config_lookup(&config, str[i])) != NULL)
	//		values[i] = config_setting_get_float(path);

	}
}

/* Function :
 * Params :
 * Return :
*/
void get_cfg_str(const char **values, char **str, int32_t str_nb)
{
	int32_t i;
	config_t config;
	config_setting_t *setting_root, *path;

	init_cfg(&config, &setting_root);

	for(i = 0; i<str_nb; i++)
	{
		printf("%s\n", str[0]);
		config_lookup_string(&config, str[i], values);
	//	if((path = config_lookup(&config, str[i])) != NULL)
	//		values[i] = config_setting_get_float(path);

	}
}
/* Function :
 * Params :
 * Return :
*/
void set_cfg(char **str, double *values, int32_t str_nb)
{
	int32_t i;
	
	config_t config;
	config_setting_t *setting_root, *path;
	init_cfg(&config, &setting_root);
	for(i = 0; i<str_nb; i++)
	{
		path = config_setting_add(setting_root, str[i], CONFIG_TYPE_FLOAT);
		path = config_lookup(&config, str[i]);
		config_setting_set_float(path, values[i]);
	}	
	config_write_file(&config, CFG_FILE);
}
/*
int main()
{
	char *str[4] = {"test1", "test2", "test3", "string"};
	double values[3];
	const char* string[1];
	get_cfg(values, str, 3);
	printf("%f\n%f\n%f\n", values[0], values[1], values[2]);
	get_cfg(string, &str[3], 1);
	printf("%s\n", string[0]);
	}

	*/

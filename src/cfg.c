/*
 * startup.c:
 *	Program launched on startup 
 *	 - Check remaining battery power
 *	 - Schedule next startup and shutdown 
 *	 - Start acquisitions subroutines
 *  @TODO : Logging system, see syslog
 */
#include "cfg.h"
#include <dirent.h>

static uint8_t get_usb_config(config_t * cfg);
/* Function :
 * Params :
 * Return :
 */
static void init_cfg(config_t *cfg, config_setting_t **root)
{
	static uint8_t moved_cfg = 0;
	config_init(cfg);
	config_read_file(cfg, CFG_FILE);


	if(moved_cfg == 0 && get_usb_config(cfg) == 2)
	{
		config_destroy(cfg);
		config_init(cfg);
		config_read_file(cfg, CFG_FILE);
		moved_cfg = 1;
	}
else
	moved_cfg = 1;

	*root = config_root_setting(cfg);
}

static uint8_t get_usb_config(config_t *cfg)
{
	DIR* FD;
	struct dirent* in_file;

	char cmd[100];
	char path[100];

	char *cfg_path = "PATH_USB_STICK";
	const char *usb_key_config;
	config_lookup_string(cfg,cfg_path, &usb_key_config);

	strncpy(path, usb_key_config, 100);

	if (NULL == (FD = opendir(usb_key_config))) 
	{
		fprintf(stderr, "Error : Failed to open input directory - ");
		return 0;
	}

	while ((in_file = readdir(FD))) 
	{
		if (!strcmp(in_file->d_name, CFG_FILE))
		{
			strcat(path, CFG_FILE);
			sprintf(cmd, "/bin/mv %s /home/pi/Surffeol/%s", 
					path, CFG_FILE);
			if(system(cmd) == -1)
			{
				printf("Error during cp of config file");
				return 0;
			}
			printf("Found a config file in USB, moving it"
				       	" to current dir\n");
			return 2;
		}
	}
	closedir(FD);
	return 1;
}

/* Function :
 * Params :
 * Return :
*/
void get_cfg_double(double *values, char **str, int32_t str_nb)
{
	int32_t i;
	config_t config;
	config_setting_t *setting_root;

	init_cfg(&config, &setting_root);

	for(i = 0; i<str_nb; i++)
	{
		config_lookup_float(&config, str[i], &values[i]);

	}
	config_destroy(&config);
}

/* Function :
 * Params :
 * Return :
*/
void get_cfg_str(char **values, char **str, int32_t str_nb)
{
	int32_t i;
	config_t config;
	config_setting_t *setting_root;
	const char *tmp_str[str_nb];

	init_cfg(&config, &setting_root);

	for(i = 0; i<str_nb; i++)
	{
		config_lookup_string(&config, str[i], &tmp_str[i]);
		strcpy(values[i], tmp_str[i]);
	}

	config_destroy(&config);
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
	config_destroy(&config);
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

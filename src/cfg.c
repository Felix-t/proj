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

static uint8_t valid_config(char * path);
static uint8_t get_usb_config(config_t * cfg);
/* Function :
 * Params :
 * Return :
 */
static uint8_t init_cfg(config_t *config, config_setting_t *root)
{
	static uint8_t moved_cfg = 0;

	config_init(config);
	root = config_root_setting(config);
	if(config_read_file(config, CFG_FILE) == CONFIG_FALSE)
	{
		printf("Error reading configuration file line %i : %s\n",
				config_error_line(config), 
				config_error_text(config));
		return 0;
	}

	//End remove
	if(moved_cfg == 0 && get_usb_config(config) == 2)
	{
		config_destroy(config);
		config_init(config);
		if(config_read_file(config, CFG_FILE) == CONFIG_FALSE)
		{
			printf("Error reading configuration file line %i : %s\n",
					config_error_line(config),
					config_error_text(config));
			return 0;
		}
		moved_cfg = 1;
	}
	else
		moved_cfg = 1;


	return 1;
}

static uint8_t get_usb_config(config_t *cfg)
{
	DIR* FD;
	struct dirent* in_file;

	char cmd[100]= "";
	char path[100]= "";

	char *cfg_path = "PATH_USB_STICK";
	const char *usb_key_config[1];
	printf("%s\n", "abcd");
	config_lookup_string(cfg, cfg_path, usb_key_config);

	strncpy(path, usb_key_config[0], 100);

	if (NULL == (FD = opendir(usb_key_config[0]))) 
	{
		fprintf(stderr, "Error : Failed to open USB - \n");
		return 0;
	}

	while ((in_file = readdir(FD))) 
	{
		if (!strcmp(in_file->d_name, CFG_FILE))
		{
			strcat(path, CFG_FILE);
			printf("Found a config file un USB stick...");
			if(!valid_config(path))
			{
				printf("\n\tSyntax/parsing error in new config"
						" file, using base values\n");
				return 0;
			}
			sprintf(cmd, "/bin/mv %s /home/pi/Surffeol/%s", 
					path, CFG_FILE);
			if(system(cmd) == -1)
			{
				printf("\n\tError during cp of config file\n");
				return 0;
			}
			printf(" moving it to current directory");
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
uint8_t get_cfg_double(double *values, char **str, int32_t str_nb)
{
	int32_t i;
	config_t config;
	config_setting_t setting_root;

	if(!init_cfg(&config, &setting_root))
		return 0;

	for(i = 0; i<str_nb; i++)
	{
		printf("%s\n", str[i]);
		config_lookup_float(&config, str[i], &values[i]);

	}
	config_destroy(&config);
	return 1;
}

/* Function :
 * Params :
 * Return :
 */
uint8_t get_cfg_str(char **values, char **str, int32_t str_nb)
{
	int32_t i;
	config_t config;
	config_setting_t setting_root;
	const char *tmp_str[str_nb];

	if(!init_cfg(&config, &setting_root))
		return 0;

	for(i = 0; i<str_nb; i++)
	{
		config_lookup_string(&config, str[i], &tmp_str[i]);
		strcpy(values[i], tmp_str[i]);
	}

	config_destroy(&config);
	return 1;
}
/* Function :
 * Params :
 * Return :
 */
uint8_t set_cfg(char **str, double *values, int32_t str_nb)
{
	int32_t i;

	config_t config;
	config_setting_t setting_root, *path;
	if(!init_cfg(&config, &setting_root))
		return 0;
	for(i = 0; i < str_nb; i++)
	{
		path = config_setting_add(&setting_root, str[i], CONFIG_TYPE_FLOAT);
		path = config_lookup(&config, str[i]);
		if(config_setting_is_array(path) == CONFIG_TRUE)
			config_setting_set_float_elem(path, 0, values[i]);
		else
			config_setting_set_float(path, values[i]);
	}	
	config_write_file(&config, CFG_FILE);
	config_destroy(&config);
	return 1;
}

static uint8_t valid_config(char * path)
{
	uint8_t i;

	config_t config;
	config_setting_t *set;

	config_init(&config);
	config_read_file(&config, path);

	char * names[] = {
		"ACQ_TIME",
		"MAX_VOLT",
		"MIN_VOLT",
		"THRESHOLD", 
		"LAST_DISCHARGE",
		"taille_max_du_fichier_sauvegarde",
		"freq_echantillonnage",
		"USB_SIZE",

		"sauvagarde_cle_usb",
		"SMS_alert",
		"mode_meas",
		"mode_debug",
		"channel_1__actif",
		"channel_2__actif",

		"PATH_LSM9DS0_DATA",
		"PATH_GPSLSM9DS0_DATA",
		"PATH_USB_STICK",
		"nom_projet",
		"chemin_cle_usb",
		"nom_fichier_sauvegarde",
		"numero_jauge_channel_1",
		"type_jauge_channel_1",
		"numero_jauge_channel_2",
		"type_jauge_channel_2",
		"module_idn",

		"GFx_jauge",
		"GFx_jauge.channel_1",
		"GFx_jauge.channel_2",
		"zeros"
	};

	for(i = 0; i < 29; i++)
	{
		if((set = config_lookup(&config, names[i])) == NULL)
		{
			printf("\n\tError with %s config\n", names[i]);
			return 0;
		}
		if((i < 8 && config_setting_type(set) != CONFIG_TYPE_FLOAT)
				|| (i < 14 && config_setting_type(set) != CONFIG_TYPE_INT)
				|| (i < 14 && config_setting_type(set) != CONFIG_TYPE_STRING))
		{
			printf("\n\tError : value of %s incorrect\n", names[i]);
			return 0;
		}
		config_destroy(&config);
		return 1;
	}

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

#ifndef UTIL_H
#define UTIL_H

#include "headers.h"
#define TIMEOUT 1

uint8_t get_cpu_usage();
uint8_t get_temp();
uint8_t set_next_startup(int32_t startup_time);
uint8_t copy(char *source, char *dest);
uint8_t exec_script(char *cmd);
uint8_t archive_data();
uint8_t move_logs();
uint8_t start_dhcp_server();
uint8_t program_shutdown();

#endif

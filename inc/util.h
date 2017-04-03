#ifndef UTIL_H
#define UTIL_H

#include "headers.h"
#define TIMEOUT 1

uint8_t set_next_startup(int32_t startup_time);
void copy(char *source, char *dest);
void exec_script(char *cmd);
void archive_data();
void move_logs();
uint8_t start_dhcp_server();

#endif

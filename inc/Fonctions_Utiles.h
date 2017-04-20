#ifndef FUNCTIONS_UTILES__H
#define FUNCTIONS_UTILES__H

#include "headers.h"


int Find_substr_in_str(char *str, char *substr);

int Find_position_substr_in_str(char *str, char *substr);

int Compare_2str(const char *str1, char *str2);

int Execute_file_sh(char * str_cmd);

int Execute_file_sh_sans_retour_info (char * str_cmd);

void Zero_str(char *str);

int fgets_stdin(char *str_line,int size_str_line);

int Verification_saisie(char *modif_date_time, char *choix_possible_modif_date_time_WLX2[], int nb_choix);

float Calcul_free_space(char *chemin, int ok_print);

int Verif_if_dir_exist(char *chemin);

int find_value_in_int_array(int value, const int array[]);

#endif

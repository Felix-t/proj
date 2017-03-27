#ifndef PREPARATION_ACQ__H
#define PREPARATION_ACQ__H

#ifndef HEADERS__H
#include "Headers.h"
#endif

#ifndef CONFIGURATION_PGM__H
#include "Configuration_Programme_Opsens_WLX2.h"
#endif

#ifndef CONNEXION__H
#include "Connexion_Opsens_WLX2.h"
#endif

#ifndef FUNCTIONS_UTILES__H
#include "Fonctions_Utiles.h"
#endif

extern _Atomic uint8_t end_program;

struct shared 
{
int ok_record;
int offset_modif;
int nb_meas_done;
float size_save_file;
int nb_save_file;
float *ch_offset;
float *ch_zero;
float *ch_value;
float size_max_free; 
char *chemin;
char cmd_acq;
pthread_mutex_t *mutex;
pthread_t thread_enregistrement;
FILE *fp;
};


struct param_pgm
{
struct parametres_connexion *pparam_connection;
struct config_all *pconfig_all;
struct shared *pshared;
};



#ifndef ACQUISITION__H
#include "Acquisition_Opsens_WLX2.h"
#endif



void* thread_Acquisition_data (void* arg);
void* thread_Wait_Command(void* arg);

int Preparation_acquisition_WLX2(struct param_pgm *pparam_pgm, struct parametres_connexion *pparam_connection, struct config_all *pconfig_all, struct shared *pshared);

int Init_struct_shared(struct shared *pshared, char *chemin, float *ch_zero, float *ch_offset, float *ch_value, pthread_mutex_t *mutex);

void Init_struct_param_pgm(struct param_pgm *pparam_pgm, struct parametres_connexion *pparam_connection, struct config_all *pconfig_all, struct shared *pshared);

int Lancement_thread_acquistion(struct param_pgm *param);

int Verif_free_space(struct param_pgm *param, int ok_print);

int Init_repertoire_pour_enregistrement_data(struct param_pgm *param);

void Find_dernier_enregistrement(struct param_pgm *param);

void Envoi_SMS_alert();

#endif



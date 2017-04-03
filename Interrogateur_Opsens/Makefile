# Makefile racine
.SUFFIXES:
.INIT:
.PHONY: clean_objet clean objects .defmain

# Paths
DSRC = Source
DINC = Include
DOBJ = Objet
DEXE = Executable
CONF = Config

EXEC = Programme_Acquisition_Opsens_WLX2
SCRIPT= Script_Kill_All_Process_Opsens_WLX2.sh
CC = gcc
CFLAGS = -I$(DINC)
LIBS= -lm -lpthread

SRC = $(wildcard $(DSRC)/*.c)
_OBJ= $(SRC:.c=.o)
#_OBJ = Connexion_Opsens_WLX2.o Fonctions_Utiles.o 
OBJ = $(patsubst $(DSRC)%,$(DOBJ)/%,$(_OBJ))
DEPS = $(wildcard $(DINC)/*.h)

objects: defmain $(OBJ)

defmain: 
	$(eval CFLAGS := $(CFLAGS) -D_MAIN_)

Main_Acquisition_Opsens_WLX2: $(OBJ)
#	@echo "Compilation du programme Main_Acquisition_Opsens_WLX2"
	$(CC) $^ -o $(EXEC) $(CFLAGS) $(LIBS)
#	sh $(DSRC)/$(SCRIPT) $(EXEC)

$(DOBJ)/%.o: $(DSRC)/$(notdir %).c $(DINC)/$(notdir %).h
#	@echo "Compilation des dépendances de Main_Acquisition_Opsens_WLX2"
	$(CC) -c $< -o $@ $(CFLAGS) $(LIBS) 

#$(DOBJ)/%.o:
#	$(CC) -c $(DSRC)/$(notdir $(basename $@)).c -o $@ $(CFLAGS) $(LIBS) 


#$(DOBJ)/Connexion_Opsens_WLX2.o:
#	gcc -c $(DSRC)/Connexion_Opsens_WLX2.c -o $@ $(CFLAGS) $(LIBS) 

#$(DOBJ)/Fonctions_Utiles.o:
#	gcc -c $(DSRC)/Fonctions_Utiles.c -o $@ $(CFLAGS) $(LIBS) 


clean_objet:   
	rm -rf $(DOBJ)/*.o
	rm -rf $(EXEC)	
#	echo "Suppression des fichiers objets"


clean:
	rm -rf $(DOBJ)/*.o
	rm -rf $(EXEC)	
	rm -rf $(DINC)/*.h~
	rm -rf $(DSRC)/*.c~
	rm -rf $(DSRC)/*.sh~
	rm -rf $(CONF)/*~
	rm -rf *~
#	echo "Suppression des fichiers temporaires"






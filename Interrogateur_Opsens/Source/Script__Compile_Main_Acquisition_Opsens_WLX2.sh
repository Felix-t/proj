gcc -c Connexion.c -o Connexion.o

gcc -c Configuration.c -o Configuration.o


gcc Main__Acquisition_Opsens__WLX2.c Connexion.o Configuration.o -o Main__Acquisition_Opsens__WLX2 -lm

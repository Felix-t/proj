.PHONY: clean all copy

CC=gcc

ODIR=obj
SDIR=src
BDIR=bin
HDIR=inc

LIBS=-lbcm2835 -pthread -lconfig -lm
CFLAGS = -I$(HDIR)

DEPS = $(wildcard $(HDIR)/*.h)

OBJ = $(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRC))
OBJS = $(wildcard $(ODIR)/*.o)

SRC = $(wildcard $(SDIR)/*.c)
all: copy $(BDIR)/acq_surffeol	

$(BDIR)/acq_surffeol: $(OBJ)
	gcc -o $@ $(OBJS) $(LIBS)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BDIR)/cfg: src/cfg.c src/cfg.h
	gcc -o $@ src/cfg.c $(LIBS)

$(BDIR)/accelerometer: $(IDIR)/accelerometer.c 
	gcc -o $@ src/accelerometer.c $(LIBS)

copy:
	cd Interrogateur_Opsens && $(MAKE) objects
	cp -u Interrogateur_Opsens/Include/*.h inc/
	cp -u Interrogateur_Opsens/Objet/*.o obj/

clean:
	rm -f $(ODIR)/*.o $(BDIR)/*
	cd Interrogateur_Opsens/ && make clean


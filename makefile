CC=gcc

ODIR=obj
IDIR=src
BDIR=bin

LIBS=-lbcm2835 -pthread -lconfig

_DEPS = ADS1256.h cfg.h include.h util.h battery.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

OBJ = $(patsubst src/%.c,$(ODIR)/%.o,$(SRC))

SRC = $(wildcard src/*.c)

$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< 

$(BDIR)/ad_converter: $(OBJ)
	gcc -o $@ $^ $(LIBS)

$(BDIR)/cfg: src/cfg.c src/cfg.h
	gcc -o $@ src/cfg.c $(LIBS)

$(BDIR)/accelerometer: $(IDIR)/accelerometer.c 
	gcc -o $@ src/accelerometer.c $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(BDIR)/*


IDIR=c_src
CC=gcc
BIN=server
CFLAGS=-I$(IDIR)

ODIR=c_src
LDIR=c_src

LIBS=-lm -pthread

_DEPS = structs.h constants.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = colors.o game.o game_logic.o memory.o player.o stats.o server.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	mv -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
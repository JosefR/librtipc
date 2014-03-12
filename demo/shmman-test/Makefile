CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -g
LDFLAGS = 

INCLUDEDIRS = -I../../include -I../../include/system -I../../include/arch 

LIBDIRS = -L../../lib

LIBS = -lrtipc

BIN = shmman-test

OBJ = 

all: $(OBJ)
	$(CC) $(CFLAGS) $(BIN).c -o $(BIN) $(OBJ) $(LDFLAGS) $(LIBDIRS) $(INCLUDEDIRS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(OBJ) $(BIN)

.PHONY: clean all

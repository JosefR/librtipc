CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall  
LDFLAGS = 

INCLUDEDIRS = -I../../include -I../../include/arch -I../../include/system 

LIBDIRS = -L../../lib

LIBS = -lrtipc

BIN = barrier-test

OBJ = 

all: $(OBJ)
	$(CC) $(CFLAGS) $(BIN).c -o $(BIN) $(OBJ) $(LDFLAGS) $(LIBDIRS) $(INCLUDEDIRS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(OBJ) $(BIN)

.PHONY: clean all

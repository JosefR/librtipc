CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -O0
LDFLAGS = 

INCLUDEDIRS = -I../../include -I../../include/arch -I../../include/system 

LIBDIRS = -L../../lib

LIBS = -lrtipc

BIN = lfmpscq-latency2

OBJ = 

all: $(OBJ)
	$(CC) $(CFLAGS) $(BIN).c -o $(BIN) $(OBJ) $(LDFLAGS) $(LIBDIRS) $(INCLUDEDIRS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(OBJ) $(BIN)

.PHONY: clean all

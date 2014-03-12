CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -O0 
LDFLAGS = 

INCLUDEDIRS = -I../../include -I../../include/arch -I../../include/system
#INCLUDEDIRS = -I../../include -I../../include/arch -I../../include/system  -I../../../apmtool/src 

LIBDIRS = -L../../lib 
#LIBDIRS = -L../../lib   -L../../../apmtool/src

LIBS = -lrtipc
#LIBS = -lrtipc -lapmtool

BIN = hourglass

OBJ = 

all: $(OBJ)
	$(CC) $(CFLAGS) $(BIN).c -o $(BIN) $(OBJ) $(LDFLAGS) $(LIBDIRS) $(INCLUDEDIRS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(OBJ) $(BIN)

.PHONY: clean all

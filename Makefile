#
# Makefile
#
# Description: librtipc Makefile  
# Author: Josef Raschen <josef@raschen.org>
#


CC= $(CROSS_COMPILE)gcc

CFLAGS= -Wall -O0 -g
LDFLAGS= 

LIBOUT=lib/librtipc.a

INCDIR=include include/arch include/system

# the config script
TOPDIR=$(shell /bin/pwd)
MKCONFIG=$(TOPDIR)/configure

# the librtipc base source
BASESRC=rtipc.c barrier.c mutex.c flag.c spscq.c wfmpscq.c lfmpscq.c mpmcq.c sensorbuffer.c
BASESRCDIR=src
BASEOBJ=$(BASESRC:.c=.o)

# the architecture specific source
ARCHSRC=atomic.c hwfunctions.c
ARCHSRCDIR=src/arch
ARCHOBJ=$(ARCHSRC:.c=.o)

# the system specific source
SYSTEMSRC=shmman.c osfunctions.c 
SYSTEMSRCDIR=src/system
SYSTEMOBJ=$(SYSTEMSRC:.c=.o)

LIBINC=$(addprefix -I,$(INCDIR))
LIBSRC=$(addprefix $(BASESRCDIR)/,$(BASESRC)) $(addprefix $(ARCHSRCDIR)/,$(ARCHSRC)) $(addprefix $(SYSTEMSRCDIR)/,$(SYSTEMSRC))
LIBOBJ=$(LIBSRC:.c=.o) 
#LIB=$(addprefix "$(BASESRCDIR)/",$(BASEOBJ)) $(addprefix "$(ARCHSRCDIR)/",$(ARCHOBJ))=$(addprefix "$(SYSTEMSRCDIR)/",$(SYSTEMOBJ))


all: $(LIBOBJ)
	$(AR) rcs $(LIBOUT) $(LIBOBJ)
	$(MAKE) -C demo all

demo: $(LIBOBJ)
	$(MAKE) -C demo all

config:
	$(MKCONFIG) $(ARCH) $(SYSTEM)

clean:
	rm -f $(LIBOUT) $(LIBOBJ)
	$(MAKE) -C demo clean

distclean: clean
	rm -f include/arch src/arch include/system src/system

%.o: %.c
	$(CC) $(CFLAGS) $(LIBINC) -c $< -o $@

.PHONY: all config clean distclean


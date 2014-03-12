SUBDIRS = shmman-test barrier-test sensorbuffer-test

all: 
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean ; \
	done

.PHONY: clean all

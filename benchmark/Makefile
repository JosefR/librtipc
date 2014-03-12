SUBDIRS = hourglass shmem-ping-pong spscq-latency spscq-ping-pong sensorbuffer-latency lfmpscq-latency2 wfmpscq-latency2

all: 
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean ; \
	done

.PHONY: clean all

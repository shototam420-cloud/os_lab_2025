CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra
COMMON = ../../common/lab_args.h
.PHONY: all clean
all: mutex_no mutex_yes factorial deadlock

mutex_no: mutex.c $(COMMON)
	$(CC) $(filter-out -O2,$(CFLAGS)) -O0 $< -pthread -o $@

mutex_yes: mutex.c $(COMMON)
	$(CC) $(filter-out -O2,$(CFLAGS)) -O0 -DUSE_MUTEX $< -pthread -o $@

factorial: factorial.c $(COMMON)
	$(CC) $(CFLAGS) $< -pthread -o $@

deadlock: deadlock.c $(COMMON)
	$(CC) $(CFLAGS) $< -pthread -o $@

clean:
	rm -f mutex_no mutex_yes factorial deadlock

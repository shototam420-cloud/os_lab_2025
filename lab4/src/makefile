CC = gcc
AR = ar
CFLAGS = -std=c11 -O2 -Wall -Wextra
COMMON = ../../common/lab_args.h
.PHONY: all clean timeout_demo
all: parallel_min_max process_memory zombie psum

parallel_min_max: parallel_min_max.c utils.c utils.h find_min_max.c find_min_max.h
	$(CC) $(CFLAGS) parallel_min_max.c utils.c find_min_max.c -o $@

parallel_min_max_slow: parallel_min_max.c utils.c utils.h find_min_max.c find_min_max.h
	$(CC) $(CFLAGS) -DDEMO_SLOW_CHILD parallel_min_max.c utils.c find_min_max.c -o $@

timeout_demo: parallel_min_max_slow
	@echo 'Demo: children sleep for 3 seconds, parent timeout is 1 second.'
	./parallel_min_max_slow --seed 42 --array_size 10000 --pnum 4 --timeout 1; code=$$?; test $$code -eq 124

process_memory: process_memory.c
	$(CC) $(CFLAGS) $< -o $@

zombie: zombie.c
	$(CC) $(CFLAGS) $< -o $@

sum.o: sum.c sum.h
	$(CC) $(CFLAGS) -c sum.c -o $@

libsum.a: sum.o
	$(AR) rcs $@ $<

psum: parallel_sum.c sum.h utils.c utils.h libsum.a $(COMMON)
	$(CC) $(CFLAGS) parallel_sum.c utils.c -L. -lsum -pthread -o $@

clean:
	rm -f parallel_min_max parallel_min_max_slow process_memory zombie psum sum.o libsum.a

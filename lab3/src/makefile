CC = gcc

CFLAGS = -I. -Wall -Wextra -Wpedantic -std=c11

COMMON_OBJECTS = utils.o find_min_max.o

SEED ?= 42
ARRAY_SIZE ?= 200000
PNUM ?= 4

.PHONY: all demo clean

all: sequential_min_max parallel_min_max exec_min_max

sequential_min_max: sequential_min_max.c $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) \
	  sequential_min_max.c \
	  $(COMMON_OBJECTS) \
	  -o $@

parallel_min_max: parallel_min_max.c $(COMMON_OBJECTS)
	$(CC) $(CFLAGS) \
	  parallel_min_max.c \
	  $(COMMON_OBJECTS) \
	  -o $@

exec_min_max: exec_min_max.c
	$(CC) $(CFLAGS) \
	  exec_min_max.c \
	  -o $@

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) \
	  -c utils.c \
	  -o $@

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) \
	  -c find_min_max.c \
	  -o $@

demo: all
	@echo
	@echo "===== SEQUENTIAL ====="
	./sequential_min_max $(SEED) $(ARRAY_SIZE)

	@echo
	@echo "===== PARALLEL: PIPE ====="
	./parallel_min_max \
	  --seed $(SEED) \
	  --array_size $(ARRAY_SIZE) \
	  --pnum $(PNUM)

	@echo
	@echo "===== PARALLEL: FILES ====="
	./parallel_min_max \
	  --seed $(SEED) \
	  --array_size $(ARRAY_SIZE) \
	  --pnum $(PNUM) \
	  --by_files

	@echo
	@echo "===== FORK + EXEC ====="
	./exec_min_max $(SEED) $(ARRAY_SIZE)

clean:
	rm -f \
	  $(COMMON_OBJECTS) \
	  sequential_min_max \
	  parallel_min_max \
	  exec_min_max
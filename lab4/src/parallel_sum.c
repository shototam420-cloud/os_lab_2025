#define _POSIX_C_SOURCE 200809L
#include <getopt.h>
#include <inttypes.h>
#include <pthread.h>
#include <time.h>
#include "../../common/lab_args.h"
#include "utils.h"
#include "sum.h"
int main(int argc, char **argv) {
    int count = -1, n = -1, seed = -1, c;
    const struct option options[] = {
        {"threads_num", required_argument, NULL, 't'},
        {"seed", required_argument, NULL, 's'},
        {"array_size", required_argument, NULL, 'a'}, {0, 0, 0, 0}
    };
    while ((c = getopt_long(argc, argv, "", options, NULL)) != -1) {
        switch (c) {
            case 't': count = number(optarg, 1); break;
            case 's': seed = number(optarg, 0); break;
            case 'a': n = number(optarg, 1); break;
            default: return EXIT_FAILURE;
        }
    }
    if (count < 1 || count > 1024 || n < 1 || seed < 0 || optind != argc) {
        fprintf(stderr, "Usage: %s --threads_num 1..1024 --seed N --array_size N\n", argv[0]);
        return EXIT_FAILURE;
    }
    int *array = checked_calloc((size_t)n, sizeof(*array));
    GenerateArray(array, (unsigned)n, (unsigned)seed);
    pthread_t *threads = checked_calloc((size_t)count, sizeof(*threads));
    struct SumArgs *args = checked_calloc((size_t)count, sizeof(*args));
    struct timespec start, finish;
    if (clock_gettime(CLOCK_MONOTONIC, &start)) { perror("clock_gettime"); return 1; }
    for (int i = 0; i < count; ++i) {
        args[i] = (struct SumArgs){array, (size_t)n * (size_t)i / (size_t)count,
            (size_t)n * (size_t)(i + 1) / (size_t)count, 0};
        thread_check(pthread_create(&threads[i], NULL, ThreadSum, &args[i]), "pthread_create");
    }
    int64_t total = 0;
    for (int i = 0; i < count; ++i) {
        thread_check(pthread_join(threads[i], NULL), "pthread_join");
        total += args[i].result;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &finish)) { perror("clock_gettime"); return 1; }
    double ms = (finish.tv_sec - start.tv_sec) * 1000.0
              + (finish.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("Total: %" PRId64 "\nElapsed time: %.3f ms\n", total, ms);
    free(args); free(threads); free(array);
    return 0;
}

#include <getopt.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>
#include "../../common/lab_args.h"
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static uint64_t result;
struct Args { uint64_t begin, end, mod; };
static void *product(void *data) {
    const struct Args *a = data;
    uint64_t local = 1 % a->mod;
    for (uint64_t i = a->begin; i < a->end; ++i)
        local = (local * i) % a->mod;
    thread_check(pthread_mutex_lock(&mutex), "pthread_mutex_lock");
    result = (result * local) % a->mod;
    thread_check(pthread_mutex_unlock(&mutex), "pthread_mutex_unlock");
    return NULL;
}
int main(int argc, char **argv) {
    int k = -1, count = -1, mod = -1, c;
    const struct option options[] = {
        {"pnum", required_argument, NULL, 'p'},
        {"mod", required_argument, NULL, 'm'}, {0, 0, 0, 0}
    };
    while ((c = getopt_long(argc, argv, "k:", options, NULL)) != -1) {
        switch (c) {
            case 'k': k = number(optarg, 0); break;
            case 'p': count = number(optarg, 1); break;
            case 'm': mod = number(optarg, 1); break;
            default: return EXIT_FAILURE;
        }
    }
    if (k < 0 || count < 1 || count > 1024 || mod < 1 || optind != argc) {
        fprintf(stderr, "Usage: %s -k N --pnum=1..1024 --mod=N\n", argv[0]);
        return EXIT_FAILURE;
    }
    result = 1 % (uint64_t)mod;
    pthread_t *threads = checked_calloc((size_t)count, sizeof(*threads));
    struct Args *args = checked_calloc((size_t)count, sizeof(*args));
    for (int i = 0; i < count; ++i) {
        args[i] = (struct Args){1 + (uint64_t)k * (uint64_t)i / (uint64_t)count,
            1 + (uint64_t)k * (uint64_t)(i + 1) / (uint64_t)count, (uint64_t)mod};
        thread_check(pthread_create(&threads[i], NULL, product, &args[i]), "pthread_create");
    }
    for (int i = 0; i < count; ++i)
        thread_check(pthread_join(threads[i], NULL), "pthread_join");
    printf("%d! mod %d = %" PRIu64 "\n", k, mod, result);
    free(args); free(threads);
    thread_check(pthread_mutex_destroy(&mutex), "pthread_mutex_destroy");
    return 0;
}

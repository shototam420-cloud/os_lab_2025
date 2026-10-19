#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "../../common/lab_args.h"
static int common = 0;
#ifdef USE_MUTEX
static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
#endif
/* Same read-increment-write experiment as the original mutex.c. */
static void *work(void *unused) {
    (void)unused;
    for (int i = 0; i < 50; ++i) {
#ifdef USE_MUTEX
        thread_check(pthread_mutex_lock(&mut), "pthread_mutex_lock");
#endif
        int local = common;
        struct timespec delay = {0, 1000000};
        nanosleep(&delay, NULL);
        common = local + 1;
#ifdef USE_MUTEX
        thread_check(pthread_mutex_unlock(&mut), "pthread_mutex_unlock");
#endif
    }
    return NULL;
}
int main(void) {
    pthread_t a, b;
    thread_check(pthread_create(&a, NULL, work, NULL), "pthread_create");
    thread_check(pthread_create(&b, NULL, work, NULL), "pthread_create");
    thread_check(pthread_join(a, NULL), "pthread_join");
    thread_check(pthread_join(b, NULL), "pthread_join");
    printf("All done, counter = %d (expected 100)\n", common);
#ifdef USE_MUTEX
    thread_check(pthread_mutex_destroy(&mut), "pthread_mutex_destroy");
#endif
    return 0;
}

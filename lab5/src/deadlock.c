#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include "../../common/lab_args.h"
static pthread_mutex_t first = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t second = PTHREAD_MUTEX_INITIALIZER;
static pthread_barrier_t barrier;
struct Locks { pthread_mutex_t *a, *b; int id; };
static void *work(void *data) {
    struct Locks *locks = data;
    thread_check(pthread_mutex_lock(locks->a), "pthread_mutex_lock");
    printf("Thread %d holds its first mutex\n", locks->id);
    int rc = pthread_barrier_wait(&barrier);
    if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
        thread_check(rc, "pthread_barrier_wait");
    printf("Thread %d is waiting for the other mutex\n", locks->id);
    thread_check(pthread_mutex_lock(locks->b), "pthread_mutex_lock");
    thread_check(pthread_mutex_unlock(locks->b), "pthread_mutex_unlock");
    thread_check(pthread_mutex_unlock(locks->a), "pthread_mutex_unlock");
    return NULL;
}
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    thread_check(pthread_barrier_init(&barrier, NULL, 2), "pthread_barrier_init");
    struct Locks a = {&first, &second, 1}, b = {&second, &first, 2};
    pthread_t t1, t2;
    thread_check(pthread_create(&t1, NULL, work, &a), "pthread_create");
    thread_check(pthread_create(&t2, NULL, work, &b), "pthread_create");
    thread_check(pthread_join(t1, NULL), "pthread_join");
    thread_check(pthread_join(t2, NULL), "pthread_join");
    return 0;
}

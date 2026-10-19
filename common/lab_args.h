#ifndef LAB_ARGS_H
#define LAB_ARGS_H
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* All numeric arguments are decimal integers in [minimum, INT_MAX]. */
static inline int number(const char *text, int minimum) {
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno || end == text || *end || value < minimum || value > INT_MAX) {
        fprintf(stderr, "Invalid number: %s (expected %d..%d)\n",
                text, minimum, INT_MAX);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}
static inline void *checked_calloc(size_t n, size_t size) {
    void *p = calloc(n, size);
    if (!p) { perror("calloc"); exit(EXIT_FAILURE); }
    return p;
}
static inline void thread_check(int error, const char *operation) {
    if (error) {
        fprintf(stderr, "%s: %s\n", operation, strerror(error));
        exit(EXIT_FAILURE);
    }
}
#endif

#ifndef SUM_H
#define SUM_H
#include <stddef.h>
#include <stdint.h>
struct SumArgs {
    const int *array;
    size_t begin, end;
    int64_t result;
};
int64_t Sum(const struct SumArgs *args);
void *ThreadSum(void *args);
#endif

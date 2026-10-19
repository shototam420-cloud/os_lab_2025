#include "sum.h"
int64_t Sum(const struct SumArgs *args) {
    int64_t total = 0;
    for (size_t i = args->begin; i < args->end; ++i)
        total += args->array[i];
    return total;
}
void *ThreadSum(void *data) {
    struct SumArgs *args = data;
    args->result = Sum(args);
    return NULL;
}

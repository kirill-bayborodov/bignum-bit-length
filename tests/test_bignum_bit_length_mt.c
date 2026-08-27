#include "bignum_bit_length.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define THREADS 8
#define ITERATIONS 20000

typedef struct {
    const bignum_t *value;
    bignum_t snapshot;
    int failed;
} worker_arg_t;

static size_t oracle(const bignum_t *num)
{
    size_t i = num->len;
    while (i > 0U && num->words[i - 1U] == 0U) --i;
    if (i == 0U) return 0U;
    uint64_t word = num->words[i - 1U];
    size_t result = (i - 1U) * 64U;
    while (word != 0U) { ++result; word >>= 1U; }
    return result;
}

static void *worker(void *opaque)
{
    worker_arg_t *arg = opaque;
    for (size_t i = 0U; i < ITERATIONS; ++i) {
        size_t result = 0U;
        if (bignum_bit_length(arg->value, &result) != BIGNUM_BIT_LENGTH_SUCCESS ||
            result != oracle(arg->value)) { arg->failed = 1; break; }
    }
    if (memcmp(arg->value, &arg->snapshot, sizeof(arg->snapshot)) != 0) arg->failed = 1;
    return NULL;
}

int main(void)
{
    pthread_t threads[THREADS];
    worker_arg_t args[THREADS];
    bignum_t values[THREADS];
    int failed = 0;
    for (size_t i = 0U; i < THREADS; ++i) {
        memset(&values[i], 0, sizeof(values[i]));
        values[i].len = BIGNUM_CAPACITY;
        values[i].words[BIGNUM_CAPACITY - 1U] = UINT64_C(1) << (i % 63U);
        values[i].words[i] = UINT64_MAX;
        args[i].value = &values[i];
        args[i].snapshot = values[i];
        args[i].failed = 0;
        if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0) failed = 1;
    }
    for (size_t i = 0U; i < THREADS; ++i) {
        if (pthread_join(threads[i], NULL) != 0 || args[i].failed) failed = 1;
    }
    printf("bignum_bit_length MT tests: %s\n", failed == 0 ? "PASS" : "FAIL");
    return failed == 0 ? 0 : 1;
}

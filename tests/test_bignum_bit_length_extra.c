#include "bignum_bit_length.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint64_t next_value(uint64_t *state)
{
    *state ^= *state << 7U;
    *state ^= *state >> 9U;
    *state ^= *state << 8U;
    return *state;
}

static size_t oracle(const bignum_t *num)
{
    size_t i = num->len;
    while (i > 0U && num->words[i - 1U] == 0U) --i;
    if (i == 0U) return 0U;
    uint64_t word = num->words[i - 1U];
    size_t bits = (i - 1U) * 64U;
    while (word != 0U) { ++bits; word >>= 1U; }
    return bits;
}

int main(void)
{
    uint64_t seed = UINT64_C(0x123456789abcdef0);
    int failures = 0;
    for (size_t trial = 0U; trial < 20000U; ++trial) {
        bignum_t value;
        bignum_t snapshot;
        size_t result = UINT64_C(0xfeedface);
        value.len = (size_t)(next_value(&seed) % (BIGNUM_CAPACITY + 1U));
        for (size_t i = 0U; i < BIGNUM_CAPACITY; ++i) value.words[i] = next_value(&seed);
        snapshot = value;
        if (bignum_bit_length(&value, &result) != BIGNUM_BIT_LENGTH_SUCCESS ||
            result != oracle(&value) || memcmp(&value, &snapshot, sizeof(value)) != 0) {
            ++failures;
            break;
        }
    }
    bignum_t invalid;
    memset(&invalid, 0xa5, sizeof(invalid));
    invalid.len = BIGNUM_CAPACITY + 1U;
    size_t canary = 777U;
    if (bignum_bit_length(&invalid, &canary) != BIGNUM_BIT_LENGTH_ERROR_LENGTH || canary != 777U) ++failures;
    printf("bignum_bit_length randomized tests: %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}

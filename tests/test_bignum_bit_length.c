#include "bignum_bit_length.h"
#include <stdio.h>
#include <string.h>

static int failures;

static void check(const char *name, int condition)
{
    if (!condition) { fprintf(stderr, "[FAIL] %s\n", name); ++failures; }
}

static size_t oracle(const bignum_t *num)
{
    size_t i = num->len;
    while (i > 0U && num->words[i - 1U] == 0U) --i;
    if (i == 0U) return 0U;
    size_t bits = 0U;
    uint64_t word = num->words[i - 1U];
    while (word != 0U) { ++bits; word >>= 1U; }
    return (i - 1U) * 64U + bits;
}

int main(void)
{
    bignum_t value;
    size_t result;
    memset(&value, 0, sizeof(value));
    result = 99U;
    check("zero status", bignum_bit_length(&value, &result) == BIGNUM_BIT_LENGTH_SUCCESS);
    check("zero length", result == 0U);

    for (size_t bit = 0U; bit < BIGNUM_CAPACITY * 64U; ++bit) {
        memset(&value, 0, sizeof(value));
        value.len = bit / 64U + 1U;
        value.words[bit / 64U] = UINT64_C(1) << (bit % 64U);
        result = 0U;
        check("single-bit oracle", bignum_bit_length(&value, &result) == 0 && result == bit + 1U);
    }

    memset(&value, 0, sizeof(value));
    value.len = BIGNUM_CAPACITY;
    value.words[BIGNUM_CAPACITY - 1U] = UINT64_MAX;
    result = 0U;
    check("maximum value", bignum_bit_length(&value, &result) == 0 && result == BIGNUM_CAPACITY * 64U);

    memset(&value, 0, sizeof(value));
    value.len = 4U;
    value.words[0] = 7U;
    value.words[1] = UINT64_MAX;
    value.words[3] = UINT64_C(0x8000000000000000);
    value.words[10] = UINT64_MAX;
    bignum_t snapshot = value;
    result = 0U;
    check("leading logical zero words", bignum_bit_length(&value, &result) == 0 && result == 4U * 64U);
    check("input unchanged", memcmp(&value, &snapshot, sizeof(value)) == 0);

    result = 123U;
    check("null input", bignum_bit_length(NULL, &result) == BIGNUM_BIT_LENGTH_ERROR_NULL_ARG && result == 123U);
    check("null output", bignum_bit_length(&value, NULL) == BIGNUM_BIT_LENGTH_ERROR_NULL_ARG);
    value.len = BIGNUM_CAPACITY + 1U;
    result = 456U;
    check("invalid length", bignum_bit_length(&value, &result) == BIGNUM_BIT_LENGTH_ERROR_LENGTH && result == 456U);

    value.len = 4U;
    check("final oracle", bignum_bit_length(&value, &result) == 0 && result == oracle(&value));
    printf("bignum_bit_length deterministic tests: %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}

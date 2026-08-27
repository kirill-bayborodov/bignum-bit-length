#include "bignum_bit_length.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    bignum_t value;
    size_t bits = 99U;
    memset(&value, 0, sizeof(value));
    value.len = 1U;
    value.words[0] = UINT64_C(0x8000000000000000);
    if (bignum_bit_length(&value, &bits) != BIGNUM_BIT_LENGTH_SUCCESS || bits != 64U) return 1;
    puts("bignum_bit_length distribution runner: PASS");
    return 0;
}

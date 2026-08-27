/**
 * @file bignum_bit_length.c
 * @brief C11 reference implementation for significant-bit counting.
 */
#include "bignum_bit_length.h"

#include <stdint.h>

/**
 * @brief Validates a bignum logical length.
 * @details A length beyond the fixed physical array cannot be inspected safely.
 * @param[in] num Candidate bignum record.
 * @return Non-zero when the record length is representable.
 */
static int valid_length(const bignum_t *num)
{
    return num->len <= BIGNUM_CAPACITY;
}

/**
 * @brief Computes the bit length of one non-zero 64-bit word.
 * @details The result is one plus the position of the highest set bit.
 * @param[in] word Non-zero unsigned word.
 * @return Significant bit count in the word.
 */
static size_t word_bit_length(uint64_t word)
{
    size_t length = 0U;
    while (word != 0U) {
        ++length;
        word >>= 1U;
    }
    return length;
}

bignum_bit_length_status_t bignum_bit_length(const bignum_t *num,
                                              size_t *bit_length)
{
    size_t word_index;
    size_t result = 0U;

    if (num == NULL || bit_length == NULL) {
        return BIGNUM_BIT_LENGTH_ERROR_NULL_ARG;
    }
    if (!valid_length(num)) {
        return BIGNUM_BIT_LENGTH_ERROR_LENGTH;
    }
    word_index = num->len;
    while (word_index > 0U) {
        uint64_t word = num->words[word_index - 1U];
        if (word != 0U) {
            result = (word_index - 1U) * 64U + word_bit_length(word);
            break;
        }
        --word_index;
    }
    *bit_length = result;
    return BIGNUM_BIT_LENGTH_SUCCESS;
}

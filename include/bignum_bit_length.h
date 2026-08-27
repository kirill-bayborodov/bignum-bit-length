/**
 * @file bignum_bit_length.h
 * @brief Public API for counting significant bits in a fixed-capacity bignum_t.
 * @details The operation reads an unsigned little-endian bignum_t without
 * modifying it. A zero value has bit length zero; otherwise the result is the
 * index of the highest set bit plus one. The API performs no allocation and
 * preserves the output value on every validation failure.
 */
#ifndef BIGNUM_BIT_LENGTH_H
#define BIGNUM_BIT_LENGTH_H

#include <bignum.h>
#include <stddef.h>

#ifndef BIGNUM_CAPACITY
#error "bignum.h must define BIGNUM_CAPACITY"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reports the result of a significant-bit-length query.
 * @details Success writes a value in the inclusive range
 * [0, BIGNUM_CAPACITY * 64]. Failure leaves the caller-provided output
 * unchanged.
 */
typedef enum bignum_bit_length_status {
    BIGNUM_BIT_LENGTH_SUCCESS = 0, /**< The bit length was written successfully. */
    BIGNUM_BIT_LENGTH_ERROR_NULL_ARG = -1, /**< The input or output pointer was NULL. */
    BIGNUM_BIT_LENGTH_ERROR_LENGTH = -2 /**< The input length exceeds capacity. */
} bignum_bit_length_status_t;

/**
 * @brief Counts the significant bits in an unsigned bignum record.
 * @details The function scans only the logical prefix described by `num->len`,
 * ignores physical words above that prefix, finds the most significant non-zero
 * word, and computes its one-based bit position. A zero value returns zero.
 * Neither the input record nor the output is modified before validation passes.
 * @param[in] num Caller-owned bignum record; must be non-NULL and have
 * `len <= BIGNUM_CAPACITY`.
 * @param[out] bit_length Caller-owned size_t receiving the significant-bit
 * count only on success; its prior value is preserved on failure.
 * @return A named bignum_bit_length_status_t value.
 * @pre `num` and `bit_length` point to live storage for the duration of the call.
 * @post On success, `*bit_length` is zero for zero and otherwise equals the
 * highest set bit index plus one. `num` remains byte-for-byte unchanged.
 * @warning Concurrent writers of `num` or `bit_length` require external
 * synchronization. Independent read-only calls are reentrant.
 * @complexity O(BIGNUM_CAPACITY) worst-case time and O(1) auxiliary space.
 */
bignum_bit_length_status_t bignum_bit_length(const bignum_t *num,
                                              size_t *bit_length);

#ifdef __cplusplus
}
#endif

#endif /* BIGNUM_BIT_LENGTH_H */

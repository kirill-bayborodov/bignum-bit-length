# bignum-bit-length Design Notes

## Scope

`bignum-bit-length` counts the significant bits of an unsigned fixed-capacity `bignum_t`. The C11 implementation is the correctness reference and baseline. The YASM implementation is the production x86-64 System V AMD64 path and does not call C helpers.

## Contract

A zero logical value has bit length zero. Otherwise, if the highest non-zero logical word is at index `i` and its highest set bit is at position `j`, the result is `i * 64 + j + 1`. Words at indexes greater than or equal to `num->len` are outside the logical value and are ignored. `num->len > BIGNUM_CAPACITY` is rejected, and the output pointer is never written on failure.

## Implementations

The C11 reference scans the logical prefix from the highest word downwards and computes the final word length with a portable shift loop. The assembly path performs the same bounded scan and uses `bsr` for the final non-zero word. Both implementations use no allocation, no mutable global state and no input mutation. Only caller-saved registers are used by the assembly routine.

## Verification

Deterministic tests cover zero, all single-bit positions, word boundaries, maximum capacity, dirty physical tails, NULL arguments, invalid lengths and output preservation. Randomized tests compare 20,000 generated records to an independent oracle. The multithreaded test uses independent immutable records. C11 coverage is measured separately with gcov; C11 and ASM benchmark matrices use identical manifests and parameters.

## References

1. GNU C Library, `__builtin_clzll` and bit operations: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
2. Intel, `BSR — Bit Scan Reverse`: https://www.felixcloutier.com/x86/bsr

# bignum-bit-length Assembly Review

## Reported issue

The review identified the expression below as a double multiplication by eight:

```asm
lea rax, [rcx*8 - 8]
shl rax, 3
```

That interpretation is incomplete. `lea rax, [rcx*8 - 8]` computes `8 * (rcx - 1)`, which is the byte offset of the highest logical word. The subsequent shift by three converts that byte offset to bits, producing `64 * (rcx - 1)`. Therefore, the old expression was algebraically correct, although it was unnecessarily indirect and easy to misread.

## Corrective implementation

The implementation now expresses the intended formula directly:

```text
word_index = rcx - 1
bit_length = word_index * 64 + bsr(word) + 1
```

The direct sequence is clearer, avoids an address-scaled expression being reused as arithmetic, and makes review of the unit conversion straightforward. `bsr` is executed only after a non-zero word test, so its zero-input undefined result is unreachable.

## Why the original deterministic tests did not report a failure

The old implementation did not produce the alleged eightfold error. In fact, the deterministic suite enumerated every single-bit position from zero through `BIGNUM_CAPACITY * 64 - 1`, and the randomized suite compared 20,000 records with an independent oracle. These tests passed because the old arithmetic was functionally equivalent to the corrected sequence.

The test suite nevertheless lacked a test named specifically for the multi-word arithmetic formula. A focused case at word index three and bit position five has therefore been added. It expects `3 * 64 + 6 = 198`, making the intended calculation explicit and protecting the readability-driven fix.

## Result

The defect was a real maintainability and reviewability problem, not an observed arithmetic failure in the prior binary. The assembly now uses the direct formula, and both ASM and C11 implementations pass the focused regression, exhaustive deterministic tests and randomized oracle tests.

## Additional review findings

The public function returns `bignum_bit_length_status_t`, whose C ABI representation is an `int`-compatible enumeration. Consequently, `mov eax, -1` was already sufficient for ordinary C callers because the caller consumes the 32-bit return value. Nevertheless, the implementation now uses `mov rax, -1` and `mov rax, -2`, which provides an explicitly sign-extended 64-bit register result and is safer for low-level wrappers that inspect the complete register.

`BIGNUM_CAPACITY` is 32 and the shared core definition is exactly `uint64_t words[32]; size_t len;`. The length field is therefore at byte offset `32 * 8 = 256`. The valid logical length range is `0..32`, inclusive: `len == 32` refers to the last valid word at index 31. The existing unsigned `ja` check is correct; changing it to `jae` would incorrectly reject the maximum-capacity value.

The new implementation and tests preserve these conclusions explicitly.

## Final polish verification

A C11 `_Static_assert` now verifies that `offsetof(bignum_t, len)` equals `BIGNUM_CAPACITY * sizeof(uint64_t)`, which is 256 bytes for the current 32-word layout. This turns a previously manual cross-language assumption into a compile-time failure if the core structure changes. The assembly retains `ja` rather than `jae`, because the documented and tested logical length range is inclusive and `len == BIGNUM_CAPACITY` accesses the final valid word.

The complete ASM and C11 suites, sanitizers, Helgrind, lint and Doxygen checks pass after this guard was added.

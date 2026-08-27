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

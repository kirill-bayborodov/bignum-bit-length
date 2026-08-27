; @file bignum_bit_length.asm
; @brief x86-64 implementation for counting significant bits.
; @details System V AMD64 ABI: rdi = const bignum_t*, rsi = size_t* output.
; The routine reads at most 32 little-endian 64-bit words, ignores physical
; words above len, writes output only after validation, makes no calls, and uses
; caller-saved registers only.
section .text

BIGNUM_CAPACITY equ 32
BIGNUM_LEN_OFFSET equ BIGNUM_CAPACITY * 8
SUCCESS equ 0
ERROR_NULL equ -1
ERROR_LENGTH equ -2

global bignum_bit_length
bignum_bit_length:
    test    rdi, rdi
    jz      .error_null
    test    rsi, rsi
    jz      .error_null

    mov     rcx, [rdi + BIGNUM_LEN_OFFSET]
    cmp     rcx, BIGNUM_CAPACITY
    ja      .error_length

    xor     rax, rax
    test    rcx, rcx
    jz      .publish

.scan:
    mov     rdx, [rdi + rcx*8 - 8]
    test    rdx, rdx
    jnz     .found
    dec     rcx
    jnz     .scan
    jmp     .publish

.found:
    bsr     rdx, rdx
    mov     rax, rcx
    dec     rax
    shl     rax, 6
    add     rax, rdx
    inc     rax

.publish:
    mov     [rsi], rax
    xor     rax, rax
    ret

.error_null:
    mov     rax, ERROR_NULL
    ret
.error_length:
    mov     rax, ERROR_LENGTH
    ret

section .note.GNU-stack noalloc noexec nowrite progbits

# gcd_u32_asm: binary GCD, hand-written RV32I.
#
# uint32_t gcd_u32_asm(uint32_t a, uint32_t b)   a in a0, b in a1, result in a0
#
# RV32I and Zicsr only: no mul, div, rem, no floating point, no compressed
# instructions. Tests: tests/tgcd.c.

    .text

    .globl  gcd_u32_asm
    .type   gcd_u32_asm, @function
    .align  2
gcd_u32_asm:

    li t0, 0         # k=0, shift amount

    loop:

        eq_0:
        beq a1, zero, result   # if b == 0: return result
        bne a0, zero, evens    # if a == 0: return result
        mv a0, a1              # a == 0
        j result

        evens:
        andi t1, a0, 1
        andi t2, a1, 1
        or t3, t1, t2          # 0(even) or 0(even) = 0 -> both even
        bne t3, zero, odd_even
        srli a0, a0, 1
        srli a1, a1, 1
        addi t0, t0, 1
        j loop

        odd_even:
        xor t3, t1, t2         # 0(even) xor 1(odd) = 1 -> gcd(2a, b)
        beq t3, zero, odds
        beq t1, zero, a_even_b_odd
        a_odd_b_even:
        srli a1, a1, 1
        j loop
        a_even_b_odd:  
        srli a0, a0, 1
        j loop

        odds:
        bltu a1, a0, a_minus_b
        sub a1, a1, a0
        j loop
        a_minus_b:
        sub a0, a0, a1
        j loop

    result:
    sll a0, a0, t0   # common divisor << k
    ret
    .size   gcd_u32_asm, .-gcd_u32_asm

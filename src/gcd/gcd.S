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

    b_eq_0:
    bne a1, zero, a_eq_0   # if b == 0: return result
    ret
    a_eq_0:
    bne a0, zero, ctz_loop     # if a == 0: return result
    mv a0, a1              # a == 0
    ret

    ctz_loop:
        andi t1, a0, 1
        andi t2, a1, 1
        or t3, t1, t2          # 0(even) or 0(even) = 0 -> both even
        bne t3, zero, a_to_odd
        srli a0, a0, 1
        srli a1, a1, 1
        addi t0, t0, 1
        j ctz_loop

    # make a odd
    a_to_odd:
    bne t1, zero, loop
    srli a0, a0, 1
    andi t1, a0, 1
    j a_to_odd

    # a is always odd
    loop:
        andi t1, a1, 1         # t1 = b & 1
        bne t1, zero, odds     # if b is odd, go to odds 
        srli a1, a1, 1
        j loop

        odds:
        bgeu a1, a0, b_minus_a   # if b >= a
        mv t1, a0
        mv a0, a1
        mv a1, t1
        b_minus_a:
        sub a1, a1, a0           # b = b-a

        # check b == 0
        beq a1, zero, result
        j loop

    result:
    sll a0, a0, t0   # common divisor << k
    ret
    .size   gcd_u32_asm, .-gcd_u32_asm

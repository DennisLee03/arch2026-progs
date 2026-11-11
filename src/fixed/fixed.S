# op1, op2, op3 and q_mul: hand-written RV32I.
#
# uint32_t op1_asm(uint32_t a, uint32_t b, int *n)   a0, a1, a2 -> a0
# uint32_t op2_asm(uint32_t a, uint32_t b)           a0, a1 -> a0
# uint32_t op3_asm(uint32_t a, uint32_t b)           a0, a1 -> a0
# int32_t  q_mul_asm(int32_t a, int32_t b)           a0, a1 -> a0
#
# RV32I and Zicsr only: no mul, div, rem, no floating point, no compressed
# instructions. q_mul forms a 64-bit product in a register pair. Tests:
# tests/tfixed.c.

    .text

    .globl  op1_asm
    .type   op1_asm, @function
    .align  2
op1_asm:
    addi t0, zero, 0           # count for n
    beq a1, zero, op1_end
    op1_loop:
        and t1, a0, a1         # t1 = a & b
        xor a0, a0, a1         # a = a ^ b
        slli a1, t1, 1         # b = (a & b) << 1
        addi t0, t0, 1         # count++
        bne a1, zero, op1_loop
    op1_end:
    sw t0, 0(a2)               # *n = count
    ret
    .size   op1_asm, .-op1_asm

    .globl  op2_asm
    .type   op2_asm, @function
    .align  2
op2_asm:
    sub a0, a0, a1
    ret
    .size   op2_asm, .-op2_asm

    .globl  op3_asm
    .type   op3_asm, @function
    .align  2
op3_asm:
    # (a & b) + ((a ^ b) >> 1)
    and t0, a0, a1
    xor t1, a0, a1
    srli t1, t1, 1
    add a0, t0, t1
    ret
    .size   op3_asm, .-op3_asm

    .globl  q_mul_asm
    .type   q_mul_asm, @function
    .align  2

# definitions for q_mul using t0-t6 and a0-a7 (caller-saved registers)
# t0: sign of product
# t1: m for temporary use
# t2: acc_hi
# t3: acc_lo
# t4: multiplicand
# t5: multier_lo
# t6: multier_hi
# a0: `a`
# a1: `b`
    .equ HALF_Q, 524288
q_mul_asm:
    # 1. record sign of the product (sltu)
    xor t0, a0, a1
    slt t0, t0, zero 

    # 2. absolute value without branch (m = srai x >> 31, |x| = (x XOR m) - m)
    srai t1, a0, 31
    xor t4, a0, t1
    sub t4, t4, t1
    srai t1, a1, 31
    xor t5, a1, t1
    sub t5, t5, t1
    add t6, zero, zero

    # shift and add: in acc_{hi, lo}
    add t2, zero, zero
    add t3, zero, zero
    beq t4, zero, q_mul_end
    q_mul_loop:
        andi t1, t4, 1
        beq t1, zero, q_mul_update_ops  # check valid addition
        add t3, t3, t5
        sltu t1, t3, t5                 # carry in m
        add t2, t2, t6
        add t2, t2, t1
        q_mul_update_ops:
        slli t6, t6, 1                  # update multiplier
        srli t1, t5, 31
        or t6, t6, t1
        slli t5, t5, 1
        srli t4, t4, 1                  # update multiplicand
        bne t4, zero, q_mul_loop

    q_mul_end:
    beq t0, zero, round                 # sign == 0: skip changing sign
    not t3, t3                          # change sign: acc_lo
    not t2, t2                          #            : acc_hi
    addi t3, t3, 1                      #            : acc_lo + 1
    seqz t1, t3                         # carry in m
    add t2, t2, t1                      # add carry to acc_hi
    round:
    li t1, HALF_Q
    add t3, t3, t1
    sltu t1, t3, t1                     # carry in m
    add t2, t2, t1
    slli t2, t2, 12
    srli t3, t3, 20
    or a0, t2, t3
    ret
    .size   q_mul_asm, .-q_mul_asm

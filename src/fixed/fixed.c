#include "fixed.h"

/* Dummies: every function returns 0. Replace with the real routines. */
uint32_t op1(uint32_t a, uint32_t b, int *n)
{
    (void) a;
    (void) b;
    *n = 0;
    return 0;
}

uint32_t op2(uint32_t a, uint32_t b)
{
    (void) a;
    (void) b;
    return 0;
}

uint32_t op3(uint32_t a, uint32_t b)
{
    (void) a;
    (void) b;
    return 0;
}

/**
 * \brief Compute 32-bit multiplication of two numbers in Q11.20 format, wrap
 * around, no saturation
 * \param a operand 1 of multiplication
 * \param b operand 2 of multiplication
 * \return a product of `a` and `b`
 */
int32_t q_mul(int32_t a, int32_t b)
{
    // record sign of the product
    int32_t s_a = a < 0;
    int32_t s_b = b < 0;
    int32_t s = s_a ^ s_b;

    // change operands to positive
    uint32_t multiplicand = (s_a ? ~(uint32_t) a + 1 : (uint32_t) a);
    uint64_t multiplier = (s_b ? ~(int64_t) b + 1 : (int64_t) b);

    // shift and add
    uint64_t acc = 0;
    while (multiplicand) {
        acc += (multiplicand & 1) ? multiplier : 0;
        multiplier <<= 1;
        multiplicand >>= 1;
    }
    acc = s ? ~acc + 1 : acc;
    acc += 524288;
    acc >>= 20;
    return (int32_t) acc;
}

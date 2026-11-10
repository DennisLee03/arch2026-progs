#include "fixed.h"

/* Compute `a` + `b` and count number of iterations of the loop */
uint32_t op1(uint32_t a, uint32_t b, int *n)
{
    *n = 0;
    while (b) {
        uint32_t c = (a & b) << 1;
        a = a ^ b;
        b = c;
        (*n)++;
    }
    return a;
}

/* Compute `a` - `b` */
uint32_t op2(uint32_t a, uint32_t b)
{
    return a - b;
}

/* Compute average of `a` and `b` without overflow */
uint32_t op3(uint32_t a, uint32_t b)
{
    // (a >> 1) + (b >> 1) + (a & b & 1)
    return (a & b) + ((a ^ b) >> 1);
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

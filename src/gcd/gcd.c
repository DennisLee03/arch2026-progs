#include "gcd.h"

/* Binary GCD for two 32-bit unsigned integers */
uint32_t gcd_u32(uint32_t a, uint32_t b)
{
    uint32_t gcd = 0, k = 0;
    while (1) {
        // gcd(0, x) = x
        if (a == 0) {
            gcd = b;
            break;
        }
        if (b == 0) {
            gcd = a;
            break;
        }

        // gcd(2a, 2b), both even
        if ((a & 1) == 0 && (b & 1) == 0) {
            a >>= 1;
            b >>= 1;
            k++;
            continue;
        }

        // gcd(2a, b), 1 even 1 odd
        if ((a & 1) == 0 && (b & 1) == 1) {
            a >>= 1;
            continue;
        }
        if ((a & 1) == 1 && (b & 1) == 0) {
            b >>= 1;
            continue;
        }

        // both odd
        if (b >= a)
            b = b - a;
        else
            a = a - b;
    }

    return gcd << k;
}

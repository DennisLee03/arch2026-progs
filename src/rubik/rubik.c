#include "rubik.h"

// constants for minirubik solver
enum {
    CUBIES = 7,
    PERMUTATIONS = 5040,
    ORIENTATIONS = 729,
    STATES = PERMUTATIONS * ORIENTATIONS,
    MOVES = 9
};


// definition of state
typedef struct {
    uint8_t p[CUBIES], o[CUBIES];
} state_t;

// sum in [0, 14]: sum%3 == sum_mod3[sum]
static const uint8_t sum_mod3[15] = {0, 1, 2, 0, 1, 2, 0, 1,
                                     2, 0, 1, 2, 0, 1, 2};

// i in [0, 13]: i%7 == i_mod7[i]
static const uint8_t i_mod7[14] = {0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6};

/**
 * \brief Validate a rubik's state
 * \param state state of a rubik
 * \return 0(invalid), 1(valid)
 */
static int valid(const state_t *state)
{
    uint8_t sum = 0;

    // Iterate each cubie's perm and orie
    for (uint8_t i = 0; i < CUBIES; ++i) {
        // Check each cubie's perm(0~6) and orie(0~2) are out of range
        if (state->p[i] >= CUBIES || state->o[i] >= 3)
            return 0;

        // Check if there are repeated cubies
        for (uint8_t j = 0; j < i; ++j)
            if (state->p[j] == state->p[i])
                return 0;
        /**
         *         i
         * P P P P P P P
         * j j j j
         */

        // Accumulate orientations
        sum = (uint8_t) (sum + state->o[i]);
    }

    // Check the sum of orientations
    return sum_mod3[sum] == 0;
}

/**
 * \brief Parse input string to a rubik's state, Returns its validity
 * \param input string to be parsed and checked
 * \param state buffer of a given rubik string
 * \return 0(invalid), 1(valid)
 */
static int parse_state(const char *input, state_t *state)
{
    // input string "PPPPPPPOOOOOOO": P is '1'~'7', O is '1'~'3'
    for (int i = 0; i < 14; ++i) {
        int limit = i < 7 ? 7 : 3;

        // if input[i] is out of range: P is '1'~'7', O is '1'~'3': return
        // invalid P, O < '1' || P > '7', O > '3'
        if (input[i] < '1' || input[i] > '0' + limit)
            return 0;

        // Normalize input values to fit state representation and algorithms
        (i < 7 ? state->p : state->o)[i_mod7[i]] = (uint8_t) (input[i] - '1');
    }

    // Check strlen(input) > 14 and validate perm and orie
    return input[14] == '\0' && valid(state);
}

/**
 * \brief Optimal solver for the 2x2x2 Rubik's Cube
 * \param state string of a rubik
 * \param out   string buffer for a optimal solution for a given rubik
 * \param cap
 * \return -1(invalid state), -2(), length(successfully solved)
 */
int rubik_solve(const char *state, char *out, size_t cap)
{
    state_t s = {0};
    if (!parse_state(state, &s))
        return -1;
    (void) out;
    (void) cap;
    return 0;
}

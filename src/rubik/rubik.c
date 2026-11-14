#include "rubik.h"
#include <string.h>
#include "tables.h"

#define FACES "RBD"

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

// face and turns for moves
static const uint8_t move_face[9] = {0, 0, 0, 1, 1, 1, 2, 2, 2};
static const uint8_t move_turns[9] = {1, 2, 3, 1, 2, 3, 1, 2, 3};

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
 * \brief Rank `state` for Lehmer rank of its perm
 * \param state a representation of minirubik
 * \return permutation rank of `state`
 */
static uint16_t rank_perm(const state_t *state)
{
    uint16_t p = 0;
    uint8_t smaller[7] = {0};  // 7 bytes
    for (uint8_t i = 0; i < CUBIES; ++i) {
        for (uint8_t j = (uint8_t) (i + 1U); j < CUBIES; ++j)
            if (state->p[j] < state->p[i])
                smaller[i] += 1;
        /**
         * p = p * (CUBIES - i) + smaller[i];
         * -----------------------------------
         * i=0, p*7: p = (p << 3) - p + smaller[0]
         * i=1, p*6: p = (p << 2) + (p << 1) + smaller[1]
         * i=2, p*5: p = (p << 2) + p + smaller[2]
         * i=3, p*4: p = (p << 2) + smaller[3]
         * i=4, p*3: p = (p << 2) - p + smaller[4]
         * i=5, p*2: p = (p << 1) + smaller[5]
         * i=6, p*1: p = p + smaller[6]
         */
    }

    // unroll:
    p = smaller[0];
    p = (p << 2) + (p << 1) + smaller[1];
    p = (p << 2) + p + smaller[2];
    p = (p << 2) + smaller[3];
    p = (p << 2) - p + smaller[4];
    p = (p << 1) + smaller[5];

    return p;
}

/**
 * \brief Rank `state` as a base-3 number
 * \param state a representation of minirubik
 * \return orientation rank of `state`
 */
static uint16_t rank_orie(const state_t *state)
{
    uint16_t o = 0;
    for (uint8_t i = 0; i < 6; ++i)
        o = (o << 1) + o /* x3 */ + state->o[i];
    return o;
}

// recorded path to solved state
static uint8_t path[11];

// heuristic fucntion
#define h(p, o) (hp[p] > ho[o] ? hp[p] : ho[o])

/**
 * \brief Explore (`p`, `o`)  to (0, 0) in DFS manner with the heuristic
 * \param p permutation of current state
 * \param o orientation of current state
 * \param g current distance
 * \param last_face last face turned, for free pruning
 * \param bound current budget
 * \return 0(not found), 1(found)
 */
static uint8_t dfs(uint16_t p,
                   uint16_t o,
                   uint16_t g,
                   uint8_t last_face,
                   uint8_t bound)
{
    // solved!
    if (p == 0 && o == 0)
        return 1;

    // prune
    //       g: current distance paid;
    // h(p, o): heuristic distance remain
    if ((g + (h(p, o))) > bound)
        return 0;

    for (uint8_t face = 0; face < 3; face++) {
        // free pruning
        if (face == last_face)
            continue;

        // search
        uint16_t next_p = p, next_o = o;
        for (uint8_t turn = 0; turn < 3; turn++) {
            // try to turn X(0), X2(1), X'(2)
            next_p = permutation[face][next_p];
            next_o = orientation[face][next_o];
            path[g] = (face << 1) + face + turn;
            if (dfs(next_p, next_o, g + 1, face, bound))
                return 1;  // found
        }
    }

    return 0;  // not found
}

/**
 * \brief Solve by IDA* from a given state to solved state
 * \param p0 permutation of the given state
 * \param o0 orientation of the given state
 * \return number of moves to solved
 */
static uint8_t ida_star(uint16_t p0, uint16_t o0)
{
    // Reset path
    memset(path, 0xFF, 11);

    // Estmate #moves from the given state to the solved state
    uint8_t bound = h(p0, o0);

    // DFS from (p0, o0) with the estimated budget
    while (bound <= 11) {
        // Search with the budget
        if (dfs(p0, o0, 0, 0xFF, bound))
            return bound;

        // Increase budget
        bound++;
    }

    return 0;  // error. 0 moves with a unsolved state
}

/**
 * \brief Optimal solver for the 2x2x2 Rubik's Cube
 * \param state string of a rubik
 * \param out string buffer for a optimal solution for a given rubik
 * \param cap maximal chars can be hold in `out`
 * \return -1(invalid state), -2(solution out of capacity), length(successfully
 * solved)
 */
int rubik_solve(const char *state, char *out, size_t cap)
{
    state_t s = {0};

    // Parse and Check
    if (!parse_state(state, &s))
        return -1;

    // Rank the state
    uint16_t p = rank_perm(&s);
    uint16_t o = rank_orie(&s);

    // Solve
    uint8_t moves = ida_star(p, o);

    // no sprintf, strcpy, strcat

    // Calculate need, Check capacity
    uint8_t need = moves ? 0 : 1;
    for (uint8_t i = 0; i < moves; i++) {
        need += (move_turns[path[i]] != 1) ? 3 : 2;  // len + white space
    }
    if (need > cap) {
        if (cap >= 1)
            out[0] = '\0';
        return -2;
    }

    // Fill moves with the need
    uint8_t c = 0;
    for (uint8_t i = 0; i < moves; i++) {
        uint8_t move = path[i];
        // 1. face char
        out[c++] = FACES[move_face[move]];

        // 2. (optional) 2 or '
        if (move_turns[move] == 2)
            out[c++] = '2';
        else if (move_turns[move] == 3)
            out[c++] = '\'';

        out[c++] = ' ';
    }
    out[moves ? c - 1 : 0] = '\0';

    return moves;
}

#include "shared/prng/biski.h"

/**
 * @internal
 * @brief Advances a 64-bit SplitMix64 PRNG state and returns a pseudo-random
 * number.
 *
 * This is used internally by seeding functions to expand a single 64-bit seed
 * into the full BiskiState.
 *
 * @param seed_state_ptr Pointer to the 64-bit state of the SplitMix64
 * generator. This state is advanced by the function. It is the caller's
 * responsibility to ensure this pointer is not NULL.
 * @return A 64-bit pseudo-random unsigned integer.
 */
static U64 splitmix64_next(U64 *seed_state_ptr) {
    U64 z = (*seed_state_ptr += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

/**
 * @brief A private helper to warm up the generator by cycling it several times.
 *
 * This function should be called after seeding to discard the initial states,
 * which might have some statistical weaknesses. The `static` keyword limits
 * its scope to this file, making it a private helper.
 *
 * @param state Pointer to the BiskiState structure to be warmed up.
 */
static void biski64_warmup(BiskiState *state) {
    for (U32 i = 0; i < 16; ++i) {
        biskiNext(state); // Assumes this function advances the state
    }
}

/**
 * @brief Initializes the state of a biski64 PRNG instance from a single 64-bit
 * seed.
 *
 * Uses a SplitMix64 generator to derive the initial values for all internal
 * state variables (`fastLoop`, `mix`, `loopMix`) from the provided seed.
 * This ensures that different seeds produce well-distributed initial states.
 * Suitable for single-threaded use or when parallel stream spacing is not
 * required.
 *
 * @param state Pointer to the BiskiState structure to be initialized.
 * The caller must ensure this pointer is not NULL.
 * @param seed  The 64-bit value to use as the seed.
 */
void biskiSeed(BiskiState *state, U64 seed) {
    // It is the caller's responsibility to ensure 'state' is not NULL.
    U64 seeder_state = seed;

    // Derive initial values for each biski64 state variable.
    state->mix = splitmix64_next(&seeder_state);
    state->loopMix = splitmix64_next(&seeder_state);
    state->fastLoop = splitmix64_next(&seeder_state);

    biski64_warmup(state);
}

/**
 * @brief Initializes the state of a biski64 PRNG stream when using parallel
 * streams.
 *
 * Initializes `mix` and `loopMix` from the provided `seed` using SplitMix64.
 * Initializes `fastLoop` based on `streamIndex` and `totalNumStreams` to
 * ensure distinct, well-spaced sequences for parallel execution.
 *
 * @param state Pointer to the BiskiState structure to be initialized.
 * The caller must ensure this pointer is not NULL.
 * @param seed The base 64-bit value to use for seeding `mix` and `loopMix`.
 * @param streamIndex The index of the current stream (0 to totalNumStreams-1).
 * The caller must ensure 0 <= streamIndex < totalNumStreams.
 * @param totalNumStreams The total number of streams.
 * The caller must ensure this is >= 1.
 */
// static void biski64_stream(BiskiState *state, U64 seed, int streamIndex,
//                            int totalNumStreams) {
//     // It is the caller's responsibility to ensure 'state' is not NULL,
//     // totalNumStreams >= 1, and 0 <= streamIndex < totalNumStreams.
//
//     U64 seeder_state = seed;
//
//     state->mix = splitmix64_next(&seeder_state);
//     state->loopMix = splitmix64_next(&seeder_state);
//
//     if (totalNumStreams == 1)
//         state->fastLoop = splitmix64_next(&seeder_state);
//     else {
//         // Space out fastLoop starting values for parallel streams.
//         U64 cyclesPerStream = ((U64)-1) / ((U64)totalNumStreams);
//         state->fastLoop =
//             (U64)streamIndex * cyclesPerStream * 0x9999999999999999ULL;
//     }
//
//     biski64_warmup(state);
// }

/**
 * @internal
 * @brief Performs a 64-bit left rotation.
 *
 * @param x The value to rotate.
 * @param k The number of bits to rotate by. Must be in the range [0, 63].
 * @return The result of rotating x left by k bits.
 */
static inline U64 rotate_left(const U64 x, int k) {
    // Assuming k is within valid range [0, 63] as per function contract.
    return (x << k) | (x >> (-k & 63));
}

/**
 * @brief Generates the next 64-bit pseudo-random number from a biski64 PRNG
 * instance.
 *
 * Advances the PRNG state and returns a new pseudo-random number.
 *
 * @param state Pointer to the BiskiState structure. Must have been
 * initialized by a seeding function. The caller must ensure this pointer is not
 * NULL.
 * @return A 64-bit pseudo-random unsigned integer.
 */
U64 biskiNext(BiskiState *state) {
    // It is the caller's responsibility to ensure 'state' is not NULL and
    // initialized.

    const U64 output = state->mix + state->loopMix;
    const U64 old_loopMix = state->loopMix;

    state->loopMix = state->fastLoop ^ state->mix;
    state->mix = rotate_left(state->mix, 16) + rotate_left(old_loopMix, 40);
    state->fastLoop +=
        0x9999999999999999ULL; // Additive constant for the Weyl sequence.

    return output;
}

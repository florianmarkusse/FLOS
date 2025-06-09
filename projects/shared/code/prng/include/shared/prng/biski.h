#ifndef SHARED_PRNG_BISKI_H
#define SHARED_PRNG_BISKI_H

#include "shared/types/numeric.h"

// Taken from https://github.com/danielcota/biski64 . Very cool PRNG
// implementation!

typedef struct {
    U64 fastLoop;
    U64 mix;
    U64 loopMix;
} BiskiState;

void biskiSeed(BiskiState *state, U64 seed);
U64 biskiNext(BiskiState *state);

#endif

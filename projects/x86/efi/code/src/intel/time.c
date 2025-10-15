#include "x86/efi/time.h"
#include "shared/types/numeric.h"
#include "x86/configuration/cpu.h"

static constexpr auto HERTZ_PER_MEGAHERTZ = 1000000;
static constexpr auto HERTZ_MINIMUM_SPEED = 500000000;
static constexpr auto MEGAHERTZ_MINIMUM_SPEED =
    HERTZ_MINIMUM_SPEED / HERTZ_PER_MEGAHERTZ;

typedef struct {
    U32 tscFreqToCrystalClockDenominator;
    U32 tscFreqToCrystalClockNumerator;
    U32 crystalClockFreqHertz;
} Leaf0x15;

typedef struct {
    U32 processorBaseFreqMHertz;
    U32 processorMaxFreqMHertz;
    U32 busFreqMHertz;
} Leaf0x16;

U64 timestampFrequencyGet() {
    Leaf0x15 leaf0x15;
    {
        CPUIDResult leaf = CPUID(TSC_AND_CORE_CRYSTAL_FREQ);
        leaf0x15 = (Leaf0x15){.tscFreqToCrystalClockDenominator = leaf.eax,
                              .tscFreqToCrystalClockNumerator = leaf.ebx,
                              .crystalClockFreqHertz = leaf.ecx};
    }
    // Not completely supported, sadly
    if (leaf0x15.tscFreqToCrystalClockNumerator &&
        leaf0x15.crystalClockFreqHertz > HERTZ_MINIMUM_SPEED) {
        return (leaf0x15.crystalClockFreqHertz *
                (leaf0x15.tscFreqToCrystalClockNumerator /
                 leaf0x15.tscFreqToCrystalClockDenominator));
    } else {
        // Calculate it through other leaf if possible
        Leaf0x16 leaf0x16;
        {
            CPUIDResult leaf = CPUID(PROCESSOR_AND_BUS_FREQ);
            leaf0x16 = (Leaf0x16){.processorBaseFreqMHertz = leaf.eax,
                                  .processorMaxFreqMHertz = leaf.ebx,
                                  .busFreqMHertz = leaf.ecx};
        }
        if (leaf0x16.processorBaseFreqMHertz > MEGAHERTZ_MINIMUM_SPEED) {
            // no-op, but for copletion's sake.
            return (leaf0x16.processorBaseFreqMHertz * HERTZ_PER_MEGAHERTZ);
        }
    }
}

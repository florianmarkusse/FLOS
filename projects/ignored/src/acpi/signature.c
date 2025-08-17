#include "acpi/signatures.h"    // for ACPITable, ERROR_AND_NUM_TABLES, ACPIT...
#include "shared/text/string.h" // for STRING, string, stringEquals
#include "shared/types/numeric.h" // for U64

static string signatures[] = {STRING("FACP"), STRING("APIC"), STRING("HPET"),
                              STRING("MCFG"), STRING("WAET")};

ACPITable ACPITablesToEnum(string signature) {
    for (U32 i = 0; i < ERROR_AND_NUM_TABLES; i++) {
        if (stringEquals(signature, signatures[i])) {
            return (ACPITable)i;
        }
    }
    return ERROR_AND_NUM_TABLES;
}

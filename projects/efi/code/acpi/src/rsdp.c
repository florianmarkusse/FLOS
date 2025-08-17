/*#include "efi/acpi/configuration-table.h" // for ConfigurationTable*/
#include "abstraction/memory/manipulation.h" // for memcmp
#include "efi/acpi/configuration-table.h"
#include "efi/acpi/guid.h"
#include "efi/acpi/rdsp.h" // for RSDP_REVISION_2, CAcpiRSDPV1, CAcpi...
#include "shared/macros.h"
#include "shared/types/numeric.h" // for USize, U8, nullptr, U16, U64
#include "shared/uuid.h"          // for Guid, ACPI_TABLE_GUID, EFI_ACPI_20_...

bool ACPIChecksum(void *ptr, U64 size) {
    U8 sum = 0;
    for (typeof(size) i = 0; i < size; i++) {
        sum += ((U8 *)ptr)[i];
    }
    return sum == 0;
}

typedef struct {
    UUID guid;
    USize size;
    RSDPRevision revision;
    U16 *string;
} RSDPStruct;

static RSDPStruct possibleRsdps[] = {
    {.guid = ACPI_TABLE_GUID,
     .size = sizeof(CAcpiRSDPV1),
     .revision = RSDP_REVISION_1,
     .string = u"RSDP REVISION 1"},
    {.guid = EFI_ACPI_20_TABLE_GUID,
     .size = sizeof(CAcpiRSDPV2),
     .revision = RSDP_REVISION_2,
     .string = u"RSDP REVISION 2"},
};
static constexpr auto POSSIBLE_RSDPS_LEN = COUNTOF(possibleRsdps);

RSDPResult getRSDP(USize tableEntries, ConfigurationTable *tables) {
    RSDPResult rsdp = {.rsdp = nullptr};
    for (typeof(tableEntries) i = 0; i < tableEntries; i++) {
        ConfigurationTable *cur_table = &tables[i];

        for (typeof_unqual(POSSIBLE_RSDPS_LEN) i = 0; i < POSSIBLE_RSDPS_LEN;
             i++) {
            if (memcmp(&cur_table->vendor_guid, &possibleRsdps[i].guid,
                       sizeof(UUID)) != 0) {
                continue;
            }
            if (!ACPIChecksum(cur_table->vendor_table, possibleRsdps[i].size)) {
                continue;
            }

            // We want to return the newest version if it exists rather then
            // returning the older version. We need to add a check for that
            // since the table entries are not in the same order for all EFI
            // systems.
            rsdp.rsdp = (void *)cur_table->vendor_table;
            rsdp.revision = possibleRsdps[i].revision;
            if (rsdp.revision == RSDP_REVISION_2) {
                return rsdp;
            }
        }
    }

    return rsdp;
}

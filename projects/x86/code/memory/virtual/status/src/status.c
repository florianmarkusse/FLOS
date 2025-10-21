#include "abstraction/memory/virtual/status.h"
#include "shared/log.h"
#include "shared/memory/management/page.h"
#include "shared/memory/management/status.h"
#include "shared/types/numeric.h"
#include "x86/memory/definitions.h"
#include "x86/memory/virtual.h"

static void mappingAppend(U64 addressVirtual[4], U64 physicalAddress,
                          U64_pow2 mappingSize) {
    U64 virtualAddress = addressVirtual[0] + addressVirtual[1] +
                         addressVirtual[2] + addressVirtual[3];
    mappingMemoryAppend(virtualAddress, physicalAddress, mappingSize);
}

static void memoryVirtualMappingTableAppend() {
    for (U32 i = 0; i < PageTableFormat.ENTRIES; i++) {
        VirtualPageTable *pageTable = pageTableRoot;
        U64 entries[4] = {0};
        U64 addressVirtual[4] = {0};
        addressVirtual[0] = 0;
        U64 pageSize = PAGE_ROOT_ENTRY_MAX_SIZE;
        if (i >= 256) {
            addressVirtual[0] = 0xFFFF000000000000ULL + (i * pageSize);
        } else {
            addressVirtual[0] = i * pageSize;
        }
        U64 addressPhysical = 0;

        entries[0] = pageTable->pages[i];
        if (!entries[0]) {
            continue;
        }

        for (U32 j = 0; j < PageTableFormat.ENTRIES; j++) {
            pageSize = X86_1GIB_PAGE;
            pageTable =
                (VirtualPageTable *)(entries[0] &
                                     VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE);
            addressVirtual[1] = j * pageSize;
            entries[1] = pageTable->pages[j];
            if (!entries[1]) {
                continue;
            }

            if (entries[1] & (VirtualPageMasks.PAGE_EXTENDED_SIZE)) {
                addressPhysical =
                    entries[1] & VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE;
                mappingAppend(addressVirtual, addressPhysical, pageSize);
                continue;
            }

            for (U32 k = 0; k < PageTableFormat.ENTRIES; k++) {
                pageSize = X86_2MIB_PAGE;
                pageTable = (VirtualPageTable *)(entries[1] &
                                                 VirtualPageMasks
                                                     .FRAME_OR_NEXT_PAGE_TABLE);

                addressVirtual[2] = k * pageSize;
                entries[2] = pageTable->pages[k];
                if (!entries[2]) {
                    continue;
                }

                if (entries[2] & (VirtualPageMasks.PAGE_EXTENDED_SIZE)) {
                    addressPhysical =
                        entries[2] & VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE;
                    mappingAppend(addressVirtual, addressPhysical, pageSize);
                    continue;
                }

                for (U32 l = 0; l < PageTableFormat.ENTRIES; l++) {
                    pageSize = X86_4KIB_PAGE;
                    pageTable =
                        (VirtualPageTable *)(entries[2] &
                                             VirtualPageMasks
                                                 .FRAME_OR_NEXT_PAGE_TABLE);
                    addressVirtual[3] = l * pageSize;
                    entries[3] = pageTable->pages[l];
                    if (!entries[3]) {
                        continue;
                    }

                    addressPhysical =
                        entries[3] & VirtualPageMasks.FRAME_OR_NEXT_PAGE_TABLE;
                    mappingAppend(addressVirtual, addressPhysical, pageSize);
                }
            }
        }
    }
}

static void memoryVirtualCustomMappingAppend() {
    VMMNode *tree = memoryMapperSizes.tree;

    if (!memoryMapperSizes.tree) {
        return;
    }

    VMMNode *buffer[2 * RB_TREE_MAX_HEIGHT];
    buffer[0] = tree;
    U32 len = 1;
    VMMNode *node;
    RB_TREE_TRAVERSAL_PRE_ORDER(node, len, buffer) {
        if (!node->mappingSize) {
            mappingVirtualGuardPageAppend(node->basic.value, node->bytes);
        } else {
            memoryAppend(
                (Memory){.start = node->basic.value, .bytes = node->bytes});
            INFO(STRING(" -> [CUSTOM MAP REGIONS, CUSTOM MAP REGIONS] size: "));
            INFO(node->mappingSize, .flags = NEWLINE);
        }
    }
}

void memoryVirtualMappingStatusAppend() {
    memoryVirtualMappingTableAppend();
    memoryVirtualCustomMappingAppend();
}

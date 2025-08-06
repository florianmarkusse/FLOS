#ifndef EFI_FIRMWARE_MEMORY_H
#define EFI_FIRMWARE_MEMORY_H

#include "shared/enum.h"
#include "shared/types/numeric.h"

#define EFI_MEMORY_TYPE_ENUM(VARIANT)                                          \
    VARIANT(RESERVED_MEMORY_TYPE)                                              \
    VARIANT(LOADER_CODE)                                                       \
    VARIANT(LOADER_DATA)                                                       \
    VARIANT(BOOT_SERVICES_CODE)                                                \
    VARIANT(BOOT_SERVICES_DATA)                                                \
    VARIANT(RUNTIME_SERVICES_CODE)                                             \
    VARIANT(RUNTIME_SERVICES_DATA)                                             \
    VARIANT(CONVENTIONAL_MEMORY)                                               \
    VARIANT(UNUSABLE_MEMORY)                                                   \
    VARIANT(ACPI_RECLAIM_MEMORY)                                               \
    VARIANT(ACPI_MEMORY_NVS)                                                   \
    VARIANT(MEMORY_MAPPED_IO)                                                  \
    VARIANT(MEMORY_MAPPED_IO_PORT_SPACE)                                       \
    VARIANT(PAL_CODE)                                                          \
    VARIANT(PERSISTENT_MEMORY)                                                 \
    VARIANT(UNACCEPTED_MEMORY_TYPE)                                            \
    VARIANT(MAX_MEMORY_TYPE)

typedef enum : U32 { EFI_MEMORY_TYPE_ENUM(ENUM_STANDARD_VARIANT) } MemoryType;

static constexpr auto EFI_MEMORY_TYPE_COUNT = EFI_MEMORY_TYPE_ENUM(PLUS_ONE);
static String efiMemoryStatusStrings[EFI_MEMORY_TYPE_COUNT] = {
    EFI_MEMORY_TYPE_ENUM(ENUM_TO_STRING)};

static constexpr U64 MEMORY_UC = 0x0000000000000001;
static constexpr U64 MEMORY_WC = 0x0000000000000002;
static constexpr U64 MEMORY_WT = 0x0000000000000004;
static constexpr U64 MEMORY_WB = 0x0000000000000008;
static constexpr U64 MEMORY_UCE = 0x0000000000000010;
static constexpr U64 MEMORY_WP = 0x0000000000001000;
static constexpr U64 MEMORY_RP = 0x0000000000002000;
static constexpr U64 MEMORY_XP = 0x0000000000004000;
static constexpr U64 MEMORY_NV = 0x0000000000008000;
static constexpr U64 MEMORY_MORE_RELIABLE = 0x0000000000010000;
static constexpr U64 MEMORY_RO = 0x0000000000020000;
static constexpr U64 MEMORY_RUNTIME = 0x8000000000000000;

static constexpr U32 MEMORY_DESCRIPTOR_VERSION = 0x00000001;

typedef struct MemoryDescriptor {
    MemoryType type;
    U64 physicalStart;
    U64 virtualStart;
    U64 numberOfPages;
    U64 attribute;
} MemoryDescriptor;

#endif

#ifndef ABSTRACTION_MEMORY_MANIPULATION_H
#define ABSTRACTION_MEMORY_MANIPULATION_H

#ifdef FREESTANDING
#include "freestanding/memory/manipulation.h"
#elif EFI
#include "freestanding/memory/manipulation.h"
#elif POSIX
#include "posix/memory/manipulation.h"
#else
#error ABSTRACTION_MEMORY_MANIPULATION_H
#endif

#endif

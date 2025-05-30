#ifndef POSIX_FILE_READ_H
#define POSIX_FILE_READ_H

#include "file-status.h"                   // for flo_FileStatus
#include "shared/memory/allocator/arena.h" // for Arena
#include "shared/text/string.h"            // for string
#include "shared/types/numeric.h"

FileStatus readFile(U8 *srcPath, string *buffer, Arena *perm);
U64 getFileSize(int fd);

#endif

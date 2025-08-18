#include "posix/file/path.h"

#include <sys/stat.h> // for mkdir

#include "posix/memory/manipulation.h"
#include "shared/memory/allocator/arena.h" // for FLO_NEW, Arena
#include "shared/text/string.h"            // for firstOccurenceOfFrom, string
#include "shared/types/numeric.h"

static constexpr auto FULL_ACCESS = 0700;

void createPath(String fileLocation, Arena scratch) {
    U32 currentIndex = 0;
    I64 slashIndex = firstOccurenceOf(fileLocation, '/', .from = currentIndex);
    if (slashIndex >= 0) {
        U8 *dirPath = NEW(&scratch, U8, .count = fileLocation.len + 1);
        memcpy(dirPath, fileLocation.buf, fileLocation.len);
        dirPath[fileLocation.len] = '\0';

        while (slashIndex > 0) {
            dirPath[slashIndex] =
                '\0'; // Temporarily terminate the string at the next slash
            mkdir((char *)dirPath, FULL_ACCESS); // Create the directory
            dirPath[slashIndex] = '/';

            currentIndex = (U32)(slashIndex + 1);
            slashIndex =
                firstOccurenceOf(fileLocation, '/', .from = currentIndex);
        }
    }
}

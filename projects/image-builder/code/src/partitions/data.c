
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "image-builder/configuration.h"
#include "posix/log.h"
#include "shared/assert.h"
#include "shared/log.h"
#include "shared/text/string.h"
#include "shared/types/numeric.h"

bool writeDataPartition(U8 *fileBuffer, int kernelfd, U32 kernelSizeBytes) {
    fileBuffer +=
        configuration.dataPartitionStartLBA * configuration.LBASizeBytes;

    for (U8 *exclusiveEnd = fileBuffer + kernelSizeBytes;
         fileBuffer < exclusiveEnd;) {
        I64 partialBytesRead =
            read(kernelfd, fileBuffer, (U32)(exclusiveEnd - fileBuffer));
        if (partialBytesRead < 0) {
            ASSERT(false);
            PFLUSH_AFTER(STDERR) {
                PERROR((STRING("Failed to read bytes from kernel file to "
                               "write to data partition!\n")));
                PERROR(STRING("Error code: "));
                PERROR(errno, NEWLINE);
                PERROR(STRING("Error message: "));
                char *errorString = strerror(errno);
                PERROR(STRING_LEN(errorString, (U32)strlen(errorString)),
                       NEWLINE);
            }
            return false;
        } else {
            fileBuffer += partialBytesRead;
        }
    }

    return true;
}

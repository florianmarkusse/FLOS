#include "posix/log.h"
#include "shared/log.h"

int main() {
    PFLUSH_AFTER(STDOUT) { INFO(STRING("hello trehrethr\n")); }
    PFLUSH_AFTER(STDOUT) { INFO(STRING("hello trehrethr\n")); }
    PFLUSH_AFTER(STDOUT) { INFO(STRING("hello trehrethr\n")); }
    PFLUSH_AFTER(STDOUT) { INFO(STRING("hello trehrethr\n")); }
    PFLUSH_AFTER(STDOUT) { INFO(STRING("hello trehrethr\n")); }
    PFLUSH_AFTER(STDOUT) { INFO(STRING("hello trehrethr\n")); }
    PFLUSH_AFTER(STDOUT) { INFO(STRING("hello trehrethr\n")); }
}

#ifndef POSIX_TEST_FRAMEWORK_EXPECTATIONS_H
#define POSIX_TEST_FRAMEWORK_EXPECTATIONS_H

#include "shared/text/string.h" // for string
#include "shared/types/numeric.h"

void appendExpectCodeWithString(U64 expected, String expectedString, U64 actual,
                                String actualString);
void appendExpectString(String expectedString, String actualString);
void appendExpectBool(bool expectedBool, bool actualBool);
void appendExpectPtrDiff(U64 expectedNumber, U64 actualNumber);
void appendExpectUint(U64 expectedNumber, U64 actualNumber);

#endif

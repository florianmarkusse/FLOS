#ifndef POSIX_TEST_FRAMEWORK_TEST_H
#define POSIX_TEST_FRAMEWORK_TEST_H

#include "abstraction/jmp.h"
#include "posix/log.h"
#include "shared/macros.h"      // for MACRO_VAR
#include "shared/text/string.h" // for string

void testSuiteStart(String mainTopic);
int testSuiteFinish();

void testTopicStart(String testTopic);
void testTopicFinish();

void unitTestStart(String testName, JumpBuffer failureHandler);

void testSuccess();

void testFailure();
void toCleanupHandler();
void appendTestFailureStart();
void appendTestFailureFinish();

#define TEST_FAILURE                                                           \
    for (auto MACRO_VAR(i) = (testFailure(), appendTestFailureStart(), 0);     \
         MACRO_VAR(i) < 1;                                                     \
         MACRO_VAR(i) = (appendTestFailureFinish(),                            \
                         PLOG(STRING("\n\n"), FLUSH), toCleanupHandler(), 1))

#define TEST(testString, failureHandler)                                       \
    for (auto MACRO_VAR(i) = (unitTestStart(testString, failureHandler), 0);   \
         MACRO_VAR(i) < 1; MACRO_VAR(i) = 1)

#define TEST_TOPIC(testTopicString)                                            \
    for (auto MACRO_VAR(i) = (testTopicStart(testTopicString), 0);             \
         MACRO_VAR(i) < 1; MACRO_VAR(i) = (testTopicFinish(), 1))

#endif

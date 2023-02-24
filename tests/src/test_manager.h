#pragma once

#include <defines.h>

#define FAILED 0
#define PASSED 1
#define BYPASS 2

typedef u8 (*PFN_test)();

void TestManagerInit();
void RegisterTest(PFN_test, char* desc);
void RunTests();
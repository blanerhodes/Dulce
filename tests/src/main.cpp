#include "test_manager.h"
#include "memory/linear_allocator_tests.h"

#include <core/logger.h>

int main(){
    TestManagerInit();

    LinearAllocatorRegisterTests();

    DDEBUG("Starting test...");

    RunTests();
    return 0;
}
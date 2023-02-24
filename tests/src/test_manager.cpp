#include "test_manager.h"

#include <containers/darray.h>
#include <core/logger.h>
#include <core/dstring.h>
#include <core/clock.h>

struct TestEntry{
    PFN_test func;
    char* desc;
};

static TestEntry* tests;

void TestManagerInit(){
    tests = (TestEntry*)DarrayCreate(TestEntry);
}

void RegisterTest(u8 (*PFN_test)(), char* desc){
    TestEntry e = {};
    e.func = PFN_test;
    e.desc = desc;
    DarrayPush(tests, e);
}

void RunTests(){
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = DarrayLength(tests);
    Clock totalTime = {};
    ClockStart(&totalTime);

    for(u32 i = 0; i < count; i++){
        Clock testTime = {};
        ClockStart(&testTime);
        u8 result = tests[i].func();
        ClockUpdate(&testTime);

        if(result == PASSED){
            passed++;
        } else if(result == BYPASS){
            DWARN("[SKIPPED]: %s", tests[i].desc);
            skipped++;
        } else{
            DERROR("[FAILED]: %s", tests[i].desc);
            failed++;
        }
        char status[20];
        StringFormat(status, failed ? (char*)"*** %d FAILED ***" : (char*)"SUCCESS", failed);
        ClockUpdate(&totalTime);
        DINFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total)", i+1, count, skipped, status, testTime.elapsed, totalTime.elapsed);
    }

    ClockStop(&totalTime);
    DINFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
}
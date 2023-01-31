#include <core/logger.h>
#include <core/asserts.h>
#include <platform/platform.h>

int main(void){
    DFATAL("A test message: %f", 3.14f);
    DERROR("A test message: %f", 3.14f);
    DWARN("A test message: %f", 3.14f);
    DINFO("A test message: %f", 3.14f);
    DDEBUG("A test message: %f", 3.14f);
    DTRACE("A test message: %f", 3.14f);

    PlatformState state = {};
    if(PlatformStartup(&state, "Dulce Engine Testbed", 100, 100, 1280, 720)){
        while(TRUE){
            PlatformPumpMessages(&state);
        }
    }
    PlatformShutdown(&state);

    return 0;
}
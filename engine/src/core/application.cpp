#include "application.h"
#include "logger.h"
#include "game_types.h"
#include "platform/platform.h"
#include "core/dmemory.h"
#include "core/event.h"
#include "core/input.h"
#include "core/clock.h"
#include "memory/linear_allocator.h"
#include "renderer/renderer_frontend.h"

struct ApplicationState{
    Game* gameInst;
    b8 isRunning;
    b8 isSuspended;
    PlatformState platform;
    i16 width;
    i16 height;
    Clock clock;
    f64 lastTime;
    LinearAllocator systemsAllocator;

    u64 memorySystemMemoryRequirement;
    void* memorySystemState;

    u64 loggingSystemMemoryRequirement;
    void* loggingSystemState;
};

static ApplicationState* appState;

//Event handlers
b8 ApplicationOnEvent(u16 code, void* sender, void* listenerInst, EventContext context);
b8 ApplicationOnKey(u16 code, void* sender, void* listenerInst, EventContext context);
b8 ApplicationOnResized(u16 code, void* sender, void* listenerInst, EventContext context);

b8 ApplicationCreate(Game* gameInst){
    if(gameInst->applicationState){
        DERROR("ApplicationCreate called more than once");
        return false;
    }

    gameInst->applicationState = (ApplicationState*)DAllocate(sizeof(ApplicationState), MEMORY_TAG_APPLICATION);
    appState = (ApplicationState*)gameInst->applicationState;
    appState->gameInst = gameInst;
    appState->isRunning = false;
    appState->isSuspended = false;

    u64 systemsAllocatorTotalSize = MegaBytes(64);
    AllocatorCreate(systemsAllocatorTotalSize, 0, &appState->systemsAllocator);

    InitializeMemory(&appState->memorySystemMemoryRequirement, 0);
    appState->memorySystemState = AllocatorAllocate(&appState->systemsAllocator, appState->memorySystemMemoryRequirement);
    InitializeMemory(&appState->memorySystemMemoryRequirement, appState->memorySystemState);

    //init subsystems
    InitializeLogging(&appState->loggingSystemMemoryRequirement, 0);
    appState->loggingSystemState = AllocatorAllocate(&appState->systemsAllocator, appState->loggingSystemMemoryRequirement);
    if(!InitializeLogging(&appState->loggingSystemMemoryRequirement, appState->loggingSystemState)){
        DERROR("Failed to initialize logging system. Shutting down.");
        return false;
    }

    InputInitialize();

    if(!EventInitialize()){
        DERROR("Event system not initialized!");
        return false;
    }

    EventRegister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventRegister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventRegister(EVENT_CODE_KEY_RELEASED, 0, ApplicationOnKey);
    EventRegister(EVENT_CODE_RESIZED, 0, ApplicationOnResized);

    if(!PlatformStartup(&appState->platform, gameInst->appConfig.name, 
                        gameInst->appConfig.startPosX, gameInst->appConfig.startPosY, 
                        gameInst->appConfig.startWidth, gameInst->appConfig.startHeight)){
        return false;
    }

    if(!RendererIntialize(gameInst->appConfig.name, &appState->platform)){
        DFATAL("Failed to initialize renderer. Aborting application.");
        return false;
    }

    if(!appState->gameInst->Initialize(appState->gameInst)){
        DFATAL("Game failed to initialize");
        return false;
    }
    
    appState->gameInst->OnResize(appState->gameInst, appState->width, appState->height);

    return true;
}

b8 ApplicationRun(){
    appState->isRunning = true;
    ClockStart(&appState->clock);
    ClockUpdate(&appState->clock);
    appState->lastTime = appState->clock.elapsed;
    f64 runningTime = 0;
    u8 frameCount = 0;
    f64 targetFrameSeconds = 1.0f / 60.0f;

    DINFO(GetMemoryUsageStr());
    while(appState->isRunning){
        if(!PlatformPumpMessages(&appState->platform)){
            appState->isRunning = false;
        }

        if(!appState->isSuspended){
            ClockUpdate(&appState->clock);
            f64 currentTime = appState->clock.elapsed;
            f64 delta = currentTime - appState->lastTime;
            f64 frameStartTime = PlatformGetAbsoluteTime();

            if(!appState->gameInst->Update(appState->gameInst, (f32)delta)){
                DFATAL("Game update failed, shutting down.");
                appState->isRunning = false;
                break;
            }
            if(!appState->gameInst->Render(appState->gameInst, (f32)delta)){
                DFATAL("Game render failed, shuttind down");
                appState->isRunning = false;
                break;
            }

            RenderPacket packet;
            packet.deltaTime = delta;
            RendererDrawFrame(&packet);

            f64 frameEndTime = PlatformGetAbsoluteTime();
            f64 frameElapsedTime = frameEndTime - frameStartTime;
            runningTime += frameElapsedTime;
            f64 remainingSeconds = targetFrameSeconds - frameElapsedTime;

            if(remainingSeconds > 0){
                u64 remainingMs = remainingSeconds * 1000;
                b8 limitFrames = false;
                if(remainingMs > 0 && limitFrames){
                    PlatformSleep(remainingMs - 1);
                }
                frameCount++;
            }

            //Input update/state copying should be handled after any input should be recorded (before this line)
            //As a safety input is the last thing to be updated before frame flip
            InputUpdate(delta);

            appState->lastTime = currentTime;
        }
    }

    appState->isRunning = false;
    EventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventUnregister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventUnregister(EVENT_CODE_KEY_RELEASED, 0, ApplicationOnKey);
    EventUnregister(EVENT_CODE_RESIZED, 0, ApplicationOnResized);
    EventShutdown();
    InputShutdown();
    RendererShutdown();
    PlatformShutdown(&appState->platform);
    ShutdownMemory(&appState->memorySystemState);
    return true;
}

void ApplicationGetFrameBufferSize(u32* width, u32* height){
    *width = appState->width;
    *height = appState->height;
}

b8 ApplicationOnEvent(u16 code, void* sender, void* listenerInst, EventContext context){
    switch(code){
        case EVENT_CODE_APPLICATION_QUIT: {
            DINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            appState->isRunning = false;
            return true;
        }
    }
    return false;
}

b8 ApplicationOnKey(u16 code, void* sender, void* listenerInst, EventContext context){
    u16 keyCode = context.data.u16[0];
    if(code == EVENT_CODE_KEY_PRESSED){
        switch(keyCode){
            case KEY_ESCAPE:{
                EventContext data = {};
                EventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);
                return true;
            }
            case KEY_A: {
                DDEBUG("Explicit - A key pressed");
            } break;
            default: {
                DDEBUG("'%c' key pressed in window.", keyCode);
            } break;
        }
    } else if(code == EVENT_CODE_KEY_RELEASED){
        switch(keyCode){
            case KEY_B: {
                DDEBUG("Explicit - B key released");
            } break;
            default: {
                DDEBUG("'%c' key released in window.", keyCode);
            } break;
        }
    }
    return false;
}

b8 ApplicationOnResized(u16 code, void* sender, void* listenerInst, EventContext context){
    if(code == EVENT_CODE_RESIZED){
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        if(width != appState->width || height != appState->height){
            appState->width = width;
            appState->height = height;
            DDEBUG("Window resize: %i, %i", width, height);

            if(width == 0 || height == 0){
                DINFO("Window minimized, suspending application.");
                appState->isSuspended = true;
                return true;
            } else{
                if(appState->isSuspended){
                    DINFO("window restored, resuming application.");
                    appState->isSuspended = false;
                }
                appState->gameInst->OnResize(appState->gameInst, width, height);
                RendererOnResized(width, height);
            }
        }
    }
    //Purposely not handled to allow other listneers to get this
    return false;
}
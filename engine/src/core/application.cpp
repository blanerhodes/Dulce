#include "application.h"
#include "logger.h"
#include "game_types.h"
#include "platform/platform.h"
#include "core/dmemory.h"
#include "core/event.h"
#include "core/input.h"

struct ApplicationState{
    Game* gameInst;
    b8 isRunning;
    b8 isSuspended;
    PlatformState platform;
    i16 width;
    i16 height;
    f64 lastTime;
};

static b8 initialized = FALSE;
static ApplicationState appState = {};

//Event handlers
b8 ApplicationOnEvent(u16 code, void* sender, void* listenerInst, EventContext context);
b8 ApplicationOnKey(u16 code, void* sender, void* listenerInst, EventContext context);

b8 ApplicationCreate(Game* gameInst){
    if(initialized){
        DERROR("ApplicationCreate called more than once");
        return FALSE;
    }

    appState.gameInst = gameInst;

    //init subsystems
    InitializeLogging();
    InputInitialize();

    //TODO: remove
    DFATAL("A test message: %f", 3.14f);
    DERROR("A test message: %f", 3.14f);
    DWARN("A test message: %f", 3.14f);
    DINFO("A test message: %f", 3.14f);
    DDEBUG("A test message: %f", 3.14f);
    DTRACE("A test message: %f", 3.14f);

    appState.isRunning = TRUE;
    appState.isSuspended = FALSE;

    if(!EventInitialize()){
        DERROR("Event system not initialized!");
        return FALSE;
    }

    EventRegister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventRegister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventRegister(EVENT_CODE_KEY_RELEASED, 0, ApplicationOnKey);

    if(!PlatformStartup(&appState.platform, gameInst->appConfig.name, 
                        gameInst->appConfig.startPosX, gameInst->appConfig.startPosY, 
                        gameInst->appConfig.startWidth, gameInst->appConfig.startHeight)){
        return FALSE;
    }

    if(!appState.gameInst->Initialize(appState.gameInst)){
        DFATAL("Game failed to initialize");
        return FALSE;
    }
    
    appState.gameInst->OnResize(appState.gameInst, appState.width, appState.height);

    initialized = TRUE;
    return TRUE;
}

b8 ApplicationRun(){
    DINFO(GetMemoryUsageStr());
    while(appState.isRunning){
        if(!PlatformPumpMessages(&appState.platform)){
            appState.isRunning = FALSE;
        }

        if(!appState.isSuspended){
            if(!appState.gameInst->Update(appState.gameInst, 0.0f)){
                DFATAL("Game update failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }
            if(!appState.gameInst->Render(appState.gameInst, (f32)0)){
                DFATAL("Game render failed, shuttind down");
                appState.isRunning = FALSE;
                break;
            }
            //Input update/state copying hsould be handled after any input should be recorded (before this line)
            //As a safety input is the last thing to be updated before frame flip
            InputUpdate(0);
        }
    }

    appState.isRunning = FALSE;
    EventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventUnregister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventUnregister(EVENT_CODE_KEY_RELEASED, 0, ApplicationOnKey);
    EventShutdown();
    InputShutdown();
    PlatformShutdown(&appState.platform);
    return TRUE;
}

b8 ApplicationOnEvent(u16 code, void* sender, void* listenerInst, EventContext context){
    switch(code){
        case EVENT_CODE_APPLICATION_QUIT: {
            DINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            appState.isRunning = FALSE;
            return TRUE;
        }
    }
    return FALSE;
}

b8 ApplicationOnKey(u16 code, void* sender, void* listenerInst, EventContext context){
    u16 keyCode = context.data.u16[0];
    if(code == EVENT_CODE_KEY_PRESSED){
        switch(keyCode){
            case KEY_ESCAPE:{
                EventContext data = {};
                EventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);
                return TRUE;
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
    return FALSE;
}
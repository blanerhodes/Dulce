#include "core/event.h"
#include "core/dmemory.h"
#include "containers/darray.h"

struct RegisteredEvent {
    void* listener;
    PfnOnEvent callback;
};

struct EventCodeEntry {
    RegisteredEvent* events;
};

#define MAX_MESSAGE_CODES 16384

struct EventSystemState {
    EventCodeEntry registered[MAX_MESSAGE_CODES];
};

static b8 isInitialized = FALSE;
static EventSystemState state = {};

b8 EventInitialize(){
    if(isInitialized){
        return FALSE;
    }
    DZeroMemory(&state, sizeof(state));
    isInitialized = TRUE;
    return TRUE;
}

void EventShutdown(){
    for(u16 i = 0; i < MAX_MESSAGE_CODES; i++){
        if(state.registered[i].events != 0){
            DarrayDestroy(state.registered[i].events);
            state.registered[i].events = 0;
        }
    }
}

b8 EventRegister(u16 code, void* listener, PfnOnEvent onEvent){
    if(!isInitialized){
        return FALSE;
    }

    //RegisteredEvent* events = state.registered[code].events;

    if(!state.registered[code].events){
        state.registered[code].events = (RegisteredEvent*)DarrayCreate(RegisteredEvent);
    }

    u64 registeredcount = DarrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredcount; i++){
        if(state.registered[code].events[i].listener == listener){
            //TODO: warn
            return FALSE;
        }
    }

    RegisteredEvent event = {};
    event.listener = listener;
    event.callback = onEvent;
    DarrayPush(state.registered[code].events, event);
    return TRUE;
}

b8 EventUnregister(u16 code, void* listener, PfnOnEvent onEvent){
    if(!isInitialized){
        return FALSE;
    }

    if(!state.registered[code].events){
        return FALSE;
    }

    u64 registeredCount = DarrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; i++){
        RegisteredEvent e = state.registered[code].events[i];
        //TODO: maybe make and equals function for events?
        if(e.listener == listener && e.callback == onEvent){
            RegisteredEvent poppedEvent = {};
            DarrayPopAt(state.registered[code].events, i, &poppedEvent);
            return TRUE;
        }
    }
    return FALSE;
}

b8 EventFire(u16 code, void* sender, EventContext context){
    if(!isInitialized){
        return FALSE;
    }

    if(!state.registered[code].events){
        return FALSE;
    }

    u64 registeredCount = DarrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; i++){
        RegisteredEvent e = state.registered[code].events[i];
        //TODO: maybe make and equals function for events?
        if(e.callback(code, sender, e.listener, context)){
            //Message handled, dont send ot other listeners
            return TRUE;
        }
    }
    return FALSE;
}
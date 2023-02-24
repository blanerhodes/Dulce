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

static b8 isInitialized = false;
static EventSystemState state = {};

b8 EventInitialize(){
    if(isInitialized){
        return false;
    }
    DZeroMemory(&state, sizeof(state));
    isInitialized = true;
    return true;
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
        return false;
    }

    //RegisteredEvent* events = state.registered[code].events;

    if(!state.registered[code].events){
        state.registered[code].events = (RegisteredEvent*)DarrayCreate(RegisteredEvent);
    }

    u64 registeredcount = DarrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredcount; i++){
        if(state.registered[code].events[i].listener == listener){
            //TODO: warn
            return false;
        }
    }

    RegisteredEvent event = {};
    event.listener = listener;
    event.callback = onEvent;
    DarrayPush(state.registered[code].events, event);
    return true;
}

b8 EventUnregister(u16 code, void* listener, PfnOnEvent onEvent){
    if(!isInitialized){
        return false;
    }

    if(!state.registered[code].events){
        return false;
    }

    u64 registeredCount = DarrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; i++){
        RegisteredEvent e = state.registered[code].events[i];
        //TODO: maybe make and equals function for events?
        if(e.listener == listener && e.callback == onEvent){
            RegisteredEvent poppedEvent = {};
            DarrayPopAt(state.registered[code].events, i, &poppedEvent);
            return true;
        }
    }
    return false;
}

b8 EventFire(u16 code, void* sender, EventContext context){
    if(!isInitialized){
        return false;
    }

    if(!state.registered[code].events){
        return false;
    }

    u64 registeredCount = DarrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; i++){
        RegisteredEvent e = state.registered[code].events[i];
        //TODO: maybe make and equals function for events?
        if(e.callback(code, sender, e.listener, context)){
            //Message handled, dont send ot other listeners
            return true;
        }
    }
    return false;
}
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

static EventSystemState* event_state_ptr;

void EventSystemInitialize(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(EventSystemState);
    if(state == 0){
        return;
    }
    DZeroMemory(state, sizeof(EventSystemState));
    event_state_ptr = (EventSystemState*)state;
}

void EventSystemShutdown(void* state) {
    if (event_state_ptr) {
        for (u16 i = 0; i < MAX_MESSAGE_CODES; i++) {
            if (event_state_ptr->registered[i].events != 0) {
                DarrayDestroy(event_state_ptr->registered[i].events);
                event_state_ptr->registered[i].events = 0;
            }
        }
    }
}

b8 EventRegister(u16 code, void* listener, PfnOnEvent on_event) {
    if (!event_state_ptr) {
        return false;
    }

    //RegisteredEvent* events = state.registered[code].events;

    if (!event_state_ptr->registered[code].events) {
        event_state_ptr->registered[code].events = (RegisteredEvent*)DarrayCreate(RegisteredEvent);
    }

    u64 registered_count = DarrayLength(event_state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; i++) {
        if (event_state_ptr->registered[code].events[i].listener == listener) {
            //TODO: warn
            return false;
        }
    }

    RegisteredEvent event = {};
    event.listener = listener;
    event.callback = on_event;
    DarrayPush(event_state_ptr->registered[code].events, event);
    return true;
}

b8 EventUnregister(u16 code, void* listener, PfnOnEvent on_event) {
    if (!event_state_ptr) {
        return false;
    }

    if (!event_state_ptr->registered[code].events) {
        return false;
    }

    u64 registered_count = DarrayLength(event_state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; i++) {
        RegisteredEvent e = event_state_ptr->registered[code].events[i];
        //TODO: maybe make and equals function for events?
        if (e.listener == listener && e.callback == on_event) {
            RegisteredEvent poppedEvent = {};
            DarrayPopAt(event_state_ptr->registered[code].events, i, &poppedEvent);
            return true;
        }
    }
    return false;
}

b8 EventFire(u16 code, void* sender, EventContext context) {
    if (!event_state_ptr) {
        return false;
    }

    if (!event_state_ptr->registered[code].events) {
        return false;
    }

    u64 registered_count = DarrayLength(event_state_ptr->registered[code].events);
    for (u64 i = 0; i < registered_count; i++) {
        RegisteredEvent e = event_state_ptr->registered[code].events[i];
        //TODO: maybe make and equals function for events?
        if (e.callback(code, sender, e.listener, context)) {
            //Message handled, dont send ot other listeners
            return true;
        }
    }
    return false;
}
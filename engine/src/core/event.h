#pragma once

#include "defines.h"

//offhand idea, have arrays for each input (an array for W, A, S, D etc) and loop the array to trigger events

struct EventContext{
    //128 bytes
    union {
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        char c[16];
    } data;
};

typedef b8 (*PfnOnEvent)(u16 code, void* sender, void* listenerInst, EventContext data);

b8 EventInitialize();
void EventShutdown();

/*
Register to listen for when events are sent twitht he provided code. Events with duplicate listener/callback combos
won't be registered and will return false.
code = event code to listen for
listener = pointer to listener instance
onEvent = callback function pointer to invoke when event code is fired
*/
DAPI b8 EventRegister(u16 code, void* listener, PfnOnEvent onEvent);

/*
Unregister from listening for when event sare sent witht he provided code. If no matching registration is found,
return false
*/
DAPI b8 EventUnregister(u16 code, void* listener, PfnOnEvent onEvent);

DAPI b8 EventFire(u16 code, void* sender, EventContext context);

//System internal codes. App should use codes beyond 255
enum SystemEventCode{
    EVENT_CODE_APPLICATION_QUIT = 0x01,
    EVENT_CODE_KEY_PRESSED = 0x02,
    EVENT_CODE_KEY_RELEASED = 0x03,
    EVENT_CODE_BUTTON_PRESSED = 0x04,
    EVENT_CODE_BUTTON_RELEASED = 0x05,
    EVENT_CODE_MOUSE_MOVED = 0x06,
    EVENT_CODE_MOUSE_WHEEL = 0x07,
    EVENT_CODE_RESIZED = 0x08,
    MAX_EVENT_CODE = 0xFF
};
#include "core/input.h"
#include "core/event.h"
#include "core/dmemory.h"
#include "core/logger.h"

struct KeyboardState{
    b8 keys[256];
};

struct MouseState{
    i16 x;
    i16 y;
    u8 buttons[BUTTON_MAX_BUTTONS];
};

struct InputState{
    KeyboardState keyboardCurrent;
    KeyboardState keyboardPrevious;
    MouseState mouseCurrent;
    MouseState mousePrevious;
};

static b8 initialized = false;
static InputState state = {};

void InputInitialize(){
    DZeroMemory(&state, sizeof(InputState));
    initialized = true;
    DINFO("Input subsytem initialized");
}

void InputShutdown(){
    initialized = false;
}

void InputUpdate(f64 deltaTime){
    if(!initialized){
        return;
    }
    //TODO: maybe change the states to pointers and just swap pointers back and forth instead of copying
    DCopyMemory(&state.keyboardPrevious, &state.keyboardCurrent, sizeof(KeyboardState));
    DCopyMemory(&state.mousePrevious, &state.mouseCurrent, sizeof(MouseState));
}

void InputProcessKey(Keys key, b8 pressed){
    if(key == KEY_LALT){
        DINFO("Left alt pressed.");
    } else if(key == KEY_RALT){
        DINFO("Right alt pressed.");
    } else if(key == KEY_LCONTROL){
        DINFO("Left control pressed.");
    } else if(key == KEY_RCONTROL){
        DINFO("Right control pressed.");
    } else if(key == KEY_LSHIFT){
        DINFO("Left shift pressed.");
    } else if(key == KEY_RSHIFT){
        DINFO("Right shift pressed.");
    }

    if(state.keyboardCurrent.keys[key] != pressed){
        state.keyboardCurrent.keys[key] = pressed;
        EventContext context = {};
        context.data.u16[0] = key;
        EventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void InputProcessButton(Buttons button, b8 pressed){
    if(state.mouseCurrent.buttons[button] != pressed){
        state.mouseCurrent.buttons[button] = pressed;
        EventContext context = {};
        context.data.u16[0] = button;
        EventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void InputProcessMouseMove(i16 x, i16 y){
    if(state.mouseCurrent.x != x || state.mouseCurrent.y != y){
        //DDEBUG("Mouse pos: %i, %i", x, y);
        state.mouseCurrent.x = x;
        state.mouseCurrent.y = y;

        EventContext context = {};
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        EventFire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void InputProcessMouseWheel(i8 zDelta){
    EventContext context = {};
    context.data.u8[0] = zDelta;
    EventFire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 InputIsKeyDown(Keys key){
    if(!initialized){
        return false;
    }
    return state.keyboardCurrent.keys[key] == true;
}

b8 InputIsKeyUp(Keys key){
    if(!initialized){
        return true;
    }
    return state.keyboardCurrent.keys[key] == false;
}

b8 InputWasKeyDown(Keys key){
    if(!initialized){
        return false;
    }
    return state.keyboardPrevious.keys[key] == true;
}

b8 InputWasKeyUp(Keys key){
    if(!initialized){
        return true;
    }
    return state.keyboardPrevious.keys[key] == false;
}

b8 InputIsButtonDown(Buttons button){
    if(!initialized){
        return false;
    }
    return state.mouseCurrent.buttons[button] == true;
}

b8 InputIsButtonUp(Buttons button){
    if(!initialized){
        return true;
    }
    return state.mouseCurrent.buttons[button] == false;
}

b8 InputWasButtonDown(Buttons button){
    if(!initialized){
        return false;
    }
    return state.mousePrevious.buttons[button] == true;
}

b8 InputWasButtonUp(Buttons button){
    if(!initialized){
        return true;
    }
    return state.mousePrevious.buttons[button] == false;
}

void InputGetMousePosition(i32* x, i32* y){
    if(!initialized){
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouseCurrent.x;
    *y = state.mouseCurrent.y;
}

void InputGetPreviousMousePosition(i32* x, i32* y){
    if(!initialized){
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mousePrevious.x;
    *y = state.mousePrevious.y;
}

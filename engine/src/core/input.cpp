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

struct InputSystemState {
    KeyboardState keyboard_current;
    KeyboardState keyboard_previous;
    MouseState mouse_current;
    MouseState mouse_previous;
};

static InputSystemState* input_state_ptr;

void InputSystemInitialize(u64* memory_requirment, void* state) {
    *memory_requirment = sizeof(InputSystemState);
    if (state == 0) {
        return;
    }

    DZeroMemory(state, sizeof(InputSystemState));
    input_state_ptr = (InputSystemState*)state;
    DINFO("Input subsytem initialized");
}

void InputSystemShutdown(void* state) {
    input_state_ptr = 0;
}

void InputUpdate(f64 delta_time) {
    if (!input_state_ptr) {
        return;
    }
    //TODO: maybe change the states to pointers and just swap pointers back and forth instead of copying
    DCopyMemory(&input_state_ptr->keyboard_previous, &input_state_ptr->keyboard_current, sizeof(KeyboardState));
    DCopyMemory(&input_state_ptr->mouse_previous, &input_state_ptr->mouse_current, sizeof(MouseState));
}

void InputProcessKey(Keys key, b8 pressed) {
    if (input_state_ptr && input_state_ptr->keyboard_current.keys[key] != pressed) {
        input_state_ptr->keyboard_current.keys[key] = pressed;

        if (key == KEY_LALT) {
            DINFO("Left alt %s.", pressed ? "pressed": "released");
        } else if (key == KEY_RALT) {
            DINFO("Right alt %s.", pressed ? "pressed": "released");
        } 
        if (key == KEY_LCONTROL) {
            DINFO("Left control %s.", pressed ? "pressed": "released");
        } else if (key == KEY_RCONTROL) {
            DINFO("Right control %s.", pressed ? "pressed": "released");
        }
        if (key == KEY_LSHIFT) {
            DINFO("Left shift %s.", pressed ? "pressed": "released");
        } else if (key == KEY_RSHIFT) {
            DINFO("Right shift %s.", pressed ? "pressed": "released");
        }

        EventContext context = {};
        context.data.u16[0] = key;
        EventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void InputProcessButton(Buttons button, b8 pressed) {
    if (input_state_ptr->mouse_current.buttons[button] != pressed) {
        input_state_ptr->mouse_current.buttons[button] = pressed;
        EventContext context = {};
        context.data.u16[0] = button;
        EventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void InputProcessMouseMove(i16 x, i16 y) {
    if (input_state_ptr->mouse_current.x != x || input_state_ptr->mouse_current.y != y) {
        //DDEBUG("Mouse pos: %i, %i", x, y);
        input_state_ptr->mouse_current.x = x;
        input_state_ptr->mouse_current.y = y;

        EventContext context = {};
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        EventFire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void InputProcessMouseWheel(i8 zDelta) {
    EventContext context = {};
    context.data.u8[0] = zDelta;
    EventFire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 InputIsKeyDown(Keys key) {
    if (!input_state_ptr) {
        return false;
    }
    return input_state_ptr->keyboard_current.keys[key] == true;
}

b8 InputIsKeyUp(Keys key) {
    if (!input_state_ptr) {
        return true;
    }
    return input_state_ptr->keyboard_current.keys[key] == false;
}

b8 InputWasKeyDown(Keys key) {
    if (!input_state_ptr) {
        return false;
    }
    return input_state_ptr->keyboard_previous.keys[key] == true;
}

b8 InputWasKeyUp(Keys key) {
    if (!input_state_ptr) {
        return true;
    }
    return input_state_ptr->keyboard_previous.keys[key] == false;
}

b8 InputIsButtonDown(Buttons button) {
    if (!input_state_ptr) {
        return false;
    }
    return input_state_ptr->mouse_current.buttons[button] == true;
}

b8 InputIsButtonUp(Buttons button) {
    if (!input_state_ptr) {
        return true;
    }
    return input_state_ptr->mouse_current.buttons[button] == false;
}

b8 InputWasButtonDown(Buttons button) {
    if (!input_state_ptr) {
        return false;
    }
    return input_state_ptr->mouse_previous.buttons[button] == true;
}

b8 InputWasButtonUp(Buttons button) {
    if (!input_state_ptr) {
        return true;
    }
    return input_state_ptr->mouse_previous.buttons[button] == false;
}

void InputGetMousePosition(i32* x, i32* y) {
    if (!input_state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = input_state_ptr->mouse_current.x;
    *y = input_state_ptr->mouse_current.y;
}

void InputGetPreviousMousePosition(i32* x, i32* y) {
    if (!input_state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = input_state_ptr->mouse_previous.x;
    *y = input_state_ptr->mouse_previous.y;
}

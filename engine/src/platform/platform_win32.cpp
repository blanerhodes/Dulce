#include "platform/platform.h"

#if DPLATFORM_WINDOWS
#include <core/logger.h>
#include "core/input.h"
#include "core/event.h"
#include "containers/darray.h"
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "renderer/vulkan/vulkan_types.inl"

struct PlatformState{
    HINSTANCE hInstance;
    HWND hwnd;
    VkSurfaceKHR surface;
};

static f64 clock_frequency;
static LARGE_INTEGER start_time;

static PlatformState* platform_state_ptr;

void ClockSetup() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0f / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);
}

LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

b8 PlatformSystemStartup(u64* memory_requirement, void* state, char* appName, i32 x, i32 y, i32 width, i32 height){
    *memory_requirement = sizeof(PlatformState);
    if (state == 0) {
        return true;
    }
    platform_state_ptr = (PlatformState*)state;
    platform_state_ptr->hInstance = GetModuleHandleA(0);

    HICON icon = LoadIcon(platform_state_ptr->hInstance, IDI_APPLICATION);
    WNDCLASSA wc = {};
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = Win32ProcessMessage;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = platform_state_ptr->hInstance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); //manage cursor manually
    wc.hbrBackground = NULL; //transparent? TODO: look up what this field is
    wc.lpszClassName = "DulceWindowClass";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    //Create Window
    u32 client_x = x;
    u32 client_y = y;
    u32 client_width = width;
    u32 client_height = height;
    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;  
    u32 window_ex_style = WS_EX_APPWINDOW;
    window_style |= WS_MAXIMIZEBOX;
    window_style |= WS_MINIMIZEBOX;
    window_style |= WS_THICKFRAME;

    //get size of border
    RECT border_rect = {};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    //expands outer rect around client size
    window_x += border_rect.left;
    window_y += border_rect.top;
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(
        window_ex_style, "DulceWindowClass", appName, 
        window_style, window_x, window_y, 
        window_width, window_height, 0, 0, platform_state_ptr->hInstance, 0);

    if (!handle) {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        DFATAL("Window creation failed!");
        return false; 
    } else {
        platform_state_ptr->hwnd = handle;
    }

    b32 should_activate = 1; //TODO: if window should not accept input, this should be false
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    //If start minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVATE
    //If start maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
    ShowWindow(platform_state_ptr->hwnd, show_window_command_flags);

    ClockSetup();
    return true;
}

void PlatformSystemShutdown(void* state) {
    if (platform_state_ptr && platform_state_ptr->hwnd) {
        DestroyWindow(platform_state_ptr->hwnd);
        platform_state_ptr->hwnd = 0;
    }
}

b8 PlatformPumpMessages() {
    if (platform_state_ptr) {
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
    return true;
}

void* PlatformAllocate(u64 size, b8 aligned) {
    return malloc(size);
}

void PlatformFree(void* block, b8 aligned) {
    free(block);
}

void* PlatformZeroMemory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* PlatformCopyMemory(void* dest, void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* PlatformSetMemory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

void PlatformConsoleWrite(char* message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    //fatal, error, info, debug, trace
    u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);

    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD bytes_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, bytes_written, 0);
}

void PlatformConsoleWriteError(char* message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    //fatal, error, info, debug, trace
    u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);

    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD numberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, numberWritten, 0);
}

f64 PlatformGetAbsoluteTime() {
    if (!clock_frequency) {
        ClockSetup();
    }
    LARGE_INTEGER curr_time;
    QueryPerformanceCounter(&curr_time);
    return (f64)curr_time.QuadPart * clock_frequency;
}

void PlatformSleep(u64 ms) {
    Sleep(ms);
}

void PlatformGetRequiredExtensionNames(char*** names_darray) {
    DarrayPush(*names_darray, (char*)"VK_KHR_win32_surface");
}

b8 PlatformCreateVulkanSurface(VulkanContext* context) {
    if (!platform_state_ptr) {
        return false;
    }
    VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    create_info.hinstance = platform_state_ptr->hInstance;
    create_info.hwnd = platform_state_ptr->hwnd;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator, &platform_state_ptr->surface);
    if (result != VK_SUCCESS) {
        DFATAL("Vulkan surface creation failed.");
        return false;
    }

    context->surface = platform_state_ptr->surface;
    return true;
}

Keys SplitCodeLeftRight(b8 pressed, i32 in_left, i32 in_right, Keys out_left, Keys out_right) {
    Keys key = {};
    if (pressed) {
        if (GetKeyState(in_right) & 0x8000) {
            key = out_right;
        } else if(GetKeyState(in_left) & 0x8000) {
            key = out_left;
        }
    } else {
        if (!(GetKeyState(in_right) & 0x8000)) {
            key = out_right;
        } else if (!(GetKeyState(in_left) & 0x8000)) {
            key = out_left;
        }
    }
    return key;
}

LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param){
    switch(msg) {
        case WM_ERASEBKGND:
            //notify the OS that erasing will be handled by the app to prevent flicker (like when rendering with vulkan)
            return 1;
        case WM_CLOSE:{
            EventContext data = {};
            EventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);
        } return 0;
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_SIZE: {
            RECT r = {};
            GetClientRect(hwnd, &r);
            u32 width = r.right - r.left;
            u32 height = r.bottom - r.top;

            EventContext context = {};
            context.data.u16[0] = (u16)width;
            context.data.u16[1] = (u16)height;
            EventFire(EVENT_CODE_RESIZED, 0, context);
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:{
            //key pressed/released
            b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            Keys key = (Keys)(u16)w_param;
            b8 is_extended = (HIWORD(l_param) & KF_EXTENDED) == KF_EXTENDED;

            if (w_param == VK_MENU) {
                key = is_extended ? KEY_RALT : KEY_LALT;
            } else if (w_param == VK_SHIFT) {
                u32 left_shift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
                u32 scan_code = ((l_param & (0xFF << 16)) >> 16);
                key = scan_code == left_shift ? KEY_LSHIFT : KEY_RSHIFT;
            } else if (w_param == VK_CONTROL) {
                key = is_extended ? KEY_RCONTROL : KEY_LCONTROL;
            }

            InputProcessKey(key, pressed);
        } return 0;
        case WM_MOUSEMOVE:{
            i32 xPos = GET_X_LPARAM(l_param);
            i32 yPos = GET_Y_LPARAM(l_param);
            InputProcessMouseMove(xPos, yPos);
        } break;
        case WM_MOUSEWHEEL: {
            i32 zDelta = GET_WHEEL_DELTA_WPARAM(w_param);
            if (zDelta != 0) {
                //flatten to OS independent range
                zDelta = (zDelta < 0) ? -1 : 1;
                InputProcessMouseWheel(zDelta);
            }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:{
            b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            Buttons mouse_button = BUTTON_MAX_BUTTONS;
            switch(msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouse_button = BUTTON_LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouse_button = BUTTON_MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouse_button = BUTTON_RIGHT;
                    break;
            }
            if (mouse_button != BUTTON_MAX_BUTTONS) {
                InputProcessButton(mouse_button, pressed);
            }
        } break;
    }
    return DefWindowProcA(hwnd, msg, w_param, l_param);
}
#endif
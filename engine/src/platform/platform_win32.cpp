#include "platform.h"
#include "core/input.h"

#if DPLATFORM_WINDOWS
    #include <windows.h>
    #include <windowsx.h>
    #include <stdlib.h>
    #include <core/logger.h>

    struct InternalState{
        HINSTANCE hInstance;
        HWND hwnd;
    };

    static f64 clockFrequency;
    static LARGE_INTEGER startTime;

    LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam);

    b8 PlatformStartup(PlatformState* platState, char* appName, i32 x, i32 y, i32 width, i32 height){
        platState->internalState = malloc(sizeof(InternalState));

        InternalState* state = (InternalState*)platState->internalState;
        state->hInstance = GetModuleHandle(0);

        HICON icon = LoadIcon(state->hInstance, IDI_APPLICATION);
        WNDCLASSA wc = {};
        wc.style = CS_DBLCLKS;
        wc.lpfnWndProc = Win32ProcessMessage;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = state->hInstance;
        wc.hIcon = icon;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW); //manage cursor manually
        wc.hbrBackground = NULL; //transparent? TODO: look up what this field is
        wc.lpszClassName = "DulceWindowClass";

        if(!RegisterClassA(&wc)){
            MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
            return false;
        }

        //Create Window
        u32 clientX = x;
        u32 clientY = y;
        u32 clientWidth = width;
        u32 clientHeight = height;
        u32 windowX = clientX;
        u32 windowY = clientY;
        u32 windowWidth = clientWidth;
        u32 windowHeight = clientHeight;

        u32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;  
        u32 windowExStyle = WS_EX_APPWINDOW;
        windowStyle |= WS_MAXIMIZEBOX;
        windowStyle |= WS_MINIMIZEBOX;
        windowStyle |= WS_THICKFRAME;

        //get size of border
        RECT borderRect = {};
        AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);

        //expands outer rect around client size
        windowX += borderRect.left;
        windowY += borderRect.top;
        windowWidth += borderRect.right - borderRect.left;
        windowHeight += borderRect.bottom - borderRect.top;

        HWND handle = CreateWindowExA(windowExStyle, "DulceWindowClass", appName, windowStyle, windowX, windowY, windowWidth, windowHeight, 0, 0, state->hInstance, 0);

        if(!handle){
            MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            DFATAL("Window creation failed!");
            return FALSE; 
        } else{
            state->hwnd = handle;
        }

        b32 shouldActivate = 1; //TODO: if window should not accept input, this should be false
        i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
        //If start minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVATE
        //If start maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
        ShowWindow(state->hwnd, showWindowCommandFlags);

        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        clockFrequency = 1.0f / (f64)frequency.QuadPart;
        QueryPerformanceCounter(&startTime);

        return TRUE;
    }

    void PlatformShutdown(PlatformState* platState){
        InternalState* state = (InternalState*)platState->internalState;
        if(state->hwnd){
            DestroyWindow(state->hwnd);
            state->hwnd = 0;
        }
    }

    b8 PlatformPumpMessages(PlatformState* platState){
        MSG message;
        while(PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        return TRUE;
    }

    void* PlatformAllocate(u64 size, b8 aligned){
        return malloc(size);
    }

    void PlatformFree(void* block, b8 aligned){
        free(block);
    }

    void* PlatformZeroMemory(void* block, u64 size){
        return memset(block, 0, size);
    }
    
    void* PlatformCopyMemory(void* dest, void* source, u64 size){
        return memcpy(dest, source, size);
    }

    void* PlatformSetMemory(void* dest, i32 value, u64 size){
        return memset(dest, value, size);
    }

    void PlatformConsoleWrite(char* message, u8 color){
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        //fatal, error, info, debug, trace
        u8 levels[6] = {64, 4, 6, 2, 1, 8};
        SetConsoleTextAttribute(consoleHandle, levels[color]);

        OutputDebugStringA(message);
        u64 length = strlen(message);
        LPDWORD bytesWritten = 0;
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, bytesWritten, 0);
    }

    void PlatformConsoleWriteError(char* message, u8 color){
        HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
        //fatal, error, info, debug, trace
        u8 levels[6] = {64, 4, 6, 2, 1, 8};
        SetConsoleTextAttribute(consoleHandle, levels[color]);

        OutputDebugStringA(message);
        u64 length = strlen(message);
        LPDWORD numberWritten = 0;
        WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, numberWritten, 0);
    }

    f64 PlatformGetAbsoluteTime(){
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        return (f64)currentTime.QuadPart * clockFrequency;
    }

    void PlatformSleep(u64 ms){
        Sleep(ms);
    }

    LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam){
        switch(msg){
            case WM_ERASEBKGND:
                //notify the OS that erasing will be handled by the app to prevent flicker (like when rendering with vulkan)
                return 1;
            case WM_CLOSE:
                //TODO: fire event for the app to quit
                return 0;
            case WM_DESTROY: {
                PostQuitMessage(0);
                return 0;
            }
            case WM_SIZE: {
                //RECT r = {};
                //GetClientRect(hwnd, &r);
                //u32 width = r.right - r.left;
                //u32 height = r.bottom - r.top;
                //TODO: fire event for window resize
            } break;
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:{
                //key pressed/released
                b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
                Keys key = (Keys)(u16)wParam;
                InputProcessKey(key, pressed);
            } break;
            case WM_MOUSEMOVE:{
                i32 xPos = GET_X_LPARAM(lParam);
                i32 yPos = GET_Y_LPARAM(lParam);
                InputProcessMouseMove(xPos, yPos);
            } break;
            case WM_MOUSEWHEEL: {
                i32 zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                if(zDelta != 0){
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
                Buttons mouseButton = BUTTON_MAX_BUTTONS;
                switch(msg){
                    case WM_LBUTTONDOWN:
                    case WM_LBUTTONUP:
                        mouseButton = BUTTON_LEFT;
                        break;
                    case WM_MBUTTONDOWN:
                    case WM_MBUTTONUP:
                        mouseButton = BUTTON_MIDDLE;
                        break;
                    case WM_RBUTTONDOWN:
                    case WM_RBUTTONUP:
                        mouseButton = BUTTON_RIGHT;
                        break;
                }
                if(mouseButton != BUTTON_MAX_BUTTONS){
                    InputProcessButton(mouseButton, pressed);
                }
            } break;
        }
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }

#endif
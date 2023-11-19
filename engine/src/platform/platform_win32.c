#include "platform.h"

// Windows platform layer.
#if KPLATFORM_WINDOWS

#include "common.h"
#include "obelisk_memory.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>  // param input extraction
#include <stdlib.h>

#define OBELISK_WNDCLASSNAME "OBELISKWNDCLASS"

// Clock
static u64 _obelisk_timer_frequency = 0;
static u64 _obelisk_timer_offset = 0;

static LRESULT CALLBACK obeliskWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

b8 obeliskPlatformInit()
{
    // register window class
    WNDCLASSA wnd_class = {
        .hInstance      = GetModuleHandleA(NULL),
        .lpfnWndProc    = obeliskWindowProc,
        .style          = CS_DBLCLKS,  // Get double-clicks
        .lpszClassName  = OBELISK_WNDCLASSNAME,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = NULL, // Transparent
        .cbClsExtra     = 0,
        .cbWndExtra     = 0
    };

    if (!RegisterClassA(&wnd_class))
    {
        KERROR("[Platform] Failed to register WindowClass");
        return FALSE;
    }

    // init time
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&_obelisk_timer_frequency))
    {
        KERROR("[Platform] High-resolution performance counter is not supported");
        return FALSE;
    }

    QueryPerformanceCounter((LARGE_INTEGER*)&_obelisk_timer_offset);

    return TRUE;
}

b8 obeliskPlatformTerminate()
{
    // unregister window class
    if (!UnregisterClassA(OBELISK_WNDCLASSNAME, GetModuleHandleA(NULL)))
    {
        KERROR("[Platform] Failed to unregister WindowClass");
        return FALSE;
    }
    return TRUE;
}

struct ObeliskWindow
{
    HINSTANCE   instance;
    HWND        handle;

    b8 should_close;
};

ObeliskWindow* obeliskCreateWindow(const char* title, i32 x, i32 y, u32 w, u32 h)
{
    ObeliskWindow* window = obeliskAlloc(sizeof(ObeliskWindow), OBELISK_MEMTAG_UNTRACED);
    if (!window) return NULL;

    // Create window
    window->instance = GetModuleHandleA(NULL);

    RECT rect = { 0 };
    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    DWORD style_ex = WS_EX_APPWINDOW;
    AdjustWindowRectEx(&rect, style, 0, style_ex);

    x += rect.left;
    y += rect.top;
    w += rect.right - rect.left;
    h += rect.bottom - rect.top;

    window->handle = CreateWindowExA(style_ex, OBELISK_WNDCLASSNAME, NULL, style, x, y, w, h, 0, 0, window->instance, 0);
    if (!window->handle)
    {
        KERROR("[Platform] Failed to create window");
        obeliskDestroyWindow(window);
        return NULL;
    }

    obeliskSetWindowTitle(window, title);

    return window;
}

void obeliskDestroyWindow(ObeliskWindow* window)
{
    // destroy window
    if (window->handle && !DestroyWindow(window->handle))
    {
        KERROR("[Platform] Failed to destroy window");
    }

    obeliskFree(window, sizeof(ObeliskWindow), OBELISK_MEMTAG_UNTRACED);
}

void obeliskSetWindowTitle(ObeliskWindow* window, const char* title)
{
    SetWindowTextA(window->handle, title);
}

b8   obeliskShouldClose(const ObeliskWindow* context) { return context->should_close; }
void obeliskCloseWindow(ObeliskWindow* context)       { context->should_close = TRUE; }

void obeliskPollWindowEvents(ObeliskWindow* context)
{
    MSG msg;
    while (PeekMessageW(&msg, context->handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

f64 obeliskGetTime()
{
    u64 value;
    QueryPerformanceCounter((LARGE_INTEGER*)&value);
    return (f64)(value - _obelisk_timer_offset) / _obelisk_timer_frequency;
}

LRESULT CALLBACK obeliskWindowProc(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg)
    {
        case WM_ERASEBKGND:
            // Notify the OS that erasing will be handled by the application to prevent flicker.
            return 1;
        case WM_CLOSE:
            // TODO: Fire an event for the application to quit.
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            // Get the updated size.
            // RECT r;
            // GetClientRect(hwnd, &r);
            // u32 width = r.right - r.left;
            // u32 height = r.bottom - r.top;

            // TODO: Fire an event for window resize.
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // Key pressed/released
            //b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            // TODO: input processing

        } break;
        case WM_MOUSEMOVE: {
            // Mouse move
            //i32 x_position = GET_X_LPARAM(l_param);
            //i32 y_position = GET_Y_LPARAM(l_param);
            // TODO: input processing.
        } break;
        case WM_MOUSEWHEEL: {
            // i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            // if (z_delta != 0) {
            //     // Flatten the input to an OS-independent (-1, 1)
            //     z_delta = (z_delta < 0) ? -1 : 1;
            //     // TODO: input processing.
            // }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            //b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            // TODO: input processing.
        } break;
    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}


void platform_console_write(const char *message, u8 colour) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[colour]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void platform_console_write_error(const char *message, u8 colour) {
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[colour]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

void platform_sleep(u64 ms) {
    Sleep(ms);
}

#endif // KPLATFORM_WINDOWS
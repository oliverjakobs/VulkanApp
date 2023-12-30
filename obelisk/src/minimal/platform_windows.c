#include "platform.h"

#ifdef MINIMAL_PLATFORM_WINDOWS

#include "common.h"
#include "minimal.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <stdlib.h>

#define MINIMAL_WNDCLASSNAME "MINIMALWNDCLASS"

#define MINIMAL_GET_X_LPARAM(lp)    ((i32)(i16)LOWORD(lp))
#define MINIMAL_GET_Y_LPARAM(lp)    ((i32)(i16)HIWORD(lp))

#define MINIMAL_GET_SCROLL(wp)      ((i32)((i16)HIWORD(wp) / (f32)WHEEL_DELTA))

//clock
static u64 _minimal_timer_frequency = 0;
static u64 _minimal_timer_offset = 0;

static LRESULT CALLBACK minimalWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

u8 minimalPlatformInit()
{
    // register window class
    WNDCLASSA wnd_class = {
        .hInstance      = GetModuleHandleA(NULL),
        .lpfnWndProc    = minimalWindowProc,
        .style          = CS_DBLCLKS,  // Get double-clicks
        .lpszClassName  = MINIMAL_WNDCLASSNAME,
        .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = NULL, // Transparent
        .cbClsExtra     = 0,
        .cbWndExtra     = 0
    };

    if (!RegisterClassA(&wnd_class))
    {
        MINIMAL_ERROR("[Platform] Failed to register WindowClass");
        return FALSE;
    }

    // init time
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&_minimal_timer_frequency))
    {
        MINIMAL_ERROR("[Platform] High-resolution performance counter is not supported");
        return MINIMAL_FAIL;
    }

    QueryPerformanceCounter((LARGE_INTEGER*)&_minimal_timer_offset);

    return MINIMAL_OK;
}

u8 minimalPlatformTerminate()
{
    // unregister window class
    if (!UnregisterClassA(MINIMAL_WNDCLASSNAME, GetModuleHandleA(NULL)))
    {
        MINIMAL_ERROR("[Platform] Failed to unregister WindowClass");
        return MINIMAL_FAIL;
    }
    return MINIMAL_OK;
}

struct MinimalWindow
{
    HINSTANCE   instance;
    HWND        handle;

    u8 should_close;
};

MinimalWindow* minimalCreateWindow(const char* title, i32 x, i32 y, u32 w, u32 h)
{
    MinimalWindow* window = MINIMAL_ALLOC(sizeof(MinimalWindow));
    if (!window) return NULL;

    // Create window
    window->instance = GetModuleHandleA(NULL);
    window->should_close = 0;

    RECT rect = { 0 };
    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    DWORD style_ex = WS_EX_APPWINDOW;
    AdjustWindowRectEx(&rect, style, 0, style_ex);

    x += rect.left;
    y += rect.top;
    w += rect.right - rect.left;
    h += rect.bottom - rect.top;

    window->handle = CreateWindowExA(style_ex, MINIMAL_WNDCLASSNAME, title, style, x, y, w, h, 0, 0, window->instance, 0);
    if (!window->handle)
    {
        MINIMAL_ERROR("[Platform] Failed to create window");
        minimalDestroyWindow(window);
        return NULL;
    }

    minimalSetWindowTitle(window, title);

    return window;
}

void minimalDestroyWindow(MinimalWindow* window)
{
    // destroy window
    if (window->handle && !DestroyWindow(window->handle))
    {
        MINIMAL_ERROR("[Platform] Failed to destroy window");
    }

    MINIMAL_FREE(window, sizeof(MinimalWindow));
}

void minimalPollWindowEvents(MinimalWindow* window)
{
    MSG msg;
    while (PeekMessageA(&msg, window->handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

void minimalSetWindowTitle(MinimalWindow* window, const char* title)
{
    SetWindowTextA(window->handle, title);
}

u8   minimalShouldClose(const MinimalWindow* window) { return window->should_close; }
void minimalClose(MinimalWindow* window)             { window->should_close = 1; }


f64 minimalGetTime()
{
    u64 value;
    QueryPerformanceCounter((LARGE_INTEGER*)&value);

    return (f64)(value - _minimal_timer_offset) / _minimal_timer_frequency;
}

static u32 minimalGetKeyMods()
{
    u32 mods = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000)                         mods |= MINIMAL_KEY_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)                       mods |= MINIMAL_KEY_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000)                          mods |= MINIMAL_KEY_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) mods |= MINIMAL_KEY_MOD_SUPER;
    if (GetKeyState(VK_CAPITAL) & 1)                            mods |= MINIMAL_KEY_MOD_CAPS_LOCK;
    if (GetKeyState(VK_NUMLOCK) & 1)                            mods |= MINIMAL_KEY_MOD_NUM_LOCK;
    return mods;
}

static u16 minimalGetMouseButton(UINT msg, WPARAM wParam)
{
    if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)   return MINIMAL_MOUSE_BUTTON_LEFT;
    if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)   return MINIMAL_MOUSE_BUTTON_RIGHT;
    if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)   return MINIMAL_MOUSE_BUTTON_MIDDLE;
    if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)         return MINIMAL_MOUSE_BUTTON_4;
    else                                                return MINIMAL_MOUSE_BUTTON_5;
}

static LRESULT CALLBACK minimalWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MinimalWindow* context = minimalGetCurrentContext();

    if (!context) return DefWindowProcA(hwnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_ERASEBKGND:
        // Notify the OS that erasing will be handled by the application to prevent flicker.
        return 1;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        minimalClose(context);
        return 0;
    case WM_SIZE:
    {
        i32 width  = LOWORD(lParam);
        i32 height = HIWORD(lParam);

        minimalDispatchEvent(MINIMAL_EVENT_WINDOW_SIZE, 0, width, height);
        return 0;
    }
    case WM_CHAR:
    case WM_SYSCHAR:
    case WM_UNICHAR:
    {
        u32 codepoint = (u32)wParam;
        u32 mods = minimalGetKeyMods();

        if (codepoint > 31)
            minimalDispatchEvent(MINIMAL_EVENT_CHAR, codepoint, 0, mods);

        return 0;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        u8 action = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        u32 keycode = (u16)wParam;
        u32 mods = minimalGetKeyMods();

        if (minimalProcessKey(keycode, action))
            minimalDispatchEvent(MINIMAL_EVENT_KEY, keycode, action, mods);

        return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
        u8 action = msg == WM_LBUTTONDOWN
                        || msg == WM_RBUTTONDOWN
                        || msg == WM_MBUTTONDOWN
                        || msg == WM_XBUTTONDOWN;

        u16 button = minimalGetMouseButton(msg, wParam);
        i32 x = MINIMAL_GET_X_LPARAM(lParam);
        i32 y = MINIMAL_GET_Y_LPARAM(lParam);

        if (minimalProcessMouseButton(button, action))
            minimalDispatchEvent(MINIMAL_EVENT_MOUSE_BUTTON, (button << 16) + action, x, y);

        return msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP;
    }
    case WM_MOUSEMOVE:
    {
        i32 x = MINIMAL_GET_X_LPARAM(lParam);
        i32 y = MINIMAL_GET_Y_LPARAM(lParam);

        if (minimalProcessMouseButton((f32)x, (f32)y))
            minimalDispatchEvent(MINIMAL_EVENT_MOUSE_MOVED, 0, x, y);
        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        i32 scroll = MINIMAL_GET_SCROLL(wParam);
        minimalDispatchEvent(MINIMAL_EVENT_MOUSE_SCROLLED, 0, 0, scroll);
        return 0;
    }
    case WM_MOUSEHWHEEL:
    {
        i32 scroll = MINIMAL_GET_SCROLL(wParam);
        minimalDispatchEvent(MINIMAL_EVENT_MOUSE_SCROLLED, 0, scroll, 0);
        return 0;
    }
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

#ifdef MINIMAL_VULKAN

#include <vulkan/vulkan_win32.h>

VkResult minimalCreateWindowSurface(VkInstance instance, const MinimalWindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
{
    VkWin32SurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = window->instance,
        .hwnd = window->handle,
        .flags = 0
    };

    return vkCreateWin32SurfaceKHR(instance, &create_info, allocator, surface);
}

static const char* const extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    "VK_KHR_win32_surface",
};

const char* const *minimalQueryRequiredExtensions(u32* count)
{
    *count = sizeof(extensions) / sizeof(extensions[0]);
    return extensions;
}

#endif

#endif // MINIMAL_PLATFORM_WINDOWS
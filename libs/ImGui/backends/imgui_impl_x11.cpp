// dear imgui: Platform Binding for Linux (standard X11 API for 32 and 64 bits applications)
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan..)

// https://www.uninformativ.de/blog/postings/2017-04-02/0/POSTING-en.html
// https://stackoverflow.com/questions/27378318/c-get-string-from-clipboard-on-linux

// Implemented features:
//  [X] Platform: Clipboard support
//  [ ] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using
//  [ ] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

#include "imgui.h"
#include "imgui_impl_x11.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include <cstdlib>
#include <climits>
#include <ctime>
#include <cstdint>

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2021-15-06: Clipboard support.
//  2019-08-31: Initial X11 implementation.

// X11 Data
static Atom g_BufId;
static Atom g_PropId;
static Atom g_FmtIdUtf8String;
static Atom g_IncrId;

static char* g_ClipboardBuffer = NULL;
static size_t g_ClipboardBufferLength = 0;
static size_t g_ClipboardBufferSize = 0;
static bool g_ClipboardOwned = false;

static Display*             g_Display = nullptr;
static Window               g_Window = 0;
static uint64_t             g_Time = 0;
static uint64_t             g_TicksPerSecond = 0;
static ImGuiMouseCursor     g_LastMouseCursor = ImGuiMouseCursor_COUNT;
static bool                 g_HasGamepad = false;
static bool                 g_WantUpdateHasGamepad = true;

static bool GetKeyState(int keysym, char keys[32])
{
    int keycode = XKeysymToKeycode(g_Display, keysym);
    return keys[keycode/8] & (1<<keycode%8);
}

static inline int KeyToImGuiKey(int key)
{
    if (key > 255)
        return key - 0xFF00;

    return key;
}

static void ImGui_ImplX11_SendClipboard(XSelectionRequestEvent* sender)
{
    XSelectionEvent event;

    XChangeProperty(g_Display, sender->requestor, sender->property, g_FmtIdUtf8String, 8, PropModeReplace,
                    (const unsigned char*)g_ClipboardBuffer, g_ClipboardBufferLength);

    event.type = SelectionNotify;
    event.requestor = sender->requestor;
    event.selection = sender->selection;
    event.target = sender->target;
    event.property = sender->property;
    event.time = sender->time;

    XSendEvent(g_Display, sender->requestor, True, NoEventMask, (XEvent *)&event);
}

static void ImGui_ImplX11_SetClipboardText(void* user_data, const char* text)
{
    g_ClipboardBufferLength = strlen(text);
    if(g_ClipboardBufferLength > 0)
    {
        if(g_ClipboardBufferLength >= g_ClipboardBufferSize)
        {
            free(g_ClipboardBuffer);
            g_ClipboardBuffer = (char*)malloc(sizeof(char) * g_ClipboardBufferLength);
            g_ClipboardBufferSize = g_ClipboardBufferLength;
        }
        memcpy(g_ClipboardBuffer, text, g_ClipboardBufferLength);
    
        if(!g_ClipboardOwned)
        {
            g_ClipboardOwned = true;
            XSetSelectionOwner(g_Display, g_BufId, g_Window, CurrentTime);
        }
    }
}

static const char* ImGui_ImplX11_GetClipboardText(void *user_data)
{
    XEvent event;
    char *result;
    unsigned long ressize, restail;
    int resbits;
    bool timed_out;
    time_t now;

    now = time(NULL);
    timed_out = false;
    XConvertSelection(g_Display, g_BufId, g_FmtIdUtf8String, g_PropId, g_Window, CurrentTime);
    do
    {
        XNextEvent(g_Display, &event);
        if(event.type == SelectionRequest)
        {// This happens when we are requesting our own buffer.
            ImGui_ImplX11_SendClipboard(&event.xselectionrequest);
            continue;
        }
        if(time(NULL) - now > 2)
        {
            timed_out = true;
            break;
        }
    } 
    while (event.type != SelectionNotify || event.xselection.selection != g_BufId);

    g_ClipboardBuffer[0] = '\0';
    if (!timed_out && event.xselection.property)
    {
        XGetWindowProperty(g_Display, g_Window, g_PropId, 0, LONG_MAX/4, False, AnyPropertyType, 
            &g_FmtIdUtf8String, &resbits, &ressize, &restail, (unsigned char**)&result);

        if (g_FmtIdUtf8String == g_IncrId)
        {
            IM_ASSERT(0 && "Buffer is too large and INCR reading is not implemented yet.\n");
        }
        else
        {
            if(ressize > g_ClipboardBufferSize)
            {
                free(g_ClipboardBuffer);
                g_ClipboardBufferSize = ressize + 1;
                g_ClipboardBuffer = (char*)malloc(sizeof(char) * g_ClipboardBufferSize);
            }
            memcpy(g_ClipboardBuffer, result, ressize);
            g_ClipboardBuffer[ressize] = '\0';
        }

        XFree(result);
    }

    return g_ClipboardBuffer;
}

// Functions
bool    ImGui_ImplX11_Init(void *display, void *window)
{
    timespec ts, tsres;
    clock_getres(CLOCK_MONOTONIC_RAW, &tsres);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    g_TicksPerSecond = 1000000000.0f / (static_cast<uint64_t>(tsres.tv_nsec) + static_cast<uint64_t>(tsres.tv_sec)*1000000000);
    g_Time = static_cast<uint64_t>(ts.tv_nsec) + static_cast<uint64_t>(ts.tv_sec)*1000000000;

    // Setup back-end capabilities flags
    g_Display = reinterpret_cast<Display*>(display);
    g_Window = reinterpret_cast<Window>(window);
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_x11";
    io.ImeWindowHandle = nullptr;

    io.GetClipboardTextFn = ImGui_ImplX11_GetClipboardText;
    io.SetClipboardTextFn = ImGui_ImplX11_SetClipboardText;

    g_ClipboardBuffer = (char*)malloc(sizeof(char) * 256);
    g_ClipboardBufferSize = 256;

    g_BufId           = XInternAtom(g_Display, "CLIPBOARD", False);
    g_PropId          = XInternAtom(g_Display, "XSEL_DATA", False);
    g_FmtIdUtf8String = XInternAtom(g_Display, "UTF8_STRING", False);
    g_IncrId          = XInternAtom(g_Display, "INCR", False);

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab]         = KeyToImGuiKey(XK_Tab);
    io.KeyMap[ImGuiKey_LeftArrow]   = KeyToImGuiKey(XK_Left);
    io.KeyMap[ImGuiKey_RightArrow]  = KeyToImGuiKey(XK_Right);
    io.KeyMap[ImGuiKey_UpArrow]     = KeyToImGuiKey(XK_Up);
    io.KeyMap[ImGuiKey_DownArrow]   = KeyToImGuiKey(XK_Down);
    io.KeyMap[ImGuiKey_PageUp]      = KeyToImGuiKey(XK_Prior);
    io.KeyMap[ImGuiKey_PageDown]    = KeyToImGuiKey(XK_Next);
    io.KeyMap[ImGuiKey_Home]        = KeyToImGuiKey(XK_Home);
    io.KeyMap[ImGuiKey_End]         = KeyToImGuiKey(XK_End);
    io.KeyMap[ImGuiKey_Insert]      = KeyToImGuiKey(XK_Insert);
    io.KeyMap[ImGuiKey_Delete]      = KeyToImGuiKey(XK_Delete);
    io.KeyMap[ImGuiKey_Backspace]   = KeyToImGuiKey(XK_BackSpace);
    io.KeyMap[ImGuiKey_Space]       = KeyToImGuiKey(XK_space);
    io.KeyMap[ImGuiKey_Enter]       = KeyToImGuiKey(XK_Return);
    io.KeyMap[ImGuiKey_Escape]      = KeyToImGuiKey(XK_Escape);
    io.KeyMap[ImGuiKey_KeyPadEnter] = KeyToImGuiKey(XK_KP_Enter);
    io.KeyMap[ImGuiKey_A]           = KeyToImGuiKey(XK_a);
    io.KeyMap[ImGuiKey_C]           = KeyToImGuiKey(XK_c);
    io.KeyMap[ImGuiKey_V]           = KeyToImGuiKey(XK_v);
    io.KeyMap[ImGuiKey_X]           = KeyToImGuiKey(XK_x);
    io.KeyMap[ImGuiKey_Y]           = KeyToImGuiKey(XK_y);
    io.KeyMap[ImGuiKey_Z]           = KeyToImGuiKey(XK_z);

    return true;
}

void    ImGui_ImplX11_Shutdown()
{
    ImGuiIO& io = ImGui::GetIO();
    io.GetClipboardTextFn = NULL;
    io.SetClipboardTextFn = NULL;

    free(g_ClipboardBuffer); g_ClipboardBuffer = NULL;
    g_ClipboardBufferSize = 0;
    g_ClipboardBufferLength = 0;

    g_Display = nullptr;
    g_Window = 0;
}

static bool ImGui_ImplX11_UpdateMouseCursor()
{
    return true;
}

static void ImGui_ImplX11_UpdateMousePos()
{
    ImGuiIO& io = ImGui::GetIO();

    // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos)
    {
    //    POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
    //    ::ClientToScreen(g_hWnd, &pos);
    //    ::SetCursorPos(pos.x, pos.y);
    }

    // Set mouse position
    Window unused_window;
    int rx, ry, x, y;
    unsigned int mask;

    XQueryPointer(g_Display, g_Window, &unused_window, &unused_window, &rx, &ry, &x, &y, &mask);

    io.MousePos = ImVec2((float)x, (float)y);
}

// Gamepad navigation mapping
static void ImGui_ImplX11_UpdateGamepads()
{
    // TODO: support linux gamepad ?
#ifndef IMGUI_IMPL_X11_DISABLE_GAMEPAD
#endif
}

void    ImGui_ImplX11_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    unsigned int width, height;
    Window unused_window;
    int unused_int;
    unsigned int unused_unsigned_int;

    XGetGeometry(g_Display, (Window)g_Window, &unused_window, &unused_int, &unused_int, &width, &height, &unused_unsigned_int, &unused_unsigned_int);

    io.DisplaySize.x = width;
    io.DisplaySize.y = height;

    timespec ts, tsres;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    uint64_t current_time = static_cast<uint64_t>(ts.tv_nsec) + static_cast<uint64_t>(ts.tv_sec)*1000000000;

    io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;

    // Read keyboard modifiers inputs
    char keys[32];
    XQueryKeymap(g_Display, keys);

    io.KeyCtrl = GetKeyState(XK_Control_L, keys);
    io.KeyShift = GetKeyState(XK_Shift_L, keys);
    io.KeyAlt = GetKeyState(XK_Alt_L, keys);
    io.KeySuper = GetKeyState(XK_Super_L, keys);

    // io.KeysDown[], io.MousePos, io.MouseDown[], io.MouseWheel: filled by the WndProc handler below.

    // Update OS mouse position
    ImGui_ImplX11_UpdateMousePos();

    // Update game controllers (if enabled and available)
    ImGui_ImplX11_UpdateGamepads();
}

// Process X11 mouse/keyboard inputs.
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
IMGUI_IMPL_API int ImGui_ImplX11_EventHandler(XEvent &event)
{
    if (ImGui::GetCurrentContext() == NULL)
        return 0;

    ImGuiIO& io = ImGui::GetIO();
    switch (event.type)
    {
        case ButtonPress:
        case ButtonRelease:
            switch(event.xbutton.button)
            {
                case Button1:
                    io.MouseDown[0] = event.type == ButtonPress;
                    break;

                case Button2:
                    io.MouseDown[2] = event.type == ButtonPress;
                    break;

                case Button3:
                    io.MouseDown[1] = event.type == ButtonPress;
                    break;

                case Button4: // Mouse wheel up
                    if( event.type == ButtonPress )
                        io.MouseWheel += 1;
                    return 0;

                case Button5: // Mouse wheel down
                    if( event.type == ButtonPress )
                        io.MouseWheel -= 1;
                    return 0;
            }

            break;

        case KeyPress:
        {
            int key = XkbKeycodeToKeysym(g_Display, event.xkey.keycode, 0, event.xkey.state & ShiftMask ? 1 : 0);
            if( key < 255 )
            {
                io.KeysDown[key] = true;
                io.AddInputCharacter(key);
            }
            else if( key >= 0x1000100 && key <= 0x110ffff )
            {
                io.AddInputCharacterUTF16(key);
            }
            else
            {
                io.KeysDown[key - 0xFF00] = true; 
            }
            return 0;
        }

        case KeyRelease:
        {
            int key = XkbKeycodeToKeysym(g_Display, event.xkey.keycode, 0, event.xkey.state & ShiftMask ? 1 : 0);
            io.KeysDown[KeyToImGuiKey(key)] = false;
            return 0;
        }

        case FocusOut:
            memset(io.KeysDown, 0, sizeof(io.KeysDown));
            return 0;


        case SelectionClear:
            g_ClipboardOwned = false;
            return 0;

        case SelectionRequest:
        {
            ImGui_ImplX11_SendClipboard(&event.xselectionrequest);
        }
    }
    return 0;
}


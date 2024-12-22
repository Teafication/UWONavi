/***********************************************************************************************/
/*                                                                                             */
/*  UWONavi - Main Program Entry and Window Management                                         */
/*                                                                                             */
/***********************************************************************************************/
/*
    This file is the main heart of UWONavi.
    that manages nautical navigation in a game. It handles:

      1. Including system and external libraries (like GDI+ for graphics).
      2. Organizing project-specific headers containing classes and functions.
      3. Setting up global variables for the app instance, window handles, etc.
      4. Defining function prototypes for everything the program does.
      5. The main function (_tWinMain) that starts the Windows app.
      6. A window procedure (WndProc) that handles OS messages like mouse clicks,
         window movement, etc.
      7. A main loop (s_mainLoop) that processes messages and interacts with
         the GameProcess object to collect game data.
      8. Utility/helper functions for user interactions like menus,
         coordinate pop-ups, or opening file dialogs.
*/

/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: INCLUDE AND LINKER DIRECTIVES                                                     */
/*                                                                                             */
/***********************************************************************************************/
/*
    - "stdafx.h" is typically a precompiled header in Visual Studio that speeds
      up compilation by grouping commonly used headers.
    - <vector>, <list>, <fstream> are standard C++ libraries for dynamic arrays,
      doubly linked lists, and file I/O, respectively.
    - <gdiplus.h> and its associated library "gdiplus.lib" are for GDI+
      (Graphics Device Interface Plus), which helps with drawing images and shapes.
    - <Shlwapi.h> with "shlwapi.lib" provides the PathRemoveFileSpec function
      for string/path manipulation.
    - <CommCtrl.h> and <CommDlg.h> with their libs are for common controls
      (like progress bars, sliders, etc.) and common dialogs (like file open/save).
*/

#include "stdafx.h"
#include <vector>
#include <list>
#include <fstream>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
#include <CommDlg.h>
#pragma comment(lib, "Comdlg32.lib")

/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: LOCAL HEADER FILES                                                                */
/*                                                                                             */
/***********************************************************************************************/
/*
    These headers are specific to this project and contain the definitions for:
      - UWONavi: Potentially the main navigation logic or application structure.
      - Config: A class that handles reading/writing config files, storing settings (e.g., window size,
                keep-foreground boolean, polling intervals).
      - GameProcess: A class that interacts with the game to fetch data about the ship's position,
                     velocity, etc.
      - WorldMap: A class that represents the map or image on which the ship is rendered.
      - Ship, ShipRouteList, ShipRouteManageView: Classes that manage ship info, routes,
                                                  and route management UI.
      - Renderer & Texture: Classes to render the map, the ship, and handle textures.
*/

#include "UWONavi.h"
#include "Config.h"
#include "GameProcess.h"
#include "WorldMap.h"
#include "Ship.h"
#include "ShipRouteList.h"
#include "Renderer.h"
#include "Texture.h"
#include "ShipRouteManageView.h"

// Uncommenting this define will enable performance measuring in the code
// #define _PERF_CHECK


/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: GLOBAL VARIABLES                                                                  */
/*                                                                                             */
/***********************************************************************************************/
/*
    The following variables are shared across this file (and potentially the entire program).
    Typically, you'd keep globals to a minimum, but here they serve as shared references
    for our window, device contexts, config, etc.
*/

HINSTANCE g_hinst;    // The application instance
HWND g_hwndMain;      // Main window handle
HDC g_hdcMain;        // Device context for the main window

// Basic app details that show up in window titles and the About dialog
static LPCWSTR const k_appName = L"UWONavi";
static LPCWSTR const k_translated = L"Maintain by Anima";
static LPCWSTR const k_architecture = L"(64bits)";
static LPCWSTR const k_copyright = L"(c) 2024 Anima, London";

static LPCWSTR const k_windowClassName = L"Navi";           // Name of the window class
static const LPCWSTR k_configFileName = L"Navi.ini";       // Default config file
static LPCWSTR const k_appMutexName = L"Global\\{7554E265-3247-4FCA-BC60-5AA814658351}";
static HANDLE s_appMutex;    // Used to ensure single instance of this application

// GDI+ startup info and token
static Gdiplus::GdiplusStartupInput s_gdisi;
static ULONG_PTR s_gdiToken;

// Our main configuration object. We pass it around to set or get user preferences.
static Config s_config(k_configFileName);

// The main modules that handle gameplay, rendering, and map state.
static GameProcess s_GameProcess;
static Renderer s_renderer;
static WorldMap s_worldMap;

// A path to save or load route data
const std::wstring&& k_routeListFilePath = g_makeFullPath(L"RouteList.dat");

// This container manages our list of ship routes (with coordinates, etc.).
static std::unique_ptr<ShipRouteList> s_shipRouteList;

// Variables to keep track of the latest ship information retrieved from the game process
static POINT  s_latestSurveyCoord;
static Vector s_latestShipVector;
static double s_latestShipVelocity;
static DWORD  s_latestTimeStamp;

// This threshold is used to detect if game updates are too far apart, 
// at which point a route may be closed or considered invalid.
static const DWORD k_surveyCoordLostThreshold = 5000;

// The ShipRouteManageView is presumably a separate dialog/UI for route management
static std::unique_ptr<ShipRouteManageView> s_shipRouteManageView;

// A texture of the ship icon. If we want to draw the ship, we need a texture 
// to represent it on the map.
static std::unique_ptr<Texture> s_shipTexture;

// Time in milliseconds between updates from the game
static UINT s_pollingInterval = 1000;

// Variables for handling mouse dragging around the map
static bool  s_isDragging = false;
static SIZE  s_clientSize;
static POINT s_dragOrg;  // The starting point of a mouse drag

#ifdef _PERF_CHECK
#include <deque>
#include <numeric>
typedef std::deque<double> PerfCountList;
static PerfCountList s_perfCountList; // Used for performance measurements
#endif

/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: FUNCTION PROTOTYPES                                                               */
/*                                                                                             */
/***********************************************************************************************/
/*
    Forward declarations for the static (file-scope) functions.
    It's helpful for the compiler to know about them before we define them.
*/

// Registration & Initialization
static ATOM MyRegisterClass(HINSTANCE hInstance);
static BOOL InitInstance(HINSTANCE, int);

// Main loop that processes messages
static LRESULT s_mainLoop();

// Utility functions for file name retrieval, updating frames, window titles, etc.
static std::wstring s_getMapFileName();
static void s_updateFrame(HWND);
static void s_updateWindowTitle(HWND, POINT, double);
static void s_toggleKeepForeground(HWND);
static void s_popupMenu(HWND, int16_t, int16_t);
static void s_popupCoord(HWND, int16_t, int16_t);
static void s_closeShipRoute();

// Our Window Procedure for handling events
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Dialog procedure for the About box
INT_PTR CALLBACK aboutDlgProc(HWND, UINT, WPARAM, LPARAM);

// Handlers for window messages: create, move, size, paint, mouse, etc.
static bool s_onCreate(HWND, LPCREATESTRUCT);
static void s_onMove(HWND, WORD, WORD);
static void s_onSize(HWND, UINT, WORD, WORD);
static void s_onMouseWheel(HWND, int16_t, UINT, int16_t, int16_t);
static void s_onMouseMove(HWND, UINT, int16_t, int16_t);
static void s_onMouseLeftButtonDown(HWND, UINT, int16_t, int16_t);
static void s_onMouseLeftButtonUp(HWND, UINT, int16_t, int16_t);
static void s_onMouseLeftButtonDoubleClick(HWND, UINT, int16_t, int16_t);
static void s_onMouseRightButtonUp(HWND, UINT, int16_t, int16_t);
static void s_onPaint(HWND);


/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: MAIN ENTRY POINT (_tWinMain)                                                      */
/*                                                                                             */
/***********************************************************************************************/
/*
    This is where Windows looks for the start of the program (for desktop apps).
    It creates a mutex to prevent multiple instances from running, initializes
    COM, sets up GDI+, registers and creates the main window, and finally
    enters our main loop.
*/
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Create a mutex to ensure only one instance of the application is run
    ::SetLastError(NOERROR);
    s_appMutex = ::CreateMutex(NULL, TRUE, k_appMutexName);
    if (::GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND hwnd = ::FindWindow(k_windowClassName, NULL);
        if (hwnd)
        {
            ::SetForegroundWindow(hwnd);
        }
        return 0;
    }

    // Initialize COM (used for various Windows APIs), 
    // set a minimal timer resolution for the system, 
    // initialize common controls, and start GDI+.
    ::CoInitialize(NULL);

    TIMECAPS tc;
    ::timeGetDevCaps(&tc, sizeof(tc));
    ::timeBeginPeriod(tc.wPeriodMin);

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES };
    ::InitCommonControlsEx(&icc);

    Gdiplus::GdiplusStartup(&s_gdiToken, &s_gdisi, NULL);

    // Load config settings from the INI file
    s_config.load();

    // Register our custom window class and create the window
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow))
    {
        return 0;
    }

    // Enter our main loop which handles messages and game updates
    const LRESULT retVal = s_mainLoop();

    // Upon exiting, attempt to save the route list data to a file
    try
    {
        std::ofstream ofs;
        ofs.exceptions(std::ios::badbit | std::ios::failbit);
        ofs.open(k_routeListFilePath, std::ios::out | std::ios::binary | std::ios::trunc);
        ofs << *s_shipRouteList;
        ofs.close();
    }
    catch (const std::exception& e)
    {
        ::OutputDebugStringA((std::string("file save error:") + e.what() + "\n").c_str());
        ::MessageBox(NULL, L"Failed to save the route", k_appName, MB_ICONERROR);
    }

    // Shutdown: close game process, save config, shut down GDI+ and revert system timer
    s_GameProcess.teardown();
    s_config.save();
    Gdiplus::GdiplusShutdown(s_gdiToken);
    ::timeEndPeriod(tc.wPeriodMin);
    ::CoUninitialize();

    return retVal;
}


/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: MyRegisterClass                                                                   */
/*                                                                                             */
/***********************************************************************************************/
/*
    Here we register a new window class with certain styles. The system
    needs to know how to create the window, handle icons, the cursor,
    background brush, etc.
*/
static ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = { sizeof(wcex) };

    // Double-click style, own device context, etc.
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAINFRAME);
    wcex.lpszClassName = k_windowClassName;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}


/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: InitInstance                                                                      */
/*                                                                                             */
/***********************************************************************************************/
/*
    InitInstance will create the main window, attempt to load the map file,
    configure polling intervals, and set up rendering. If we can’t load a map,
    we let the user pick one.
*/
static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    // Attempt to load the map file from config
    if (!s_worldMap.loadFromFile(s_config.m_mapFileName))
    {
        // If that fails, prompt user for a map image file
        std::wstring fileName = s_getMapFileName();
        if (!s_worldMap.loadFromFile(fileName))
        {
            // If we still fail, show an error
            ::MessageBox(NULL,
                L"Could not open the map image.",
                k_appName,
                MB_ICONERROR | MB_SETFOREGROUND | MB_OK);
            return FALSE;
        }
        // If it’s successful, remember the file name in our config
        s_config.m_mapFileName = fileName;
    }

    // Prepare window style bits
    DWORD exStyle = 0;
    if (s_config.m_keepForeground)
    {
        exStyle |= WS_EX_TOPMOST;
    }

    DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    // Create the main window
    HWND hwnd = CreateWindowEx(exStyle,
        k_windowClassName,
        k_appName,
        style,
        s_config.m_windowPos.x,
        s_config.m_windowPos.y,
        s_config.m_windowSize.cx,
        s_config.m_windowSize.cy,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hwnd)
    {
        return FALSE;
    }

    // Keep references to our instance, main window handle, device context
    g_hinst = hInstance;
    g_hwndMain = hwnd;
    g_hdcMain = ::GetDC(g_hwndMain);

    // Set up the renderer with the map and config
    s_renderer.setup(&s_config, g_hdcMain, &s_worldMap);

    // Load any previously saved route data
    s_shipRouteList.reset(new ShipRouteList());
    try
    {
        std::ifstream ifs;
        ifs.open(k_routeListFilePath, std::ios::in | std::ios::binary);
        if (ifs)
        {
            ifs.exceptions(std::ios::badbit | std::ios::failbit);
            ifs >> *s_shipRouteList;
            ifs.close();
        }
    }
    catch (const std::exception& e)
    {
        ::OutputDebugStringA((std::string("file load error:") + e.what() + "\n").c_str());
        ::MessageBox(NULL, L"Failed to read path", k_appName, MB_ICONERROR);
    }

    // Set polling interval from config, 
    // then connect with the game (open process handle, etc.)
    s_pollingInterval = s_config.m_pollingInterval;
    s_GameProcess.setup(s_config);

    // Update the window title to reflect coords, version, etc.
    s_updateWindowTitle(hwnd, s_config.m_initialSurveyCoord, s_renderer.viewScale());

    // Finally, show and update the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return TRUE;
}


/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: s_mainLoop                                                                        */
/*                                                                                             */
/***********************************************************************************************/
/*
    Our main message loop that runs until the program is closed. We:
      - Peek messages from the OS. If it’s a WM_QUIT, we break.
      - Process accelerator keys, translate messages, etc.
      - Wait for either user input or some event from the GameProcess (like dataReadyEvent)
        to keep updating the map and data.
*/
static LRESULT s_mainLoop()
{
    MSG msg;
    HACCEL hAccelTable = LoadAccelerators(g_hinst, MAKEINTRESOURCE(IDR_MAINFRAME));

    for (;;)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // If we got a quit message, break out
            if (msg.message == WM_QUIT)
            {
                break;
            }
            // If it’s not an accelerator key, handle normal translation
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            continue;
        }

        // If there are no messages to process, we set up a wait on:
        //  1. The game process handle (in case the game closes).
        //  2. The dataReadyEvent (in case new data is available).
        std::vector<HANDLE> handles;
        if (s_GameProcess.processHandle())
        {
            handles.push_back(s_GameProcess.processHandle());
        }
        handles.push_back(s_GameProcess.dataReadyEvent());

        // If we don’t have anything to wait on, we just wait for a message
        if (handles.empty())
        {
            ::WaitMessage();
            continue;
        }

        DWORD waitResult = ::MsgWaitForMultipleObjects(
            static_cast<DWORD>(handles.size()),
            &handles[0],
            FALSE,
            INFINITE,
            QS_ALLINPUT
        );

        // If the wait caused a message to pop, handle that first
        if (handles.size() <= waitResult)
        {
            continue;
        }

        // Check which handle triggered: if it’s the process handle, the game is closing
        HANDLE activeHandle = handles[waitResult];
        if (activeHandle == s_GameProcess.processHandle())
        {
            s_GameProcess.clear();
            continue;
        }

        // If it’s the dataReadyEvent, let’s update the frame
        if (activeHandle == s_GameProcess.dataReadyEvent())
        {
            s_updateFrame(g_hwndMain);
            continue;
        }
    }
    return (int)msg.wParam;
}


/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: WndProc                                                                           */
/*                                                                                             */
/***********************************************************************************************/
/*
    This is our window procedure: the callback that Windows calls
    whenever there’s an event (mouse click, paint request, etc.)
    for our main window. We handle each message in a switch statement.
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    int wmId, wmEvent;

    switch (message)
    {
    case WM_ERASEBKGND:
        // We handle background erase in our rendering, so return TRUE to skip default
        return TRUE;

    case WM_PAINT:
        s_onPaint(hwnd);
        break;

    case WM_TIMER:
        // We use a timer to periodically update the frame
        s_updateFrame(hwnd);
        break;

    case WM_MOVE:
        // Update config with new window position
        s_onMove(hwnd, int16_t(LOWORD(lp)), int16_t(HIWORD(lp)));
        break;

    case WM_SIZE:
        // Handle resizing (e.g., store new size, update renderer, etc.)
        s_onSize(hwnd, wp, LOWORD(lp), HIWORD(lp));
        break;

    case WM_COMMAND:
        wmId = LOWORD(wp);
        wmEvent = HIWORD(wp);
        switch (wmId)
        {
        case IDM_ABOUT:
            ::DialogBox(g_hinst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, aboutDlgProc);
            break;
        case IDM_EXIT:
            DestroyWindow(hwnd);
            break;
        case IDM_TOGGLE_TRACE_SHIP:
            s_config.m_traceShipPositionEnabled = !s_config.m_traceShipPositionEnabled;
            s_renderer.enableTraceShip(s_config.m_traceShipPositionEnabled);
            break;
        case IDM_ERASE_SHIP_ROUTE:
            s_shipRouteList->clearAllItems();
            break;
        case IDM_TOGGLE_KEEP_FOREGROUND:
            s_toggleKeepForeground(hwnd);
            break;
        case IDM_TOGGLE_SPEED_METER:
            s_config.m_speedMeterEnabled = !s_config.m_speedMeterEnabled;
            s_renderer.enableSpeedMeter(s_config.m_speedMeterEnabled);
            ::InvalidateRect(hwnd, NULL, FALSE);
            break;
        case IDM_TOGGLE_VECTOR_LINE:
            s_config.m_shipVectorLineEnabled = !s_config.m_shipVectorLineEnabled;
            s_renderer.setVisibleShipRoute(s_config.m_shipVectorLineEnabled);
            break;
        case IDM_SAME_SCALE:
            if (s_renderer.viewScale() != 1.0)
            {
                s_renderer.resetViewScale();
            }
            break;
        case IDM_ZOOM_IN:
            if (s_renderer.zoomIn())
            {
#ifdef _PERF_CHECK
                s_perfCountList.clear();
#endif
                s_updateWindowTitle(hwnd, s_latestSurveyCoord, s_renderer.viewScale());
                ::InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case IDM_ZOOM_OUT:
            if (s_renderer.zoomOut())
            {
#ifdef _PERF_CHECK
                s_perfCountList.clear();
#endif
                s_updateWindowTitle(hwnd, s_latestSurveyCoord, s_renderer.viewScale());
                ::InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        case IDM_SHOW_SHIPROUTEMANAGEVIEW:
            // Show or create the ShipRouteManageView dialog
            if (!s_shipRouteManageView.get())
            {
                s_shipRouteManageView.reset(new ShipRouteManageView());
                if (!s_shipRouteManageView->setup(*s_shipRouteList.get()))
                {
                    ::MessageBox(hwnd, L"Something went wrong", L"Error", MB_OK | MB_ICONERROR);
                }
            }
            else
            {
                s_shipRouteManageView->activate();
            }
            break;
#ifndef NDEBUG
        case IDM_TOGGLE_DEBUG_AUTO_CRUISE:
            s_config.m_debugAutoCruiseEnabled = !s_config.m_debugAutoCruiseEnabled;
            s_GameProcess.enableDebugAutoCruise(s_config.m_debugAutoCruiseEnabled);
            break;
        case IDM_DEBUG_CLOSE_ROUTE:
            s_closeShipRoute();
            break;
        case IDM_DEBUG_INTERVAL_NORMAL:
            s_pollingInterval = 1000;
            s_GameProcess.setPollingInterval(s_pollingInterval);
            break;
        case IDM_DEBUG_INTERVAL_HIGH:
            s_pollingInterval = 1;
            s_GameProcess.setPollingInterval(s_pollingInterval);
            break;
#endif
        default:
            return DefWindowProc(hwnd, message, wp, lp);
        }
        break;

    case WM_MOUSEWHEEL:
        s_onMouseWheel(hwnd, HIWORD(wp), LOWORD(wp), int16_t(LOWORD(lp)), int16_t(HIWORD(lp)));
        break;

    case WM_MOUSEMOVE:
        s_onMouseMove(hwnd, wp, int16_t(LOWORD(lp)), int16_t(HIWORD(lp)));
        break;

    case WM_LBUTTONDOWN:
        s_onMouseLeftButtonDown(hwnd, wp, int16_t(LOWORD(lp)), int16_t(HIWORD(lp)));
        break;

    case WM_LBUTTONUP:
        s_onMouseLeftButtonUp(hwnd, wp, int16_t(LOWORD(lp)), int16_t(HIWORD(lp)));
        break;

    case WM_RBUTTONUP:
        s_onMouseRightButtonUp(hwnd, wp, int16_t(LOWORD(lp)), int16_t(HIWORD(lp)));
        break;

    case WM_LBUTTONDBLCLK:
        s_onMouseLeftButtonDoubleClick(hwnd, wp, int16_t(LOWORD(lp)), int16_t(HIWORD(lp)));
        break;

    case WM_CREATE:
        if (!s_onCreate(hwnd, reinterpret_cast<LPCREATESTRUCT>(lp)))
        {
            return -1;
        }
        break;

    case WM_DESTROY:
        // Cleanup
        if (s_shipRouteManageView.get())
        {
            s_shipRouteManageView.reset();
        }
        s_renderer.teardown();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wp, lp);
    }
    return 0;
}


/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: HELPER & EVENT HANDLERS                                                           */
/*                                                                                             */
/***********************************************************************************************/
/*
    These functions each handle a specific window message,
    or provide a small utility.
*/

// Called when the window is first created
static bool s_onCreate(HWND /*hwnd*/, LPCREATESTRUCT /*cs*/)
{
    return true;
}

// When user drags the window, save the new position in the config
static void s_onMove(HWND hwnd, WORD /*cx*/, WORD /*cy*/)
{
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
    if (style & WS_MAXIMIZE) return;

    RECT rc = { 0 };
    ::GetWindowRect(hwnd, &rc);
    s_config.m_windowPos.x = rc.left;
    s_config.m_windowPos.y = rc.top;
}

// Called when window is resized
static void s_onSize(HWND hwnd, UINT state, WORD cx, WORD cy)
{
    RECT rc = { 0 };
    switch (state)
    {
    case SIZE_RESTORED:
        ::GetWindowRect(hwnd, &rc);
        s_config.m_windowSize.cx = rc.right - rc.left;
        s_config.m_windowSize.cy = rc.bottom - rc.top;
        break;
    case SIZE_MAXIMIZED:
        break;
    default:
        return;
    }

    // If the client area changed, inform the renderer
    if (s_clientSize.cx != cx || s_clientSize.cy != cy)
    {
        s_clientSize.cx = cx;
        s_clientSize.cy = cy;
        s_renderer.setViewSize(s_clientSize);
    }
}

// Called when user scrolls the mouse wheel (zoom in/out)
static void s_onMouseWheel(HWND hwnd, int16_t delta, UINT /*vkey*/, int16_t /*x*/, int16_t /*y*/)
{
    bool isChanged = false;
    if (delta > 0)
    {
        isChanged = s_renderer.zoomIn();
    }
    else
    {
        isChanged = s_renderer.zoomOut();
    }

    if (isChanged)
    {
#ifdef _PERF_CHECK
        s_perfCountList.clear();
#endif
        s_updateWindowTitle(hwnd, s_latestSurveyCoord, s_renderer.viewScale());
        ::InvalidateRect(hwnd, NULL, FALSE);
    }
}

// Called whenever the mouse is moved over the window
static void s_onMouseMove(HWND hwnd, UINT /*vkey*/, int16_t x, int16_t y)
{
    if (s_isDragging)
    {
        int dx = x - s_dragOrg.x;
        int dy = y - s_dragOrg.y;

        // If we’re in tracing-ship mode, a tiny drag might be ignored
        int threshold = 1;
        if (s_config.m_traceShipPositionEnabled)
        {
            if (::abs(dx) <= threshold && ::abs(dy) < threshold)
            {
                return;
            }
        }

        POINT offset = { -dx, -dy };
        s_renderer.offsetFocusInViewCoord(offset);
        ::InvalidateRect(hwnd, NULL, FALSE);

        s_dragOrg.x = x;
        s_dragOrg.y = y;

        // Turn off trace-ship if the user started dragging
        s_config.m_traceShipPositionEnabled = false;
        s_renderer.enableTraceShip(false);
    }
    else
    {
        // If not dragging, do nothing extra
    }
}

// Called when user presses left mouse button
static void s_onMouseLeftButtonDown(HWND hwnd, UINT /*vkey*/, int16_t x, int16_t y)
{
    if (!s_isDragging)
    {
        ::SetCapture(hwnd);
        s_isDragging = true;
        s_dragOrg.x = x;
        s_dragOrg.y = y;
    }
}

// Called when user releases left mouse button
static void s_onMouseLeftButtonUp(HWND /*hwnd*/, UINT /*vkey*/, int16_t /*x*/, int16_t /*y*/)
{
    if (s_isDragging)
    {
        ::ReleaseCapture();
        s_isDragging = false;
        s_dragOrg.x = 0;
        s_dragOrg.y = 0;
    }
}

// Double-click with left mouse button
static void s_onMouseLeftButtonDoubleClick(HWND hwnd, UINT /*vkey*/, int16_t x, int16_t y)
{
    if (!s_isDragging)
    {
        s_popupCoord(hwnd, x, y);
    }
}

// Right mouse button up, pop up context menu
static void s_onMouseRightButtonUp(HWND hwnd, UINT /*vkey*/, int16_t x, int16_t y)
{
    if (!s_isDragging)
    {
        s_popupMenu(hwnd, x, y);
    }
}

// Called when the window is invalidated and needs repainting
static void s_onPaint(HWND hwnd)
{
#ifdef _PERF_CHECK
    int64_t perfBegin = g_queryPerformanceCounter();
#endif

    // Draw map, route, ship, etc.
    s_renderer.render(
        s_latestShipVector,
        s_latestShipVelocity,
        s_shipTexture.get(),
        s_shipRouteList.get()
    );
    ::ValidateRect(hwnd, NULL);

#ifdef _PERF_CHECK
    int64_t perfEnd = g_queryPerformanceCounter();
    int64_t freq = g_queryPerformanceFrequency();
    double deltaPerSec = (double(perfEnd - perfBegin) / double(freq)) * 1000.0;
    s_perfCountList.push_back(deltaPerSec);

    // Average over a certain number of frames
    double average = std::accumulate(s_perfCountList.begin(), s_perfCountList.end(), 0.0)
        / s_perfCountList.size();
    if (s_perfCountList.size() > 100)
    {
        s_perfCountList.pop_front();
    }

    // Display performance measurement in the window title
    std::wstring s = std::wstring(L"Drawing speed:") + std::to_wstring(average) + L"(ms)\n";
    ::SetWindowText(hwnd, s.c_str());
#endif
}


/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: FILE DIALOG & FRAME UPDATES                                                       */
/*                                                                                             */
/***********************************************************************************************/
/*
    These functions help us get a file path for the map,
    update frames with game data, and refresh the window title with new info.
*/

// Pops up a file dialog for selecting the map file
static std::wstring s_getMapFileName()
{
    wchar_t dir[MAX_PATH] = { 0 };
    ::GetModuleFileName(g_hinst, dir, _countof(dir));
    ::PathRemoveFileSpec(dir);

    wchar_t filePath[MAX_PATH] = { 0 };
    OPENFILENAME ofn = { sizeof(ofn) };
    ofn.lpstrTitle = L"Please select a map image file.";
    ofn.lpstrInitialDir = &dir[0];
    // We filter for image files like bmp, png, jpg, etc.
    ofn.lpstrFilter = L"Image file\0*.bmp;*.jpg;*.jpeg;*.png;*.gif;*.tif;*.tiff\0All Files\0*.*\0\0";
    ofn.Flags = OFN_READONLY | OFN_FILEMUSTEXIST;
    ofn.nMaxFile = _countof(filePath);
    ofn.lpstrFile = &filePath[0];

    if (!::GetOpenFileName(&ofn))
    {
        return L"";
    }
    return filePath;
}

// Update frame with fresh data from the game process
static void s_updateFrame(HWND hwnd)
{
    // Ask GameProcess for new data 
    std::vector<GameStatus> gameStats = s_GameProcess.getState();
    if (gameStats.empty())
    {
        return;
    }

    // If ship texture wasn’t created yet, try now
    if (!s_shipTexture.get())
    {
        const Image* image = s_GameProcess.shipIconImage();
        if (image)
        {
            s_shipTexture.reset(s_renderer.createTextureFromImage(*image));
        }
    }

    // For each new status, update our variables and ship route
    for (auto& status : gameStats)
    {
        s_latestSurveyCoord = status.m_surveyCoord;
        s_latestShipVector = status.m_shipVector;
        s_latestShipVelocity = status.m_shipVelocity;

        // Keep the config up to date with the latest coordinate
        s_config.m_initialSurveyCoord = s_latestSurveyCoord;
        s_renderer.setShipPositionInWorld(s_latestSurveyCoord);

        // If it’s been too long since last update, 
        // consider closing out the route
        if ((s_latestTimeStamp + k_surveyCoordLostThreshold) < status.m_timeStamp)
        {
            s_closeShipRoute();
        }
        s_latestTimeStamp = status.m_timeStamp;

        // Add the new route point to our route list
        s_shipRouteList->addRoutePoint(s_worldMap.normalizedPoint(s_latestSurveyCoord));
    }

#ifndef _PERF_CHECK
    // Update the title with coordinate info
    s_updateWindowTitle(hwnd, s_latestSurveyCoord, s_renderer.viewScale());
#endif
    // Invalidate to trigger a repaint
    ::InvalidateRect(hwnd, NULL, FALSE);
}

// Refresh window title with the latest coordinates and scale
static void s_updateWindowTitle(HWND hwnd, POINT surveyCoord, double viewScale)
{
    std::vector<wchar_t> buf(4096);
    ::swprintf(&buf[0], buf.size(), L"%d,%d - (%.1f%%) - %s %s",
        surveyCoord.x,
        surveyCoord.y,
        viewScale * 100.0,
        k_appName,
        k_architecture);
    ::SetWindowText(hwnd, &buf[0]);
}


/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: MISCELLANEOUS FUNCTIONS                                                           */
/*                                                                                             */
/***********************************************************************************************/

// Toggle the window's "always on top" mode
static void s_toggleKeepForeground(HWND hwnd)
{
    if (::GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
    {
        // Remove topmost
        ::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
        s_config.m_keepForeground = false;
    }
    else
    {
        // Make it topmost
        ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
        s_config.m_keepForeground = true;
    }
}

// Display a pop-up menu at the cursor position
static void s_popupMenu(HWND hwnd, int16_t x, int16_t y)
{
    HMENU hmenu = ::LoadMenu(g_hinst, MAKEINTRESOURCE(IDR_POPUPMENU));
    HMENU popupMenu = ::GetSubMenu(hmenu, 0);

    // Check menu items based on current config state
    ::CheckMenuItem(popupMenu, IDM_TOGGLE_TRACE_SHIP,
        s_config.m_traceShipPositionEnabled ? MF_CHECKED : MF_UNCHECKED);
    ::CheckMenuItem(popupMenu, IDM_TOGGLE_KEEP_FOREGROUND,
        s_config.m_keepForeground ? MF_CHECKED : MF_UNCHECKED);
    ::CheckMenuItem(popupMenu, IDM_TOGGLE_SPEED_METER,
        s_config.m_speedMeterEnabled ? MF_CHECKED : MF_UNCHECKED);
    ::CheckMenuItem(popupMenu, IDM_TOGGLE_VECTOR_LINE,
        s_config.m_shipVectorLineEnabled ? MF_CHECKED : MF_UNCHECKED);

#ifndef NDEBUG
    // Debug options for development
    MENUITEMINFO mii = { sizeof(mii) };
    mii.fMask = MIIM_TYPE | MIIM_ID;
    mii.fType = MFT_STRING;

    mii.wID = IDM_TOGGLE_DEBUG_AUTO_CRUISE;
    mii.dwTypeData = L"[DEBUG]Enable automatic sailing";
    ::InsertMenuItem(popupMenu, ::GetMenuItemCount(popupMenu), TRUE, &mii);
    ::CheckMenuItem(popupMenu, IDM_TOGGLE_DEBUG_AUTO_CRUISE,
        s_config.m_debugAutoCruiseEnabled ? MF_CHECKED : MF_UNCHECKED);

    mii.wID = IDM_DEBUG_CLOSE_ROUTE;
    mii.dwTypeData = L"[DEBUG]Close route";
    ::InsertMenuItem(popupMenu, ::GetMenuItemCount(popupMenu), TRUE, &mii);

    mii.wID = IDM_DEBUG_INTERVAL_NORMAL;
    mii.dwTypeData = L"[DEBUG]Update interval - standard";
    ::InsertMenuItem(popupMenu, ::GetMenuItemCount(popupMenu), TRUE, &mii);

    mii.wID = IDM_DEBUG_INTERVAL_HIGH;
    mii.dwTypeData = L"[DEBUG]Update interval - high";
    ::InsertMenuItem(popupMenu, ::GetMenuItemCount(popupMenu), TRUE, &mii);
#endif

    // While the menu is open, let’s keep updating 
    UINT_PTR timerID = ::SetTimer(hwnd, 0, s_pollingInterval, NULL);
    s_updateFrame(hwnd);

    // Convert client coords to screen coords before popping up
    POINT p = { x, y };
    ::ClientToScreen(hwnd, &p);

    ::TrackPopupMenu(popupMenu,
        TPM_NONOTIFY | TPM_NOANIMATION | TPM_LEFTALIGN | TPM_TOPALIGN,
        p.x, p.y, 0, hwnd, NULL);
    ::DestroyMenu(popupMenu);

    // Destroy the timer after the user’s done with the menu
    ::KillTimer(hwnd, timerID);
}

// A function called on double-click, could be extended to do something with map coords
static void s_popupCoord(HWND /*hwnd*/, int16_t /*x*/, int16_t /*y*/)
{
    // Currently does nothing, but it's a place holder for double-click 
    // coordinate interactions.
}

// Close the current route (when we suspect the route is stale or the user requested it)
static void s_closeShipRoute()
{
    s_shipRouteList->closeRoute();
}


/***********************************************************************************************/
/*                                                                                             */
/*  SECTION: aboutDlgProc                                                                      */
/*                                                                                             */
/***********************************************************************************************/
/*
    The dialog callback for the “About” box. It sets text fields for version,
    translator, and copyright info, then closes on OK or Cancel.
*/
INT_PTR CALLBACK aboutDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM /*lp*/)
{
    switch (msg)
    {
    case WM_INITDIALOG:
    {
        std::wstring versionString = std::wstring(k_appName) + L" " + k_architecture;
        std::wstring tlString = std::wstring(k_translated);
        std::wstring copyRightString = std::wstring(k_copyright);

        ::SetDlgItemText(hwnd, IDC_VERSION_LABEL, versionString.c_str());
        ::SetDlgItemText(hwnd, IDC_VERSION_LABEL2, tlString.c_str());
        ::SetDlgItemText(hwnd, IDC_COPYRIGHT_LABEL, copyRightString.c_str());
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wp))
        {
        case IDOK:
        case IDCANCEL:
            ::EndDialog(hwnd, 0);
            break;
        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

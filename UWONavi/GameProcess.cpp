#include "stdafx.h"
#include <process.h>
#include "UWONavi.h"
#include "GameProcess.h"
#include "WorldMap.h"
#include "SurveyCoordExtractor.h"

// These external variables are declared elsewhere and used here.
extern HWND g_hwndMain;
extern HDC g_hdcMain;

/**
 * Anonymous namespace for static (file-scoped) variables and constants
 * that we only use in this file.
 */
namespace {

    // The window class name and caption to look for when locating the game window.
    LPWSTR const k_gvoWindowClassName = L"Greate Voyages Online Game MainFrame";
    LPWSTR const k_gvoWindowCaption = L"Uncharted Waters Online";

    // Offset and size for reading survey coordinates from the game screen.
    const POINT k_surveyCoordOffsetFromRightBottom = { 70, 273 };
    const SIZE  k_surveyCoordSize = { 60, 11 };

#ifndef NDEBUG
    static double   s_xDebugAutoCruise = 0.0;
    static double   s_yDebugAutoCruise = 0.0;
    static double   s_debugAutoCruiseAngle = 0.0;
    static bool     s_debugAutoCruiseEnabled = false;
    static double   s_debugAutoCruiseVelocity = 0.0;
    static uint32_t s_debugAutoCruiseTurnInterval = 0;
    static double   s_debugAutoCruiseTurnAngle = 0.0;
#endif

#ifdef GVO_ANALYZE_DEBUG
    LPCWSTR const k_debugImageFileName = L"..\\debug.png";
#endif

} // anonymous namespace

/**
 * Clear out the GameProcess fields that reference the process and window.
 * This function closes the handle to the process (if it exists).
 */
void GameProcess::clear() {
    if (m_process) {
        ::CloseHandle(m_process);
        m_process = NULL;
    }
    m_window = NULL;
}

/**
 * Setup initializes the GameProcess using the provided configuration.
 * It also starts a timer event for polling and begins a worker thread.
 */
void GameProcess::setup(const Config& config) {
    m_surveyCoord = config.m_initialSurveyCoord;
    m_ship.setInitialSurveyCoord(config.m_initialSurveyCoord);
    m_pollingInterval = config.m_pollingInterval;

#ifndef NDEBUG
    s_xDebugAutoCruise = config.m_initialSurveyCoord.x;
    s_yDebugAutoCruise = config.m_initialSurveyCoord.y;
    s_debugAutoCruiseEnabled = config.m_debugAutoCruiseEnabled;
    s_debugAutoCruiseVelocity = config.m_debugAutoCruiseVelocity;
    s_debugAutoCruiseTurnInterval = config.m_debugAutoCruiseTurnInterval;
    s_debugAutoCruiseTurnAngle = config.m_debugAutoCruiseTurnAngle;
#endif

    // Create a periodic timer event for polling.
    m_pollingTimerEventID = ::timeSetEvent(
        m_pollingInterval,
        1,
        LPTIMECALLBACK(m_pollingTimerEvent),
        0,
        TIME_PERIODIC | TIME_CALLBACK_EVENT_SET
    );

    // Create a signal to tell the worker thread when to quit.
    m_threadQuitSignal = ::CreateEvent(NULL, TRUE, FALSE, NULL);

    // Start the worker thread.
    m_workerThread = reinterpret_cast<HANDLE>(::_beginthreadex(
        NULL,
        0,
        threadMainThunk,
        this,
        0,
        NULL
    ));
}

/**
 * Teardown cleans up the worker thread, stops the timer, and ensures
 * all handles are closed.
 */
void GameProcess::teardown() {
    if (m_workerThread) {
        ::SetEvent(m_threadQuitSignal);
        ::WaitForSingleObject(m_workerThread, INFINITE);
        ::CloseHandle(m_workerThread);
        ::CloseHandle(m_threadQuitSignal);
        m_threadQuitSignal = NULL;
        m_workerThread = NULL;
    }

    if (m_pollingTimerEventID) {
        ::timeKillEvent(m_pollingTimerEventID);
        m_pollingTimerEventID = 0;
    }
}

#ifndef NDEBUG
/**
 * Toggles whether debug auto-cruise mode is enabled.
 * Debug auto-cruise simulates movement in the game world when not running the actual game.
 */
void GameProcess::enableDebugAutoCruise(bool enabled) {
    s_debugAutoCruiseEnabled = enabled;
}

/**
 * Adjusts how frequently polling occurs, even after the timer event
 * has already been created. This kills the old timer and recreates it
 * with the new interval.
 */
void GameProcess::setPollingInterval(DWORD interval) {
    m_pollingInterval = interval;
    if (m_pollingTimerEventID) {
        ::timeKillEvent(m_pollingTimerEventID);
    }
    m_pollingTimerEventID = ::timeSetEvent(
        m_pollingInterval,
        1,
        LPTIMECALLBACK(m_pollingTimerEvent),
        0,
        TIME_PERIODIC | TIME_CALLBACK_EVENT_SET
    );
}
#endif

/**
 * updateState attempts to locate the game window, grab part of the screen,
 * and extract the current survey coordinates. It then updates the game status
 * and stores that status in a buffer.
 *
 * Returns true if the update was successful or in debug mode; false otherwise.
 */
bool GameProcess::updateState() {
    GameStatus status;

    // Attempt to locate the game window and open the process handle if needed.
    if (!m_window) {
        m_window = ::FindWindow(k_gvoWindowClassName, k_gvoWindowCaption);
        if (m_window) {
            if (!m_process) {
                DWORD pid = 0;
                ::GetWindowThreadProcessId(m_window, &pid);
                m_process = ::OpenProcess(SYNCHRONIZE, FALSE, pid);
            }
            extractGameIcon();
        }
    }

#ifdef GVO_ANALYZE_DEBUG
    {
        static bool done = false;
        if (!done) {
            done = true;
        }
        else {
            return false;
        }

        static GVOImage debugImage;
        if (!debugImage.bitmapHandle()) {
            if (!debugImage.loadFromFile(::g_makeFullPath(k_debugImageFileName))) {
                ::MessageBox(NULL, L"No debugging image", L"Error", MB_ICONERROR);
                exit(0);
            }
        }

        HDC hdc = ::GetWindowDC(::GetDesktopWindow());
        HDC hdcMem = ::CreateCompatibleDC(hdc);
        ::SaveDC(hdcMem);
        ::SelectObject(hdcMem, debugImage.bitmapHandle());
        grabImage(hdcMem, POINT(), debugImage.size());
        ::RestoreDC(hdcMem, -1);
        ::DeleteDC(hdcMem);
        ::ReleaseDC(NULL, hdc);

        updateSurveyCoord();
        return true;
    }
#endif

#ifndef NDEBUG
    // Debug auto-cruise logic to simulate movement if enabled.
    if (s_debugAutoCruiseEnabled) {
        static bool isRandInitialized = false;
        if (!isRandInitialized) {
            srand(::timeGetTime());
            isRandInitialized = true;
        }

        double rad = (s_debugAutoCruiseAngle * M_PI) / 180.0;
        double vx = ::cos(rad);
        double vy = ::sin(rad);

        s_xDebugAutoCruise += vx * s_debugAutoCruiseVelocity;
        s_yDebugAutoCruise += vy * s_debugAutoCruiseVelocity;

        static DWORD tick = ::timeGetTime();
        static DWORD count = 0;

        if ((tick + s_debugAutoCruiseTurnInterval) < ::timeGetTime()) {
            if (10 < (++count)) {
                count = 0;
                s_debugAutoCruiseAngle += 90
                    + (LONG(rand() / double(RAND_MAX) * 90) & ~0x1);
            }
            else {
                if (rand() & 1) {
                    s_debugAutoCruiseAngle += s_debugAutoCruiseTurnAngle;
                }
                else {
                    s_debugAutoCruiseAngle -= s_debugAutoCruiseTurnAngle;
                }
            }
            tick = ::timeGetTime();
        }
        s_debugAutoCruiseAngle = fmod(::fabs(s_debugAutoCruiseAngle), 360.0);

        if (s_xDebugAutoCruise < 0) {
            s_xDebugAutoCruise += k_worldWidth;
        }
        if (s_yDebugAutoCruise < 0) {
            s_yDebugAutoCruise += k_worldHeight;
        }

        s_xDebugAutoCruise = fmod(s_xDebugAutoCruise, double(k_worldWidth));
        s_yDebugAutoCruise = fmod(s_yDebugAutoCruise, double(k_worldHeight));

        m_surveyCoord.x = LONG(s_xDebugAutoCruise);
        m_surveyCoord.y = LONG(s_yDebugAutoCruise);

        uint32_t timeStamp = ::timeGetTime();
        m_speedMeter.updateVelocity(m_ship.velocity(), timeStamp);
        m_ship.updateWithSurveyCoord(m_surveyCoord, timeStamp);

        status.m_surveyCoord = m_surveyCoord;
        status.m_shipVector = m_ship.vector();
        status.m_shipVelocity = m_speedMeter.velocity();
        status.m_timeStamp = timeStamp;

        ::EnterCriticalSection(&m_lock);
        m_statusArray.push_back(status);
        ::SetEvent(m_dataReadyEvent);
        ::LeaveCriticalSection(&m_lock);

        return true;
    }
#endif

    // Actual grab from the game window in non-debug mode.
    if (m_window) {
        RECT rc;
        POINT clientOrg = { 0, 0 };
        ::ClientToScreen(m_window, &clientOrg);
        ::GetClientRect(m_window, &rc);

        SIZE size;
        size.cx = rc.right;
        size.cy = rc.bottom;

        HDC hdc = ::GetDC(::GetDesktopWindow());
        if (!hdc) {
            return false;
        }

        grabImage(hdc, clientOrg, size);
        ::ReleaseDC(::GetDesktopWindow(), hdc);
        m_timeStamp = ::timeGetTime();

        if (!updateSurveyCoord()) {
            return false;
        }

        m_speedMeter.updateVelocity(m_ship.velocity(), m_timeStamp);
        m_ship.updateWithSurveyCoord(m_surveyCoord, m_timeStamp);

        status.m_surveyCoord = m_surveyCoord;
        status.m_shipVector = m_ship.vector();
        status.m_shipVelocity = m_speedMeter.velocity();
        status.m_timeStamp = m_timeStamp;

        ::EnterCriticalSection(&m_lock);
        m_statusArray.push_back(status);
        ::SetEvent(m_dataReadyEvent);
        ::LeaveCriticalSection(&m_lock);

        return true;
    }

    // If we fail to find the window or in debug logic, return false.
    return false;
}

/**
 * getState transfers all collected statuses from the internal buffer
 * to a returned vector. It also resets the data-ready event, which
 * indicates no new data is immediately available once this is called.
 */
std::vector<GameStatus> GameProcess::getState() {
    std::vector<GameStatus> statusArray;

    ::EnterCriticalSection(&m_lock);
    m_statusArray.swap(statusArray);
    ::ResetEvent(m_dataReadyEvent);
    ::LeaveCriticalSection(&m_lock);

    return statusArray;
}

/**
 * This is a static callback (per Windows thread creation convention)
 * that forwards execution to the member function threadMain().
 */
UINT CALLBACK GameProcess::threadMainThunk(LPVOID arg) {
    GameProcess* self = reinterpret_cast<GameProcess*>(arg);
    self->threadMain();
    return 0;
}

/**
 * The main routine for the worker thread. It waits on either the quit signal
 * or the polling event. If the polling event triggers, we call updateState().
 * If the quit signal triggers, we break out of the loop.
 */
void GameProcess::threadMain() {
    std::vector<HANDLE> signals;
    signals.push_back(m_threadQuitSignal);
    signals.push_back(m_pollingTimerEvent);

    while (true) {
        DWORD ret = ::WaitForMultipleObjects(
            static_cast<DWORD>(signals.size()),
            signals.data(),
            FALSE,
            INFINITE
        );

        if (ret >= signals.size()) {
            exit(-1);
        }

        HANDLE active = signals[ret];
        if (active == m_threadQuitSignal) {
            break;
        }
        else if (active == m_pollingTimerEvent) {
            ::ResetEvent(m_pollingTimerEvent);
            updateState();
            continue;
        }
    }
}

/**
 * grabImage captures a portion of the screen (the region of interest)
 * where the survey coordinates are rendered. It stores this snippet
 * in an internal image buffer (m_surveyCoordImage) for further analysis.
 */
void GameProcess::grabImage(HDC hdc, const POINT& offset, const SIZE& size) {
    if (!m_surveyCoordImage.bitmapHandle()) {
        m_surveyCoordImage.createImage(k_surveyCoordSize);
    }

    HDC hdcMem = ::CreateCompatibleDC(hdc);
    ::SaveDC(hdcMem);

    int leftEdge = offset.x;
    int rightEdge = leftEdge + size.cx;
    int topEdge = offset.y;
    int bottomEdge = offset.y + size.cy;

    int xSurvey = rightEdge - k_surveyCoordOffsetFromRightBottom.x;
    int ySurvey = bottomEdge - k_surveyCoordOffsetFromRightBottom.y;

    ::SelectObject(hdcMem, m_surveyCoordImage.bitmapHandle());
    ::BitBlt(
        hdcMem,
        0,
        0,
        k_surveyCoordSize.cx,
        k_surveyCoordSize.cy,
        hdc,
        xSurvey,
        ySurvey,
        SRCCOPY
    );
    ::GdiFlush();

    ::RestoreDC(hdcMem, -1);
    ::DeleteDC(hdcMem);
}

/**
 * updateSurveyCoord uses the SurveyCoordExtractor to read two numbers
 * (X and Y) from the snippet grabbed by grabImage(). Those coordinates
 * are stored in m_surveyCoord. Returns true if successfully extracted.
 */
bool GameProcess::updateSurveyCoord() {
    bool succeeded = false;
    SurveyCoordExtractor extractor(m_surveyCoordImage);
    const std::vector<int>& values = extractor.extractNumbers();

    if (values.size() == 2) {
        m_surveyCoord.x = values[0];
        m_surveyCoord.y = values[1];
        succeeded = true;
    }
    return succeeded;
}

/**
 * extractGameIcon captures the small icon handle from the game window
 * and reads it into m_shipIconImage. This is typically used to show or
 * store the game’s icon for identification or overlay rendering.
 */
void GameProcess::extractGameIcon() {
    if (m_shipIconImage.bitmapHandle()) {
        return;
    }

    HICON icon = reinterpret_cast<HICON>(
        ::GetClassLongPtr(m_window, GCLP_HICONSM)
        );

    if (!icon) {
        return;
    }

    HDC hdcMem = ::CreateCompatibleDC(g_hdcMain);
    ::SaveDC(hdcMem);

    ICONINFO iconInfo = { 0 };
    ::GetIconInfo(icon, &iconInfo);

    BITMAP bmp = { 0 };
    ::GetObject(iconInfo.hbmColor, sizeof(bmp), &bmp);
    int width = bmp.bmWidth;
    int height = bmp.bmHeight;

    ::EnterCriticalSection(&m_lock);
    m_shipIconImage.createImage(width, height, k_PixelFormat_RGBA);

    uint32_t* d = reinterpret_cast<uint32_t*>(m_shipIconImage.mutableImageBits());
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            ::SelectObject(hdcMem, iconInfo.hbmColor);
            COLORREF color = ::GetPixel(hdcMem, x, y);

            uint8_t r = GetRValue(color);
            uint8_t g = GetGValue(color);
            uint8_t b = GetBValue(color);

            ::SelectObject(hdcMem, iconInfo.hbmMask);
            COLORREF mask = ::GetPixel(hdcMem, x, y);
            uint8_t a = mask ? 0x00 : 0xFF;

            *d++ = RGB(b, g, r) | (a << 24);
        }
    }
    ::LeaveCriticalSection(&m_lock);

    ::RestoreDC(hdcMem, -1);
    ::DeleteDC(hdcMem);
}

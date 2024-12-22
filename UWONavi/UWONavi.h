#pragma once

#include "resource.h" // Includes resource definitions for the project
#include <Shlwapi.h>  // Provides path and string utility functions
#include <string>     // For std::wstring

#include "Noncopyable.h" // Custom utility to prevent copying of objects

// External variables shared across the application
extern HINSTANCE g_hinst; // Handle to the application instance
extern HWND g_hwndMain;   // Handle to the main application window
extern HDC g_hdcMain;     // Handle to the main device context

// Constants defining the dimensions of the game world
const int32_t k_worldWidth = 16384;  //!< Width of the game world
const int32_t k_worldHeight = 8192; //!< Height of the game world

/**
 * @brief Converts a relative file path to an absolute path.
 * If the file path is relative, it appends it to the application's directory.
 *
 * @param fileName The file name or relative path
 * @return Full absolute path as a std::wstring
 */
inline std::wstring g_makeFullPath(const std::wstring& fileName) {
    wchar_t filePath[MAX_PATH] = { 0 };

    // Check if the file path is relative
    if (::PathIsRelative(fileName.c_str())) {
        wchar_t dir[MAX_PATH] = { 0 };

        // Get the directory of the current module (application)
        ::GetModuleFileName(::GetModuleHandle(NULL), dir, _countof(dir));
        ::PathRemoveFileSpec(dir); // Remove the file name from the path
        ::PathCombine(filePath, dir, fileName.c_str()); // Combine the directory and file name
    }
    else {
        // If the path is absolute, copy it directly
        ::lstrcpy(filePath, fileName.c_str());
    }

    return filePath;
}

/**
 * @brief Converts radians to degrees.
 *
 * @param radian The angle in radians
 * @return The angle in degrees
 */
inline double g_degreeFromRadian(const double radian) {
    return radian * (180.0 / M_PI);
}

/**
 * @brief Converts degrees to radians.
 *
 * @param degree The angle in degrees
 * @return The angle in radians
 */
inline double g_radianFromDegree(const double degree) {
    return degree * (M_PI / 180.0);
}

/**
 * @brief Queries the current value of the performance counter.
 * This provides a high-resolution timestamp for performance measurements.
 *
 * @return Current performance counter value as int64_t
 */
inline int64_t g_queryPerformanceCounter() {
    int64_t v = 0;
    ::QueryPerformanceCounter((LARGE_INTEGER*)&v);
    return v;
}

/**
 * @brief Queries the frequency of the performance counter.
 * This provides the number of performance counter ticks per second.
 *
 * @return Performance counter frequency as int64_t
 */
inline int64_t g_queryPerformanceFrequency() {
    int64_t v = 0;
    ::QueryPerformanceFrequency((LARGE_INTEGER*)&v);
    return v;
}

/**
 * @brief Retrieves the screen rectangle of a window.
 *
 * @param hwnd Handle to the window
 * @return RECT structure containing the screen coordinates of the window
 */
inline RECT s_windowRect(HWND hwnd) {
    RECT rc = { 0 };
    ::GetWindowRect(hwnd, &rc);
    return std::move(rc); // Returns the rectangle
}

/**
 * @brief Retrieves the client rectangle of a window.
 *
 * @param hwnd Handle to the window
 * @return RECT structure containing the client area coordinates
 */
inline RECT s_clientRect(HWND hwnd) {
    RECT rc = { 0 };
    ::GetClientRect(hwnd, &rc);
    return rc;
}

/**
 * @brief Converts screen coordinates to client area coordinates.
 *
 * @param hwnd Handle to the window
 * @param rc RECT structure with screen coordinates
 * @return RECT structure with client area coordinates
 */
inline RECT s_clientRectFromScreenRect(HWND hwnd, const RECT& rc) {
    RECT rcOut = rc;

    // Convert the top-left and bottom-right corners to client coordinates
    ::ScreenToClient(hwnd, reinterpret_cast<LPPOINT>(&rcOut.left));
    ::ScreenToClient(hwnd, reinterpret_cast<LPPOINT>(&rcOut.right));

    return rcOut;
}

/**
 * @brief Converts client area coordinates to screen coordinates.
 *
 * @param hwnd Handle to the window
 * @param rc RECT structure with client area coordinates
 * @return RECT structure with screen coordinates
 */
inline RECT s_screenRectFromClientRect(HWND hwnd, const RECT& rc) {
    RECT rcOut = rc;

    // Convert the top-left and bottom-right corners to screen coordinates
    ::ClientToScreen(hwnd, reinterpret_cast<LPPOINT>(&rcOut.left));
    ::ClientToScreen(hwnd, reinterpret_cast<LPPOINT>(&rcOut.right));

    return rcOut;
}

#pragma once

#include <Windows.h> // Includes necessary Windows API functions
#include <Shlwapi.h> // For additional string and path utility functions
#include <string>    // For handling std::wstring
#include <vector>    // For std::vector usage

/**
 * @brief Class to handle configuration settings for the application.
 * This class reads and writes configuration data from a file and provides default values for certain settings.
 */
class Config {
private:
    // Configuration file name and predefined section names
    const std::wstring m_fileName;                       // Configuration file path
    const LPCWSTR m_coreSectionName = L"core";           // Core settings section
    const LPCWSTR m_windowSectionName = L"window";       // Window-related settings section
    const LPCWSTR m_surveyCoordSectionName = L"survey";  // Survey coordinates section

#ifndef NDEBUG
    const LPCWSTR m_debugSectionName = L"debug";         // Debug settings section (only in debug mode)
#endif

public:
    // Configuration variables for various features
    std::wstring m_mapFileName;              // Map file name
    UINT m_pollingInterval;                  // Polling interval in milliseconds
    POINT m_windowPos;                       // Position of the window
    SIZE m_windowSize;                       // Size of the window
    bool m_keepForeground;                   // Keep the application window in the foreground
    bool m_traceShipPositionEnabled;         // Enable tracing of ship positions
    bool m_speedMeterEnabled;                // Enable speed meter display
    bool m_shipVectorLineEnabled;            // Enable ship vector line display
    POINT m_initialSurveyCoord;              // Initial survey coordinates

#ifndef NDEBUG
    // Debug-specific settings
    bool m_debugAutoCruiseEnabled;           // Enable auto-cruise for debugging
    double m_debugAutoCruiseVelocity;        // Debug auto-cruise speed
    double m_debugAutoCruiseTurnAngle;       // Debug auto-cruise turn angle
    uint32_t m_debugAutoCruiseTurnInterval;  // Debug auto-cruise turn interval
#endif

    // Constructor initializes default values and configuration file path
    Config(LPCWSTR fileName)
        : m_fileName(g_makeFullPath(fileName)),
        m_mapFileName(L"map.png"),
        m_pollingInterval(1000),
        m_windowPos(defaultPosition()),
        m_windowSize(defaultSize()),
        m_keepForeground(false),
        m_traceShipPositionEnabled(true),
        m_speedMeterEnabled(true),
        m_shipVectorLineEnabled(true),
        m_initialSurveyCoord(defaultSurveyCoord())
#ifndef NDEBUG
        , m_debugAutoCruiseEnabled(false),
        m_debugAutoCruiseVelocity(1.0),
        m_debugAutoCruiseTurnAngle(12.0),
        m_debugAutoCruiseTurnInterval(7000)
#endif
    {
    }

    // Destructor
    ~Config() {}

    /**
     * @brief Saves the current configuration settings to the file.
     */
    void save() {
        LPCWSTR fn = m_fileName.c_str(); // File path
        LPCWSTR section;

        // Save core settings
        section = m_coreSectionName;
        ::WritePrivateProfileString(section, L"map", m_mapFileName.c_str(), fn);
        ::WritePrivateProfileString(section, L"pollingInterval", std::to_wstring(m_pollingInterval).c_str(), fn);
        ::WritePrivateProfileString(section, L"traceEnabled", std::to_wstring(m_traceShipPositionEnabled).c_str(), fn);
        ::WritePrivateProfileString(section, L"speedMeterEnabled", std::to_wstring(m_speedMeterEnabled).c_str(), fn);
        ::WritePrivateProfileString(section, L"shipVectorLineEnabled", std::to_wstring(m_shipVectorLineEnabled).c_str(), fn);

        // Save window settings
        section = m_windowSectionName;
        ::WritePrivateProfileString(section, L"x", std::to_wstring(m_windowPos.x).c_str(), fn);
        ::WritePrivateProfileString(section, L"y", std::to_wstring(m_windowPos.y).c_str(), fn);
        ::WritePrivateProfileString(section, L"cx", std::to_wstring(m_windowSize.cx).c_str(), fn);
        ::WritePrivateProfileString(section, L"cy", std::to_wstring(m_windowSize.cy).c_str(), fn);
        ::WritePrivateProfileString(section, L"keepForeground", std::to_wstring(m_keepForeground).c_str(), fn);

        // Save survey coordinates
        section = m_surveyCoordSectionName;
        ::WritePrivateProfileString(section, L"x", std::to_wstring(m_initialSurveyCoord.x).c_str(), fn);
        ::WritePrivateProfileString(section, L"y", std::to_wstring(m_initialSurveyCoord.y).c_str(), fn);

#ifndef NDEBUG
        // Save debug settings
        section = m_debugSectionName;
        ::WritePrivateProfileString(section, L"autoCruiseEnabled", std::to_wstring(m_debugAutoCruiseEnabled).c_str(), fn);
        ::WritePrivateProfileString(section, L"autoCruiseVelocity", std::to_wstring(m_debugAutoCruiseVelocity).c_str(), fn);
        ::WritePrivateProfileString(section, L"autoCruiseTurnAngle", std::to_wstring(m_debugAutoCruiseTurnAngle).c_str(), fn);
        ::WritePrivateProfileString(section, L"autoCruiseTurnInterval", std::to_wstring(m_debugAutoCruiseTurnInterval).c_str(), fn);
#endif

        // Flush the configuration file
        ::WritePrivateProfileString(NULL, NULL, NULL, fn);
    }

    /**
     * @brief Loads configuration settings from the file.
     */
    void load() {
        LPCWSTR fn = m_fileName.c_str();
        LPCWSTR section;
        std::vector<wchar_t> buf(4096);

        // Load core settings
        section = m_coreSectionName;
        ::GetPrivateProfileStringW(section, L"map", m_mapFileName.c_str(), &buf[0], buf.size(), fn);
        m_mapFileName = &buf[0];
        m_pollingInterval = ::GetPrivateProfileInt(section, L"pollingInterval", m_pollingInterval, fn);
        m_traceShipPositionEnabled = ::GetPrivateProfileInt(section, L"traceEnabled", m_traceShipPositionEnabled, fn) != 0;
        m_speedMeterEnabled = ::GetPrivateProfileInt(section, L"speedMeterEnabled", m_speedMeterEnabled, fn) != 0;
        m_shipVectorLineEnabled = ::GetPrivateProfileInt(section, L"shipVectorLineEnabled", m_shipVectorLineEnabled, fn) != 0;

        // Load window settings
        section = m_windowSectionName;
        m_windowPos.x = ::GetPrivateProfileInt(section, L"x", m_windowPos.x, fn);
        m_windowPos.y = ::GetPrivateProfileInt(section, L"y", m_windowPos.y, fn);
        m_windowSize.cx = ::GetPrivateProfileInt(section, L"cx", m_windowSize.cx, fn);
        m_windowSize.cy = ::GetPrivateProfileInt(section, L"cy", m_windowSize.cy, fn);
        m_keepForeground = ::GetPrivateProfileInt(section, L"keepForeground", m_keepForeground, fn) != 0;

        // Load survey coordinates
        section = m_surveyCoordSectionName;
        m_initialSurveyCoord.x = ::GetPrivateProfileInt(section, L"x", m_initialSurveyCoord.x, fn);
        m_initialSurveyCoord.y = ::GetPrivateProfileInt(section, L"y", m_initialSurveyCoord.y, fn);

#ifndef NDEBUG
        // Load debug settings
        section = m_debugSectionName;
        m_debugAutoCruiseEnabled = ::GetPrivateProfileInt(section, L"autoCruiseEnabled", m_debugAutoCruiseEnabled, fn) != 0;
        ::GetPrivateProfileString(section, L"autoCruiseVelocity", std::to_wstring(m_debugAutoCruiseVelocity).c_str(), &buf[0], buf.size(), fn);
        m_debugAutoCruiseVelocity = std::stod(std::wstring(&buf[0]));
        ::GetPrivateProfileString(section, L"autoCruiseTurnAngle", std::to_wstring(m_debugAutoCruiseTurnAngle).c_str(), &buf[0], buf.size(), fn);
        m_debugAutoCruiseTurnAngle = std::stod(std::wstring(&buf[0]));
        m_debugAutoCruiseTurnInterval = ::GetPrivateProfileInt(section, L"autoCruiseTurnInterval", m_debugAutoCruiseTurnInterval, fn);
#endif
    }

private:
    // Default window position
    static POINT defaultPosition() {
        POINT pt = { CW_USEDEFAULT, 0 };
        return pt;
    }

    // Default window size
    static SIZE defaultSize() {
        SIZE size = { CW_USEDEFAULT, 0 };
        return size;
    }

    // Default survey coordinates
    static POINT defaultSurveyCoord() {
        POINT p = { 15785, 3204 }; // Example coordinates
        return p;
    }
};

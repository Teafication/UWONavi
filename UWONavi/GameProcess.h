#pragma once

#include "Noncopyable.h"  // Prevent copying of objects (inheritance for non-copyable class)
#include "Image.h"        // Handles image operations
#include "Config.h"       // Handles configuration data
#include "SpeedMeter.h"   // Tracks and calculates speed
#include "Ship.h"         // Represents the ship object
#include "GameStatus.h"   // Represents the current game state

/**
 * @class GameProcess
 * @brief Core class for managing the game process of "Uncharted Waters Online."
 * Handles game state updates, image processing, speed calculations, and more.
 */
class GameProcess : private Noncopyable {
private:
    // Handles and variables for managing the game process
    HANDLE m_process;             // Handle to the game process
    HWND m_window;                // Handle to the game window
    Image m_shipIconImage;     // Image of the ship's icon
    Image m_surveyCoordImage;  // Image for survey coordinate extraction
    POINT m_surveyCoord;          // Current survey coordinates
    DWORD m_timeStamp;            // Timestamp of the last update

    SpeedMeter m_speedMeter;   // Tracks ship's speed
    Ship m_ship;               // Represents the player's ship

    uint32_t m_pollingInterval;   // Polling interval for updates
    HANDLE m_pollingTimerEvent;   // Event for polling timer
    UINT m_pollingTimerEventID;   // ID for the polling timer

    HANDLE m_workerThread;        // Worker thread for handling updates
    HANDLE m_threadQuitSignal;    // Signal to stop the worker thread
    HANDLE m_dataReadyEvent;      // Event signaling data is ready
    CRITICAL_SECTION m_lock;      // Critical section for thread safety

    std::vector<GameStatus> m_statusArray; // Stores the game statuses

public:
    /**
     * @brief Constructor initializes all member variables and sets up synchronization objects.
     */
    GameProcess()
        : m_process(NULL),
        m_window(NULL),
        m_surveyCoord(),
        m_timeStamp(),
        m_pollingInterval(),
        m_pollingTimerEvent(::CreateEvent(NULL, TRUE, TRUE, NULL)), // Create timer event
        m_pollingTimerEventID(),
        m_workerThread(),
        m_dataReadyEvent(::CreateEvent(NULL, TRUE, FALSE, NULL)) // Create data-ready event
    {
        ::InitializeCriticalSection(&m_lock); // Initialize critical section
    }

    /**
     * @brief Destructor ensures proper cleanup of resources and synchronization objects.
     */
    virtual ~GameProcess() {
        clear(); // Clear process resources
        ::CloseHandle(m_dataReadyEvent);
        ::CloseHandle(m_pollingTimerEvent);
        ::DeleteCriticalSection(&m_lock); // Delete critical section
    }

    /**
     * @brief Retrieves the handle to the game process.
     * @return The handle to the game process.
     */
    HANDLE processHandle() const {
        return m_process;
    }

    /**
     * @brief Clears all resources related to the game process.
     */
    void clear();

    /**
     * @brief Sets up the game process using configuration data.
     * @param config Reference to the configuration object.
     */
    void setup(const Config& config);

    /**
     * @brief Tears down the game process by stopping the worker thread and releasing resources.
     */
    void teardown();

#ifndef NDEBUG
    /**
     * @brief Enables or disables debug auto-cruise mode.
     * @param enabled True to enable, false to disable.
     */
    void enableDebugAutoCruise(bool enabled);

    /**
     * @brief Sets the polling interval for game state updates.
     * @param interval Polling interval in milliseconds.
     */
    void setPollingInterval(DWORD interval);
#endif

    /**
     * @brief Retrieves the current game state.
     * @return A vector containing game statuses.
     */
    std::vector<GameStatus> getState();

    /**
     * @brief Retrieves the timestamp of the last game state update.
     * @return The timestamp.
     */
    DWORD timeStamp() const {
        return m_timeStamp;
    }

    /**
     * @brief Retrieves the handle to the event signaling that data is ready.
     * @return The handle to the data-ready event.
     */
    HANDLE dataReadyEvent() const {
        return m_dataReadyEvent;
    }

#ifndef NDEBUG
    /**
     * @brief Retrieves the survey coordinate image for debugging.
     * @return A reference to the survey coordinate image.
     */
    const Image& surveyCoordImage() const {
        return m_surveyCoordImage;
    }
#endif

    /**
     * @brief Retrieves the ship icon image.
     * @return A pointer to the ship icon image, or NULL if not available.
     */
    const Image* shipIconImage() {
        const Image* img = NULL;
        ::EnterCriticalSection(&m_lock);
        if (m_shipIconImage.bitmapHandle()) {
            img = &m_shipIconImage;
        }
        ::LeaveCriticalSection(&m_lock);
        return img;
    }

private:
    /**
     * @brief Updates the game state by processing the current state of the game window.
     * @return True if the update was successful, false otherwise.
     */
    bool updateState();

    /**
     * @brief Thread entry point for handling game process updates.
     * @param arg Pointer to the GameProcess instance.
     * @return Exit code of the thread.
     */
    static UINT CALLBACK threadMainThunk(LPVOID arg);

    /**
     * @brief Main function executed by the worker thread.
     */
    void threadMain();

    /**
     * @brief Captures a portion of the screen and stores it in a Image.
     * @param hdc Handle to the device context.
     * @param offset Top-left corner of the region to capture.
     * @param size Size of the region to capture.
     */
    void grabImage(HDC hdc, const POINT& offset, const SIZE& size);

    /**
     * @brief Updates the survey coordinates by processing the captured image.
     * @return True if the coordinates were successfully updated, false otherwise.
     */
    bool updateSurveyCoord();

    /**
     * @brief Extracts the game icon from the game window.
     */
    void extractGameIcon();
};

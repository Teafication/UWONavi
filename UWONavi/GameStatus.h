#pragma once

#include <cinttypes>   // Provides fixed-width integer types like uint32_t
#include <Windows.h>   // Provides definitions for POINT and other Windows types
#include "Vector.h" // Handles vector-related operations

/**
 * @class GameStatus
 * @brief Represents the current status of the game at a specific point in time.
 *
 * This class encapsulates information such as the timestamp, ship position,
 * velocity, and vector direction, which are used to track the game's state.
 */
class GameStatus {
public:
    DWORD m_timeStamp;         //!< Timestamp of when the status was recorded
    POINT m_surveyCoord;       //!< Coordinates of the survey location (player's position)
    Vector m_shipVector;    //!< Direction vector of the ship's movement
    double m_shipVelocity;     //!< Speed of the ship (units per second)

    /**
     * @brief Constructor initializes the velocity to zero by default.
     */
    GameStatus()
        : m_shipVelocity(0.0) // Default ship velocity is 0
    {
    }
};

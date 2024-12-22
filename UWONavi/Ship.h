#pragma once

#include <Windows.h>           // Windows API for basic types like POINT
#include <vector>              // For using the vector container
#include <deque>               // For using the deque container (for storing vectors)
#include "Noncopyable.h"    // Prevent copying of the class
#include "UWONavi.h"         // Presumably for navigation-related functionality
#include "Vector.h"         // For vector mathematics (e.g., representing directions, movement)
#include "Velocity.h"       // For handling velocity calculations

//! @brief The Ship class represents a ship's movement, velocity, and survey coordinates.
class Ship : private Noncopyable {
private:
    typedef std::deque<Vector> VectorArray;  // Type alias for a deque (double-ended queue) of Vector

private:
    POINT m_surveyCoord;              //!< Current survey coordinates of the ship (latitude/longitude or similar)
    Vector m_vector;               //!< Current movement vector of the ship (direction and speed)
    VectorArray m_vectorArray;        //!< A history of vectors representing the ship's movement over time
    double m_velocity;                //!< The current velocity of the ship (not velocity per second)
    uint32_t m_timeStamp;             //!< The last timestamp when the ship's position was updated
    Velocity m_velocityPerSecond; //!< The ship's velocity measured per second (a more precise measurement)

public:
    //! @brief Default constructor initializes member variables to default values.
    Ship() :
        m_surveyCoord(),
        m_velocity(),
        m_timeStamp()
    {
    }

    //! @brief Set the initial survey coordinates of the ship.
    //! @param initialSurveyCoord The initial coordinates (e.g., starting point) of the ship.
    inline void setInitialSurveyCoord(const POINT& initialSurveyCoord)
    {
        m_surveyCoord = initialSurveyCoord;  // Set the initial survey coordinates
    }

    //! @brief Get the current vector (direction and speed) of the ship.
    //! @return A reference to the current movement vector.
    inline const Vector& vector() const
    {
        return m_vector;  // Return the current movement vector of the ship
    }

    //! @brief Update the ship's state based on new survey coordinates and timestamp.
    //! This function updates the ship's velocity and heading.
    //! @param surveyCoord The new survey coordinates of the ship.
    //! @param timeStamp The current timestamp (time when the position is updated).
    void updateWithSurveyCoord(const POINT& surveyCoord, const uint32_t timeStamp);

    //! @brief Get the velocity of the ship.
    //! @return The velocity of the ship (calculated per second).
    inline double velocity() const
    {
        return m_velocityPerSecond.velocity();  // Return the velocity calculated per second
    }
};

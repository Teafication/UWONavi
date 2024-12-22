#include "stdafx.h"
#include <string>
#include "Ship.h"

namespace {
    // Helper function to round the angle in the game world. 
    // Converts a radian to degrees, rounds it to the nearest multiple of 2, and converts it back to radians.
    inline double s_roundInGameAngle(const double radian)
    {
        double degree = g_degreeFromRadian(radian);  // Convert radian to degree
        degree = ::floor(::round(degree) * 0.5) * 2.0;  // Round to the nearest multiple of 2
        return g_radianFromDegree(degree);  // Convert back to radians
    }

    // Helper function to round a vector to the nearest game direction.
    inline Vector s_roundInGameVector(const Vector& v)
    {
        // Calculate the angle between the given vector and the unit vector (1, 0)
        double sita = Vector(1, 0).angleTo(v);

        // Round the angle to the nearest multiple of 2 degrees (normalized game angle)
        sita = s_roundInGameAngle(sita);

        // Convert back to a unit vector after rounding the angle
        return Vector(::cos(sita), -::sin(sita));  // Return the normalized vector
    }

    // Helper function to calculate the resolution for a vector based on its length.
    inline double s_resolutionForVector(const Vector& vector)
    {
        const double length = vector.length();
        if (length == 0.0) {
            return 0.0;
        }
        const double resolution = M_PI_2 / length;  // Resolution is inversely proportional to the length
        return resolution;
    }

    // Helper function to determine if two vectors point in significantly different directions.
    inline bool s_isAnotherDirection(const Vector& v1, const Vector& v2)
    {
        const double resolution = s_resolutionForVector(v2);  // Get the resolution for the second vector
        const double angle = v1.angleTo(v2);  // Calculate the angle between the two vectors
        return resolution < ::fabs(angle);  // Return true if the angle is greater than the resolution
    }
}

// Function to update the ship's state with a new survey coordinate and timestamp
void Ship::updateWithSurveyCoord(const POINT& surveyCoord, const uint32_t timeStamp)
{
    const Vector v(m_surveyCoord, surveyCoord);  // Create a vector from the current and new survey coordinates

    m_velocity = v.length();  // Calculate the velocity as the length of the vector (distance)
    m_velocityPerSecond.setVelocity(m_velocity, timeStamp - m_timeStamp);  // Update the velocity per second based on the timestamp
    m_timeStamp = timeStamp;  // Update the timestamp to the current time

    // If the velocity is zero (no movement), exit early.
    if (m_velocity == 0.0) {
        return;
    }
    m_surveyCoord = surveyCoord;  // Update the current survey coordinate

    // If the ship doesn't have a current heading vector (m_vector is zero), initialize it
    if (m_vector.length() == 0.0) {
        m_vector = s_roundInGameVector(v.normalizedVector());  // Normalize the vector and round it to the nearest game vector
        return;
    }

    // Composite the movement vector with previous vectors in the vector array
    Vector headVector = v;
    for (VectorArray::const_reverse_iterator it = m_vectorArray.rbegin(); it != m_vectorArray.rend(); ++it) {
        headVector.composite(*it);  // Apply the previous vectors in reverse order
        if (s_isAnotherDirection(m_vector, headVector)) {
            // If the direction has changed significantly, erase the previous vectors from the array
            m_vectorArray.erase(m_vectorArray.begin(), std::next(it).base());
            break;
        }
    }

    // Round the new vector to the nearest game vector
    m_vector = s_roundInGameVector(headVector.normalizedVector());

    // Add the new vector to the vector array
    m_vectorArray.push_back(v);

    // Remove the first element of the vector array if the distance between the first vector and the head vector is too large (threshold of 180 degrees)
    if (180 < (headVector.length() - m_vectorArray.front().length())) {
        m_vectorArray.pop_front();  // Remove the oldest vector from the array
    }
}

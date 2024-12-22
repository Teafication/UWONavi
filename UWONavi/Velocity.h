#pragma once
#include <cstdint>  // For fixed-width integer types like uint32_t

// 1b・ velocity class, representing velocity over time
class Velocity {
private:
    double m_velocityPerSecond;  //!< Stores the velocity in units per second (calculated velocity)

public:
    // Default constructor initializes m_velocityPerSecond to zero
    Velocity() :
        m_velocityPerSecond(0.0)
    {
    }

    // Constructor that initializes the velocity based on a given velocity (distance) and time (dt)
    Velocity(const double velocity, const uint32_t dt)
    {
        setVelocity(velocity, dt);  // Set the velocity using the provided distance (velocity) and time (dt)
    }

    // Destructor - doesn't perform any specific cleanup in this case
    ~Velocity()
    {
    }

    // Sets the velocity based on distance (velocity) and time (dt) in milliseconds
    void setVelocity(const double velocity, const uint32_t dt)
    {
        // Calculate the velocity per second by dividing the distance by time (in milliseconds) and multiplying by 1000
        m_velocityPerSecond = (velocity / dt) * 1000.0;
    }

    // Returns the velocity in units per second
    double velocity() const
    {
        return m_velocityPerSecond;
    }
};

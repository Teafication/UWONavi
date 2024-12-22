#pragma once

#include <deque>             // For using deque (double-ended queue) container
#include <algorithm>         // For using algorithms like std::max_element
#include "Noncopyable.h"  // Prevents copying of the class

//! @brief The SpeedMeter class is responsible for calculating and tracking the speed (velocity) of an object.
class SpeedMeter : private Noncopyable {
private:
    typedef std::deque<double> Array;  // Type alias for a deque of doubles (to store velocities)

    //! @brief Structure to represent a velocity log item (timestamp and velocity)
    struct VelocityLogItem {
        uint32_t timeStamp;  //!< Timestamp when the velocity was recorded
        double velocity;     //!< Recorded velocity value

        // Default constructor initializing to default values
        VelocityLogItem() :
            timeStamp(),
            velocity()
        {
        }

        // Constructor with timestamp and velocity parameters
        VelocityLogItem(const uint32_t timeStamp, const double velocity) :
            timeStamp(timeStamp),
            velocity(velocity)
        {
        }
    };

    typedef std::deque<VelocityLogItem> VelocityyArray;  // Type alias for deque of VelocityLogItem (velocity log)
    typedef std::deque<double> VelocityLog;              // Type alias for deque of velocity values (to track average velocities)

    const uint32_t k_velocityMeasuringDistance = 5000;   //!< Distance in units for velocity measurement

private:
    VelocityyArray m_velocityArray;  //!< Array storing the velocity logs with timestamps
    VelocityLog m_velocityLog;       //!< Log of calculated velocities per second
    double m_velocity;               //!< The current velocity (updated based on latest measurements)

public:
    // Constructor that initializes the velocity to 0
    SpeedMeter() :
        m_velocity()
    {
    }

    // Destructor
    ~SpeedMeter()
    {
    }

    //! @brief Updates the velocity with a new value and timestamp, and performs necessary calculations
    //! @param velocity The new velocity value to be recorded
    //! @param timeStamp The timestamp at which the velocity was measured
    inline void updateVelocity(const double velocity, const uint32_t timeStamp)
    {
        m_velocityArray.push_back(VelocityLogItem(timeStamp, velocity));  // Add the new velocity log item

        removeOldItem(timeStamp);  // Remove outdated items based on the timestamp
        updateVelocityLog();       // Update the velocity log with new calculated values

        m_velocity = fastestVelocity();  // Update the current velocity with the fastest recorded velocity
    }

    //! @brief Returns the current velocity (the fastest recorded velocity)
    //! @return The current velocity
    double velocity() const
    {
        return m_velocity;
    }

private:
    //! @brief Calculates the average velocity per second based on the velocity log
    //! @return The average velocity per second
    inline double calcVelocityPerSecond()
    {
        double velocity = 0.0;

        // If there are less than 2 items in the log, return 0.0 as we need at least two points to calculate
        if (m_velocityArray.size() < 2) {
            return 0.0;
        }

        // Sum up all the velocities in the log
        for (VelocityyArray::const_iterator it = m_velocityArray.begin(); it != m_velocityArray.end(); ++it) {
            velocity += it->velocity;
        }

        // Calculate the average velocity
        velocity /= m_velocityArray.size();
        return velocity;
    }

    //! @brief Removes old velocity log items based on the timestamp and distance
    //! @param timeStamp The current timestamp, used to filter out outdated logs
    inline void removeOldItem(const uint32_t timeStamp)
    {
        VelocityyArray::const_iterator removeMark = m_velocityArray.end();

        // Iterate through the velocity log and find the oldest items
        for (VelocityyArray::const_iterator it = m_velocityArray.begin(); it != m_velocityArray.end(); ++it) {
            const uint32_t dt = timeStamp - it->timeStamp;  // Calculate the difference between current timestamp and log item timestamp
            if (dt <= k_velocityMeasuringDistance) {
                break;  // If the timestamp is within the measuring distance, stop removing items
            }
            removeMark = it;  // Mark this item for removal
        }

        // Erase the outdated items from the beginning of the array
        if (removeMark != m_velocityArray.end()) {
            m_velocityArray.erase(m_velocityArray.begin(), std::next(removeMark));
        }
    }

    //! @brief Updates the velocity log with the current average velocity
    inline void updateVelocityLog()
    {
        m_velocityLog.push_back(calcVelocityPerSecond());  // Add the current calculated velocity to the log

        // Keep only the last 3 recorded velocities (limit the size of the log)
        if (3 < m_velocityLog.size()) {
            m_velocityLog.pop_front();  // Remove the oldest entry if there are more than 3 entries
        }
    }

    //! @brief Returns the fastest velocity recorded in the log
    //! @return The fastest velocity recorded in the log
    inline double fastestVelocity() const
    {
        double fastest = 0.0;

        // Find the maximum velocity in the log
        VelocityLog::const_iterator it = std::max_element(m_velocityLog.begin(), m_velocityLog.end());
        if (it != m_velocityLog.end()) {
            fastest = *it;  // Set the fastest velocity to the maximum value
        }
        return fastest;
    }
};

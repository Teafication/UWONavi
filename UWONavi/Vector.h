#pragma once

#include <math.h>
#include "UWONavi.h"        // For navigation-related constants and functions
#include "NormalizedPoint.h" // For using normalized point objects (for specific vector-based operations)

//! @brief The Vector class represents a 2D vector with operations like angle calculation, normalization, and addition.
class Vector {
private:
    double m_x = 0.0;  //!< X component of the vector
    double m_y = 0.0;  //!< Y component of the vector
    double m_length = 0.0; //!< Length (magnitude) of the vector

public:
    // Default constructor initializes the vector to (0, 0) with zero length
    Vector() = default;

    // Constructor that initializes the vector with x and y values and calculates its length
    Vector(const double x, const double y) :
        m_x(x),
        m_y(y),
        m_length(calcLength(x, y))  // Calculates the length based on x and y
    {
    }

    // Constructor that initializes a vector based on two survey coordinates (POINT)
    // Takes the coordinates of two points and calculates the difference to create a vector
    Vector(const POINT& p1, const POINT& p2)
    {
        const LONG k_threshold = k_worldWidth / 2;  // Threshold for calculating vector (world wrapping consideration)
        LONG dx = p2.x - p1.x;

        // Handle world wrapping for the x-coordinate
        if (k_threshold < dx) {
            dx -= k_worldWidth;
        }
        else if (dx < -k_threshold) {
            dx += k_worldWidth;
        }

        // Set x and y based on the difference between points
        m_x = dx;
        m_y = p2.y - p1.y;
        m_length = calcLength(m_x, m_y);  // Calculate the length of the vector
    }

    // Constructor that initializes a vector based on two normalized points (NormalizedPoint)
    Vector(const NormalizedPoint& p1, const NormalizedPoint& p2)
    {
        const float k_threshold = 0.5f;  // Threshold for the normalized points
        float dx = p2.x() - p1.x();

        // Handle world wrapping for normalized points
        if (k_threshold < dx) {
            dx -= k_threshold;
        }
        else if (dx < -k_threshold) {
            dx += k_threshold;
        }

        // Set x and y based on the difference between normalized points
        m_x = dx;
        m_y = p2.y() - p1.y();
        m_length = calcLength(m_x, m_y);  // Calculate the length of the vector
    }

    // Returns the x component of the vector
    inline double x() const
    {
        return m_x;
    }

    // Returns the y component of the vector
    inline double y() const
    {
        return m_y;
    }

    // Returns the length (magnitude) of the vector
    inline double length() const
    {
        return m_length;
    }

    // Returns a normalized version of the vector (length 1)
    inline Vector normalizedVector() const
    {
        return normalizedVector(length());  // Normalize the vector based on its current length
    }

    // Normalizes the vector to a specific length (norm)
    inline Vector normalizedVector(const double norm) const
    {
        Vector v(m_x, m_y);
        const double length = v.length();

        // Scale the vector components to achieve the desired length
        v.m_x = (v.m_x / length) * norm;
        v.m_y = (v.m_y / length) * norm;
        v.m_length = norm;
        return v;
    }

    // Returns the angle (in radians) between this vector and another vector
    inline double angleTo(const Vector& other) const
    {
        return ::atan2(other.m_x * m_y - m_x * other.m_y, m_x * other.m_x + m_y * other.m_y);
    }

    // Adds another vector to this vector (composite operation)
    inline void composite(const Vector& other)
    {
        m_x += other.m_x;
        m_y += other.m_y;
        m_length = calcLength(m_x, m_y);  // Recalculate the length of the new vector
    }

    //! @brief Returns the point on the map from the origin (starting point) given a length in the direction of the vector
    //! @param origin The starting point (origin)
    //! @param length The distance from the origin along the vector
    //! @return A POINT structure representing the calculated point from the origin
    POINT pointFromOriginWithLength(const POINT& origin, const LONG length) const
    {
        Vector v = normalizedVector();  // Get the normalized vector
        const POINT p = {
            origin.x + LONG(v.x() * length),  // Calculate the x position
            origin.y + LONG(v.y() * length)   // Calculate the y position
        };
        return p;  // Return the calculated point
    }

private:
    // A static helper function to calculate the length (magnitude) of a vector from its x and y components
    static inline double calcLength(const double x, const double y)
    {
        return ::pow(x * x + y * y, 0.5);  // Pythagorean theorem: sqrt(x^2 + y^2)
    }

};

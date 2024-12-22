#pragma once  // Ensures that the header file is included only once in a compilation unit.

/////////////////////////////////////////////////////////////////////////////
// Class: NormalizedPoint
// This class represents a normalized 2D point with float coordinates (x, y).
// A "normalized" point typically means its values are within a specific range, 
// but in this class, it's used as a simple point with two floating-point values.

class NormalizedPoint {
private:
    float m_x;  // The x-coordinate of the point.
    float m_y;  // The y-coordinate of the point.

public:
    // Default constructor: Initializes m_x and m_y to 0.0 by default.
    NormalizedPoint() :
        m_x(0.0f),
        m_y(0.0f)
    {
    }

    // Parameterized constructor: Initializes m_x and m_y with the provided values.
    NormalizedPoint(float x, float y) :
        m_x(x),
        m_y(y)
    {
    }

    // Method to compare two NormalizedPoint objects for equality.
    // It returns true if both x and y values are equal between the two points.
    bool isEqualValue(const NormalizedPoint rhs) const
    {
        return m_x == rhs.m_x && m_y == rhs.m_y;
    }

    // Getter for the x-coordinate.
    // This function returns the x value of the point.
    float x() const
    {
        return m_x;
    }

    // Getter for the y-coordinate.
    // This function returns the y value of the point.
    float y() const
    {
        return m_y;
    }

};

// Static assertion to check the size of NormalizedPoint.
// The size of a NormalizedPoint object should be equal to the size of two float values (sizeof(float)*2).
// If this condition is not met, a compile-time error with the message "bat size." will occur.
static_assert(sizeof(NormalizedPoint) == (sizeof(float) * 2), "bat size.");

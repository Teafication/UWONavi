#pragma once

#include <deque>             // For using deque container to store lines
#include <ctime>             // For time-related functions
#include "NormalizedPoint.h" // For using normalized points to represent coordinates

//! @brief Represents a ship's route with a series of points and related metadata.
class ShipRoute {
    friend std::ostream& operator << (std::ostream& os, ShipRoute& shipRoute); // Serialization of route
    friend std::istream& operator >> (std::istream& is, ShipRoute& shipRoute); // Deserialization of route

public:
    typedef std::vector<NormalizedPoint> Line;   // Each line in the route is a vector of normalized points
    typedef std::deque<Line> Lines;                  // A deque of lines, each representing a segment of the route

private:
    Lines m_lines;            //!< Stores all the lines in the route (each line is a series of points)
    double m_length = 0.0;    //!< The total length of the route (in the same units as the points)
    bool m_favorite = false;  //!< Flag to indicate if the route is marked as a favorite
    bool m_hilight = false;   //!< Flag to indicate if the route is highlighted
    bool m_fixed = false;     //!< Flag to indicate if the route is fixed (not editable)

public:
    // Default constructor
    ShipRoute() = default;
    // Default destructor
    ~ShipRoute() = default;

    //! @attention Add a new point to the route.
    //! @param point The new normalized point to add to the route.
    void addRoutePoint(const NormalizedPoint& point);

    //! @brief Get the lines (segments) of the ship's route.
    //! @return A constant reference to the lines of the route.
    const Lines& getLines() const
    {
        return m_lines;
    }

    //! @brief Check if the route is marked as a favorite.
    //! @return `true` if the route is a favorite, `false` otherwise.
    bool isFavorite() const
    {
        return m_favorite;
    }

    //! @brief Set whether the route is a favorite.
    //! @param favorite `true` to mark the route as a favorite, `false` to unmark it.
    void setFavorite(bool favorite)
    {
        m_favorite = favorite;
    }

    //! @brief Check if the route is highlighted.
    //! @return `true` if the route is highlighted, `false` otherwise.
    bool isHilight() const
    {
        return m_hilight;
    }

    //! @brief Set whether the route is highlighted.
    //! @param hilight `true` to highlight the route, `false` to remove the highlight.
    void setHilight(bool hilight)
    {
        m_hilight = hilight;
    }

    //! @brief Join the current route with a source route by merging their lines.
    //! @param srcRoute The source route to join with the current route.
    void jointPreviousLinesWithRoute(const ShipRoute& srcRoute);

    //! @brief Check if the route is empty (has no lines or all lines are empty).
    //! @return `true` if the route is empty, `false` otherwise.
    bool isEmptyRoute() const
    {
        if (m_lines.empty()) {
            return true;  // The route is empty if no lines exist
        }

        // Check if all lines are empty
        for (auto line : m_lines) {
            if (!line.empty()) {
                return false;  // The route is not empty if any line contains points
            }
        }
        return true;
    }

    //! @brief Check if the route is fixed (non-editable).
    //! @return `true` if the route is fixed, `false` otherwise.
    bool isFixed() const
    {
        return m_fixed;
    }

    //! @brief Set whether the route is fixed.
    //! @param isFixed `true` to make the route fixed, `false` to allow edits.
    void setFix(bool isFixed)
    {
        m_fixed = isFixed;
    }

    //! @brief Get the total length of the route.
    //! @return The total length of the route in the same units as the points.
    double length() const
    {
        return m_length;
    }

    //! @brief Add a new line to the route (a new segment).
    //! @param line The line (a series of normalized points) to add to the route.
    void addLine(Line&& line)
    {
        m_lines.push_back(line);  // Add the line to the list of lines
    }

private:
};

typedef std::shared_ptr<ShipRoute> ShipRoutePtr;    // A shared pointer to a ship route
typedef std::weak_ptr<ShipRoute> ShipRouteWeakPtr;  // A weak pointer to a ship route
